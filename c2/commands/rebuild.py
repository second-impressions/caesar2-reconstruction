"""``c2 rebuild`` — link the recovered source + delinked objects into a runnable PS.EXE.

``rebuild`` produces a self-contained
DOS/4GW-bound ``build/PS.EXE`` (the original stays at ``original/PS.EXE``;
nothing ever writes there) from:

  * every recovered game TU in ``src/`` (compiled with the proven
    ``PS_CFLAGS``), EXCEPT the data-only scaffolds of modules we delink
    (``smackinp.c`` / ``sndail.c`` / ``sndnull.c`` — their real bodies AND
    their real data come from the delinked object);
  * the eight hand-written C2 asm modules (wasm; includes ``palet.asm``,
    the VGA DAC loader);
  * the **delinked** third-party blob recovered byte-for-byte from
    PS.EXE by ``c2 delink``: ``av.obj`` — RAD Smacker + Miles AIL +
    RAD file I/O (one object; they share scratch data);
  * the Watcom 10.0a CRT (``clib3r.lib``), pulled in by ``SYSTEM dos4g``
    so ``_cstart_`` is the entry and calls the recovered ``main`` (c2.c);
  * PS.EXE's own DOS/4GW Professional 1.97 stub, lifted verbatim and
    prepended to the freshly-linked LE (see the bind step below).

**No auto-stubbing.**  Unlike the verifier, ``rebuild`` links exactly
what the corpus provides: an extern that nothing defines is a hard
wlink "undefined symbol" error, surfaced verbatim.  The rebuild is
self-sustained or it does not build — recover the missing definition
(or fix the reference) instead of papering over it.

Work dir: ``.c2-cache/rebuild`` (gitignored, incremental; delete it to
force a cold rebuild — ~minutes instead of ~1 s warm).

Usage:
    c2 rebuild                      # -> build/PS.EXE
    c2 rebuild -o /tmp/PS.EXE
    c2 rebuild --no-bind            # stop at the wlink LE (c2_x.exe)
"""

from __future__ import annotations

import bisect
import json
import re
import shutil
import time
from pathlib import Path
from typing import Annotated

import typer
from reccmp.formats.watcom_debug import parse_watcom_debug_file

from c2 import le as le_mod
from c2.buildenv import (
    PS_CFLAGS,
    _STOCK_IMAGE,
    _compare_bytes,
    _run_in_container,
    _write_if_changed,
)
from c2.commands.delink import (
    _delink,
    _load_context,
    _mod_base,
    _resolve_modules,
    _verify_verbatim,
)
from c2.commands.export import ensure_symbols_json
from c2.commands.fixups import canonicalize_le_fixup_record_order
from c2.original import ensure_original
from c2.parsers.omf import rewrite_pubdef_offsets
from c2.reccmp_project import publish_build_artifacts
from c2.watcom_dbg import rebuild_watcom_global_symbol_table

_REBUILD_DIR = Path(".c2-cache/rebuild")


# Scaffold TUs whose module is delinked wholesale: the real code AND the
# real data (e.g. unsmack's `simspeed` overlay, sndail's `sndinit`) come
# from av.obj; compiling the scaffold too would double-define the data.
_EXCLUDED_TUS = {
    "smackinp.c", "sndail.c", "sndnull.c",
    # Native Windows platform modules are retained as source oracles, but
    # are not inputs to the DOS executable.
    "winadvisor.c", "winaudio.c", "windialog.c", "winfile.c",
    "wingdi.c", "winmenu.c", "winplatform.c",
    # All recovered initializers now live in their owning TUs.  This file is
    # declarations/comments only; compiling it creates a synthetic empty
    # debug module that PS never had.
    "datainit.c",
}

# The eight hand-written C2 asm modules (same set the verifier builds).
_C2_ASM_NAMES = {
    "library.asm", "sprites.asm", "dia_ptrs.asm",
    "dialarga.asm", "dialargb.asm", "dia_medi.asm", "dia_smal.asm",
    "palet.asm",
}

# The delink group whose modules are linked as per-module objects
# (--split): mirrors the original build's separate library objects and
# lets the .lnk interleave them in PS.EXE's module order.
_DELINK_GROUP = "av"

# The reconstructed vendor archives and their members, exactly as the
# 1995 link consumed them.  dllload/sndail are NOT members: PS's module
# list places them at indexes 5-6, i.e. the original linked them as
# LOOSE SDK-glue objects.  palet.obj (module 64) IS a Smacker SDK
# library member (consistent with its MASM reg-reg direction bits).
_LIB_MEMBERS: dict[str, list[str]] = {
    "ail.lib": ["aildebug", "aila", "ail", "ailss", "ailsfile",
                "ailxmidi", "ailxdig", "ailssa"],
    "smack.lib": ["smackinp", "palet", "unsmack", "rfile", "sndnull",
                  "qread"],
}


def _delink_objs(work: Path, symbols_json: Path, exe_path: Path,
                 verify: bool) -> tuple[dict[int, str], list[str]]:
    """(Re-)delink the third-party modules into per-module objs in *work*.

    Returns ({module_index: obj_name}, summary lines).
    """

    ctx = _load_context(symbols_json, exe_path)
    d = ctx[0]
    module_indices = _resolve_modules(d, [], _DELINK_GROUP)
    parts, total = _delink(*ctx, module_indices=module_indices,
                           module_name=_DELINK_GROUP, split=True)
    if verify:
        checked, bad = _verify_verbatim(parts, total)
        if bad:
            raise RuntimeError(
                f"delink verbatim check FAILED: {bad}/{checked} bytes differ")
        total["verify_checked"] = checked
    for p in parts:
        _write_if_changed(work / f"{p.name}.obj", p.o.build())
    mod_to_obj = {mi: f"{_mod_base(d['modules'], mi)}.obj"
                  for mi in module_indices}
    lines = [
        f"{_DELINK_GROUP} (split, {len(parts)} objs): "
        f"{total['functions']} funcs, "
        f"{total['publics']}+{total['data_publics']} publics"
        + (f", {total['synthetics']} synthetic label(s)"
           if total.get("synthetics") else "")
        + f", _TEXT {total['text_bytes']}b _DATA {total['data_bytes']}b"
        + (f", verbatim {total['verify_checked']}b OK" if verify else "")
    ]
    return mod_to_obj, lines


def _map_symbols(map_path: Path) -> tuple[dict[str, int], dict[str, int]]:
    """Parse a WLINK .map → ({code_name: off}, {data_name: off}).

    Offsets are object-relative (segment 0001 = code, 0002 = DGROUP), the
    same coordinate system as symbols.json offsets and the LE loaders.
    """
    code_m, data_m = _map_symbols_multi(map_path)
    return ({n: offs[-1] for n, offs in code_m.items()},
            {n: offs[-1] for n, offs in data_m.items()})


def _map_symbols_multi(map_path: Path) -> tuple[dict[str, list[int]],
                                                dict[str, list[int]]]:
    """Duplicate-name-aware WLINK .map parse → name → [offsets…] (file
    order).  Needed because a symbol name can legitimately exist twice
    (the game public `_dig` in pcsound.c AND the AIL vendor static
    `_dig` in ailss.c); a flat dict silently collapses them and any
    name-only mapper then reports the vendor one against the game
    public's address (TODO §4)."""
    code: dict[str, list[int]] = {}
    data: dict[str, list[int]] = {}
    rx = re.compile(r"^000([12]):([0-9A-Fa-f]{8})[* +]?\s+(\S+)")
    for line in map_path.read_text(errors="replace").splitlines():
        m = rx.match(line)
        if m:
            d = code if m.group(1) == "1" else data
            d.setdefault(m.group(3), []).append(int(m.group(2), 16))
    return code, data


def _bucket_of(mod_name: str) -> str:
    """Classify a symbols.json module into a comparison bucket."""
    u = (mod_name or "").upper()
    if "D:\\C2\\CODE" in u:
        return "game"
    base = u.rsplit("\\", 1)[-1].lower()
    if base in {n.lower() for n in _C2_ASM_NAMES} | {"palet.asm"}:
        return "c2-asm"
    if ("SMACK\\20" in u or "NET\\LIBS\\AIL" in u
            or base in ("unsmack.asm", "aila.asm", "ailssa.asm",
                        "rfile.asm", "qread")):
        return "av-delink"
    return "crt"


def _strict_code_sites(o_code: bytes, r_code: bytes,
                       o_cfix: set[int], r_cfix: set[int],
                       ps_syms: list[tuple[int, str]],
                       rc_syms: list[tuple[int, str]]) -> tuple[int, list[dict]]:
    """STRICT whole-code-object comparison (TODO §1): mask ONLY the LE
    loader-fixup fields, leave relative branch displacements VISIBLE, and
    decode every differing byte to its containing rel branch with the
    resolved PS and RC canonical targets.  This is the metric the normal
    per-function oracle is intentionally blind to (ComTail canonical
    tail choice).

    Returns (diff_byte_count, [site dicts]).
    """

    n = min(len(o_code), len(r_code))
    diffs = [i for i in range(n)
             if i not in o_cfix and i not in r_cfix
             and o_code[i] != r_code[i]]

    def _sym(table: list[tuple[int, str]], off: int) -> str:
        i = bisect.bisect_right(table, (off, "\uffff")) - 1
        if i < 0:
            return f"{off:#x}"
        base, name = table[i]
        name = name[:-1] if name.endswith("_") else name
        d = off - base
        return f"{name}+{d:#x}" if d else name

    def _decode_branch(code: bytes, diff_off: int):
        """Find the rel branch whose displacement field covers diff_off.
        Earliest-start-first so a 6-byte 0F 8x form beats a bogus inner
        7x byte match.  Returns (start, mnemonic, disp_lo, disp_hi, target)."""
        _CC = ["o", "no", "b", "ae", "e", "ne", "be", "a",
               "s", "ns", "p", "np", "l", "ge", "le", "g"]
        for start in range(max(diff_off - 5, 0), diff_off + 1):
            op = code[start]
            if op == 0x0F and start + 6 <= len(code) \
                    and 0x80 <= code[start + 1] <= 0x8F \
                    and start + 2 <= diff_off < start + 6:
                disp = int.from_bytes(code[start + 2:start + 6],
                                      "little", signed=True)
                return (start, "j" + _CC[code[start + 1] - 0x80],
                        start + 2, start + 6, start + 6 + disp)
            if op in (0xE9, 0xE8) and start + 5 <= len(code) \
                    and start + 1 <= diff_off < start + 5:
                disp = int.from_bytes(code[start + 1:start + 5],
                                      "little", signed=True)
                return (start, "jmp" if op == 0xE9 else "call",
                        start + 1, start + 5, start + 5 + disp)
            if op in (0xEB, *range(0x70, 0x80)) and diff_off == start + 1:
                disp = code[start + 1] - (0x100 if code[start + 1] >= 0x80
                                          else 0)
                mnem = "jmp short" if op == 0xEB else "j" + _CC[op - 0x70]
                return (start, mnem, start + 1, start + 2, start + 2 + disp)
        return None

    sites: dict[int, dict] = {}
    for off in diffs:
        bo = _decode_branch(o_code, off)
        br = _decode_branch(r_code, off)
        if bo is None or br is None or bo[0] != br[0]:
            # not a decodable rel-branch displacement: report raw
            sites.setdefault(-off, {
                "off": off, "caller": _sym(ps_syms, off), "mnem": "??",
                "ps_target": f"{o_code[off]:#04x}",
                "rc_target": f"{r_code[off]:#04x}", "bytes": 1,
            })
            continue
        s = sites.get(bo[0])
        if s is None:
            sites[bo[0]] = {
                "off": bo[0], "caller": _sym(ps_syms, bo[0]),
                "mnem": bo[1],
                "ps_target": _sym(ps_syms, bo[4]),
                "rc_target": _sym(rc_syms, br[4]),
                "bytes": 1,
            }
        else:
            s["bytes"] += 1
    return len(diffs), [sites[k] for k in sorted(sites, key=abs)]


def _compare_vs_original(work: Path, symbols_json: Path, exe_path: Path,
                         verbose: bool = False) -> dict:
    """Byte-compare the rebuilt LE against the original, per symbol.

    Code: every named function (game / c2-asm / av-delink / crt), compared
    over min(orig span, rebuild span) with LE fixup fields and rel32
    call/jmp displacements masked on both sides (see
    ``c2.buildenv._compare_bytes``), applied to the FINAL link.

    Data: every named file-backed data symbol, masked by data fixup
    fields (pointers relocate differently by construction).

    Returns the summary dict (also printed).
    """

    # Original: symbols.json-driven (its memory_map sizes ARE the exe's).
    d, o_code, o_data, o_cvsize, o_dvsize, o_dfsize, o_cfm, o_dfm = _load_context(
        symbols_json, exe_path)

    # Rebuild: everything from its own LE header.
    rexe = work / "c2_x.exe"
    rimg = le_mod.load_le(rexe)
    rraw = rimg.data
    r_code = rraw[le_mod.object_file_offset(rimg, 0):
                  le_mod.object_file_offset(rimg, 0)
                  + le_mod.object_file_size(rimg, 0)]
    r_dfsize = le_mod.object_file_size(rimg, 1)
    r_data2 = rraw[le_mod.object_file_offset(rimg, 1):
                   le_mod.object_file_offset(rimg, 1) + r_dfsize]
    r_cfm, r_dfm = le_mod.segment_fixup_maps(rimg)
    o_cfix = {off + k for off in o_cfm for k in range(4)}
    r_cfix = {off + k for off in r_cfm for k in range(4)}
    o_dfix = {off + k for off in o_dfm for k in range(4)}
    r_dfix = {off + k for off in r_dfm for k in range(4)}

    map_code, map_data = _map_symbols(work / "ps.map")
    mods = d["modules"]
    alld = sorted([s for s in d["symbols"] if s.get("is_data")],
                  key=lambda s: s["offset"])
    dstarts = [s["offset"] for s in alld]

    # ── code symbols, deduped by offset (aliases keep all names) ──────
    by_off: dict[int, list[dict]] = {}
    for s in d["symbols"]:
        if s.get("is_code"):
            by_off.setdefault(s["offset"], []).append(s)
    offs = sorted(by_off)
    rc_code_offs = sorted(map_code.values())

    def rc_span(off: int) -> int:
        i = bisect.bisect_right(rc_code_offs, off)
        end = rc_code_offs[i] if i < len(rc_code_offs) else len(r_code)
        return end - off

    # Resolve every PS code symbol to a rebuild offset:
    #   1. by linker name (the .map);
    #   2. statics with no linker name are ANCHORED off a resolved
    #      neighbour — forward from the previous, then backward from the
    #      next — when layout is provably preserved between them:
    #        * same module (a .obj is placed as a unit), or
    #        * both av-delink with PS-contiguity (delinked clusters are
    #          verbatim byte runs).
    entries: list[dict] = []
    for i, off in enumerate(offs):
        end = offs[i + 1] if i + 1 < len(offs) else o_cvsize
        if end - off <= 0:
            continue
        aliases = by_off[off]
        mod_idx = aliases[0]["module_index"]
        rc_off = None
        name = aliases[0]["raw_name"]
        for a in aliases:
            if a["raw_name"] in map_code:
                rc_off = map_code[a["raw_name"]]
                name = a["raw_name"]
                break
        entries.append({
            "off": off, "span": end - off, "name": name, "mod": mod_idx,
            "bucket": _bucket_of(mods[mod_idx].get("name") or ""),
            "rc": rc_off, "anchored": False,
        })

    def _anchorable(a: dict, b: dict) -> bool:
        return (a["mod"] == b["mod"]
                or (a["bucket"] == "av-delink" and b["bucket"] == "av-delink"))

    for i, e in enumerate(entries):          # forward
        if e["rc"] is None and i > 0:
            p = entries[i - 1]
            if p["rc"] is not None and _anchorable(e, p):
                cand = p["rc"] + (e["off"] - p["off"])
                if 0 <= cand < len(r_code):
                    e["rc"], e["anchored"] = cand, True
    for i in range(len(entries) - 2, -1, -1):  # backward
        e = entries[i]
        if e["rc"] is None:
            nx = entries[i + 1]
            if nx["rc"] is not None and _anchorable(e, nx):
                cand = nx["rc"] - (nx["off"] - e["off"])
                if 0 <= cand < len(r_code):
                    e["rc"], e["anchored"] = cand, True
    # last resort for anchor-less statics: the POINTER CHASE.  Symbols
    # like crwd386's __null_FPE_rtn (a single ret byte) or noefgfmt's
    # _no_support_loaded exist only to be POINTED AT — so PS carries a
    # fixup targeting them from a site we CAN translate (a named data
    # slot like __FPE_handler / ___EFG_printf, or a resolved function).
    # Translate the site into the rebuild, read the rebuild's own fixup
    # there, and the target IS the symbol's rebuild address — exact, no
    # heuristics.  The subsequent byte compare is the verification.
    ps_data_sym_of = {}
    for s in alld:
        ps_data_sym_of[s["offset"]] = s
    dsym_starts = [s["offset"] for s in alld]

    def _rc_data_site(ps_site: int) -> int | None:
        i = bisect.bisect_right(dsym_starts, ps_site) - 1
        if i < 0:
            return None
        s = alld[i]
        rc = map_data.get(s["raw_name"])
        if rc is None:
            return None
        return rc + (ps_site - s["offset"])

    ent_starts = [e["off"] for e in entries]

    def _rc_code_site(ps_site: int) -> int | None:
        i = bisect.bisect_right(ent_starts, ps_site) - 1
        if i < 0:
            return None
        e = entries[i]
        if e["rc"] is None or ps_site >= e["off"] + e["span"]:
            return None
        return e["rc"] + (ps_site - e["off"])

    # reverse index: PS target -> referencing fixup sites
    code_refs: dict[int, list[tuple[str, int]]] = {}
    for site, (tobj, toff) in o_cfm.items():
        if tobj == 1:
            code_refs.setdefault(toff, []).append(("c", site))
    for site, (tobj, toff) in o_dfm.items():
        if tobj == 1:
            code_refs.setdefault(toff, []).append(("d", site))

    def _chase_code(ps_off: int) -> int | None:
        for kind, site in code_refs.get(ps_off, [])[:16]:
            if kind == "d":
                rc_site = _rc_data_site(site)
                hit = r_dfm.get(rc_site) if rc_site is not None else None
            else:
                rc_site = _rc_code_site(site)
                hit = r_cfm.get(rc_site) if rc_site is not None else None
            if hit is not None and hit[0] == 1:
                return hit[1]
        return None

    for e in entries:
        if e["rc"] is None:
            rc = _chase_code(e["off"])
            if rc is not None and 0 <= rc < len(r_code):
                e["rc"], e["anchored"] = rc, True

    stats: dict[str, dict] = {}
    difflist: dict[str, list[str]] = {}
    missing: dict[str, list[str]] = {}
    for e in entries:
        st = stats.setdefault(e["bucket"], {"exact": 0, "diff": 0, "missing": 0,
                                            "anchored": 0, "diff_bytes": 0,
                                            "tail": 0})
        e["status"] = "exact"
        e["diff_bytes"] = 0
        if e["rc"] is None:
            st["missing"] += 1
            e["status"] = "missing"
            missing.setdefault(e["bucket"], []).append(e["name"])
            continue
        if e["anchored"]:
            st["anchored"] += 1
        n = min(e["span"], rc_span(e["rc"]))
        oslice = o_code[e["off"]:e["off"] + n]
        diffs = _compare_bytes(oslice, r_code[e["rc"]:e["rc"] + n],
                               e["off"], e["rc"], o_cfix, r_cfix)
        if diffs:
            # Span-overreach artefact: a symbol's gap-to-next-symbol span
            # can swallow trailing unnamed bytes (alignment filler, switch
            # tables, the next module's SYM_TEMP pool).  If every diff
            # lies AFTER the function's last ret, it's positional noise,
            # not a code diff.
            first_diff = min(diffs)
            last_ret = max((k for k in range(len(oslice))
                            if oslice[k] in (0xC3, 0xC2)), default=-1)
            has_ret_before = any(oslice[k] in (0xC3, 0xC2)
                                 for k in range(first_diff))
            if has_ret_before and (all(x > last_ret for x in diffs)
                                   or n - first_diff <= 16):
                st["tail"] += 1
                e["status"] = "tail"
            else:
                st["diff"] += 1
                st["diff_bytes"] += len(diffs)
                e["status"], e["diff_bytes"] = "diff", len(diffs)
                difflist.setdefault(e["bucket"], []).append(
                    f"{e['name']}({len(diffs)}b)")
        else:
            st["exact"] += 1

    # ── named initialized data ──────────────────────────────────────
    rc_data_offs = sorted(map_data.values())
    dentries: list[dict] = []
    for i, s in enumerate(alld):
        off = s["offset"]
        if off >= o_dfsize:
            continue                          # BSS: no content to compare
        if s["raw_name"] == "__nullarea":
            # Its gap-to-next-symbol span swallows the unnamed string-literal
            # pool, which relocates freely in the rebuild (documented in
            # data_init.py) — a positional artefact, not a data diff.
            continue
        end = dstarts[i + 1] if i + 1 < len(dstarts) else o_dfsize
        span = min(end, o_dfsize) - off
        if span <= 0:
            continue
        rc_off = map_data.get(s["raw_name"])
        if rc_off is not None and rc_off >= r_dfsize:
            rc_off = None                     # named but BSS-placed: a diff
        dentries.append({"off": off, "span": span, "name": s["raw_name"],
                         "mod": s["module_index"], "rc": rc_off,
                         "named": s["raw_name"] in map_data,
                         "anchored": False})
    # Pointer-chase first (exact): a static's rebuild address is read off
    # the rebuild's own fixup at a translatable referencing site.
    data_refs: dict[int, list[tuple[str, int]]] = {}
    for site, (tobj, toff) in o_cfm.items():
        if tobj == 2:
            data_refs.setdefault(toff, []).append(("c", site))
    for site, (tobj, toff) in o_dfm.items():
        if tobj == 2:
            data_refs.setdefault(toff, []).append(("d", site))

    def _chase_data(ps_off: int, span: int) -> int | None:
        # References may carry a folded displacement (e.g. dosret's
        # `xlat[i - 3]` fixups target _xlat-3), so chase any target in a
        # small window around the symbol and correct by the delta —
        # NEAREST FIRST (an exact/interior hit must win over a hit that
        # really belongs to the preceding symbol's tail).
        cands = list(range(ps_off, ps_off + span)) \
            + [ps_off - k for k in range(1, 17)]
        for t in cands:
            for kind, site in data_refs.get(t, [])[:16]:
                if kind == "c":
                    rc_site = _rc_code_site(site)
                    hit = r_cfm.get(rc_site) if rc_site is not None else None
                else:
                    rc_site = _rc_data_site(site)
                    hit = r_dfm.get(rc_site) if rc_site is not None else None
                if hit is not None and hit[0] == 2:
                    return hit[1] + (ps_off - t)
        return None

    for e in dentries:
        if e["rc"] is None and not e["named"]:
            rc = _chase_data(e["off"], e["span"])
            if rc is not None and 0 <= rc < r_dfsize:
                e["rc"], e["anchored"] = rc, True
    # Same-module adjacency anchoring for whatever remains: a module's
    # _DATA is one segment in its object, so PS-adjacent same-module data
    # symbols keep their relative offsets in the rebuild.
    for i, e in enumerate(dentries):          # forward
        if e["rc"] is None and i > 0:
            p = dentries[i - 1]
            if (p["rc"] is not None and p["mod"] == e["mod"]
                    and p["off"] + p["span"] == e["off"]):
                e["rc"] = p["rc"] + (e["off"] - p["off"])
                e["anchored"] = True
    for i in range(len(dentries) - 2, -1, -1):  # backward
        e = dentries[i]
        if e["rc"] is None:
            nx = dentries[i + 1]
            if (nx["rc"] is not None and nx["mod"] == e["mod"]
                    and e["off"] + e["span"] == nx["off"]):
                e["rc"] = nx["rc"] - (nx["off"] - e["off"])
                e["anchored"] = True

    dstat = {"exact": 0, "diff": 0, "missing": 0, "anchored": 0}
    ddiff: list[str] = []
    dmissing: list[str] = []
    for e in dentries:
        off, span, rc_off = e["off"], e["span"], e["rc"]
        e["status"] = "exact"
        if rc_off is None or rc_off >= r_dfsize:
            e["status"] = "diff" if e["named"] else "missing"
            if e["named"]:
                # in the map but placed in BSS while PS initializes it:
                # a REAL missing-initializer bug (e.g. the smacks movie
                # table), not a resolution failure — count as diff.
                dstat["diff"] += 1
                ddiff.append(f"{e['name']}(uninitialized)")
            else:
                dstat["missing"] += 1
                dmissing.append(e["name"])
            continue
        if e["anchored"]:
            dstat["anchored"] += 1
        j = bisect.bisect_right(rc_data_offs, rc_off)
        rc_end = rc_data_offs[j] if j < len(rc_data_offs) else r_dfsize
        n = min(span, min(rc_end, r_dfsize) - rc_off)
        bad = 0
        for k in range(n):
            if (off + k) in o_dfix or (rc_off + k) in r_dfix:
                continue
            if o_data[off + k] != r_data2[rc_off + k]:
                bad += 1
        if bad:
            dstat["diff"] += 1
            e["status"] = "diff"
            ddiff.append(f"{e['name']}({bad}b)")
        else:
            dstat["exact"] += 1

    # ── report ──────────────────────────────────────────────────────
    typer.echo(f"  compare vs {exe_path} (fixup+rel32 masked):")
    order = ["game", "c2-asm", "av-delink", "crt"]
    for b in order + sorted(set(stats) - set(order)):
        if b not in stats:
            continue
        st = stats[b]
        total = st["exact"] + st["diff"] + st["missing"] + st.get("tail", 0)
        line = (f"    {b:10} {st['exact']}/{total} exact"
                + (f", {st['diff']} diff ({st['diff_bytes']}b)" if st["diff"] else "")
                + (f", {st['tail']} ~tail" if st.get("tail") else "")
                + (f", {st['missing']} unmatched" if st["missing"] else "")
                + (f"  [{st['anchored']} static(s) anchored]"
                   if st.get("anchored") else ""))
        typer.echo(line)
        if verbose:
            if difflist.get(b):
                typer.echo("      diff: " + ", ".join(difflist[b][:20])
                           + (" ..." if len(difflist[b]) > 20 else ""))
            if missing.get(b):
                typer.echo("      missing: " + ", ".join(missing[b][:20])
                           + (" ..." if len(missing[b]) > 20 else ""))
    # layout order: functions appearing OUT of PS's address order in the
    # rebuild.  Cross-module breaks = link-order infidelity (want 0: the
    # .lnk FILE list follows PS's module order); within-module breaks =
    # the recovered TU defines its functions in a different order than
    # the original source did (source-reorganization worklist).
    within = cross = 0
    last_rc = -1
    last_mod = None
    for e in entries:
        if e["rc"] is None:
            continue
        if e["rc"] < last_rc:
            if e["mod"] == last_mod:
                within += 1
            else:
                cross += 1
        else:
            last_rc = e["rc"]
        last_mod = e["mod"]
    typer.echo(f"    layout     module order: {cross} cross-module break(s); "
               f"{within} within-module function-order break(s)")

    dtotal = dstat["exact"] + dstat["diff"] + dstat["missing"]
    typer.echo(f"    data       {dstat['exact']}/{dtotal} named initialized "
               f"symbols exact"
               + (f", {dstat['diff']} diff" if dstat["diff"] else "")
               + (f", {dstat['missing']} unmatched" if dstat["missing"] else "")
               + (f"  [{dstat['anchored']} static(s) anchored]"
                  if dstat["anchored"] else ""))
    if verbose and ddiff:
        typer.echo("      data diff: " + ", ".join(ddiff[:20])
                   + (" ..." if len(ddiff) > 20 else ""))
    if verbose and dmissing:
        typer.echo("      data unmatched: " + ", ".join(dmissing[:20])
                   + (" ..." if len(dmissing) > 20 else ""))
    r_cvsize = rimg.sections[0].virtual_size
    r_dvsize = rimg.sections[1].virtual_size
    typer.echo(f"    LE sizes   code(vsize) {r_cvsize}b vs {o_cvsize}b"
               f" · data(file) {r_dfsize}b vs {o_dfsize}b"
               f" · data(vsize) {r_dvsize}b vs {o_dvsize}b")
    # ── PLACEMENT: every named symbol at its exact PS offset ────────
    # Code starts: a PS code symbol is exactly placed when the rebuild
    # map lists (any alias of) its name at the SAME object offset.  A
    # mismatch whose PS-position bytes are nevertheless identical is a
    # public/debug LABEL alias (the `sound_error_` class: wlink resolves
    # the public to an identical `ret` elsewhere while the loaded byte
    # at the PS address is already correct) — reported as ~alias, not a
    # code-order break.
    code_multi, data_multi = _map_symbols_multi(work / "ps.map")
    st_exact = st_alias = 0
    st_breaks: list[str] = []
    for e in entries:
        cands: set[int] = set()
        for a in by_off[e["off"]]:
            cands.update(code_multi.get(a["raw_name"], []))
        if e["off"] in cands or (not cands and e["rc"] == e["off"]):
            st_exact += 1
            continue
        n = min(e["span"], 64)
        same = all(
            (e["off"] + k) in o_cfix or (e["off"] + k) in r_cfix
            or (e["off"] + k < len(r_code)
                and o_code[e["off"] + k] == r_code[e["off"] + k])
            for k in range(n))
        if same:
            st_alias += 1
            rc_note = (f" RC→0x{min(cands):x}" if cands else "")
            st_breaks.append(f"~{e['name'].rstrip('_')}(alias{rc_note})")
        else:
            st_breaks.append(e["name"].rstrip("_"))
    # Data placement (initialized + BSS): module-aware duplicate-name
    # pairing — game-bucket symbols claim map entries first; a vendor
    # (av-delink/crt) static whose name is fully claimed is placement-
    # verified by the verbatim delink blob, not by name (the `_dig`
    # class), and is counted as a skipped static.
    dp_exact = dp_static = 0
    dp_breaks: list[str] = []
    by_name: dict[str, list[dict]] = {}
    for s in alld:
        by_name.setdefault(s["raw_name"], []).append(s)
    for name, group in by_name.items():
        offs = list(data_multi.get(name, []))
        # game-bucket first (map publics are overwhelmingly first-party)
        group = sorted(group, key=lambda s: (
            _bucket_of(mods[s["module_index"]].get("name") or "") != "game",
            s["offset"]))
        for s in group:
            if s["offset"] in offs:
                offs.remove(s["offset"])
                dp_exact += 1
            elif not offs or _bucket_of(
                    mods[s["module_index"]].get("name") or "") != "game":
                dp_static += 1        # unmapped static / vendor twin
            else:
                dp_breaks.append(f"{name}(PS 0x{s['offset']:x} "
                                 f"RC 0x{offs[0]:x})")
    typer.echo(f"    placement  code starts {st_exact}/{len(entries)} exact"
               + (f", {st_alias} ~alias" if st_alias else "")
               + (f", {len(st_breaks) - st_alias} BREAK"
                  if len(st_breaks) > st_alias else "")
               + f" · data {dp_exact}/{dp_exact + len(dp_breaks)} named exact"
               + (f", {len(dp_breaks)} MISPLACED" if dp_breaks else "")
               + (f" [{dp_static} static(s) via delink/anchor]"
                  if dp_static else ""))
    if st_breaks and (verbose or len(st_breaks) <= 6):
        typer.echo("      starts: " + ", ".join(st_breaks[:12])
                   + (" ..." if len(st_breaks) > 12 else ""))
    if dp_breaks and (verbose or len(dp_breaks) <= 6):
        typer.echo("      data misplaced: " + ", ".join(dp_breaks[:12])
                   + (" ..." if len(dp_breaks) > 12 else ""))
    # ── STRICT code stream: loader fixups masked, rel branches VISIBLE ──
    ps_syms = sorted((off, by_off[off][0]["raw_name"]) for off in by_off)
    rc_syms = sorted((e["rc"], e["name"]) for e in entries
                     if e["rc"] is not None)
    strict_bytes, strict_sites = _strict_code_sites(
        o_code[:o_cvsize], r_code[:o_cvsize], o_cfix, r_cfix,
        ps_syms, rc_syms)
    typer.echo(f"    strict     {strict_bytes} differing code byte(s) / "
               f"{min(o_cvsize, len(r_code))} (rel branches visible)"
               + (f" across {len(strict_sites)} branch site(s)"
                  if strict_sites else ""))
    if strict_sites and (verbose or len(strict_sites) <= 12):
        for s in strict_sites:
            typer.echo(f"      {s['caller']} {s['mnem']} @{s['off']:#x}: "
                       f"PS→{s['ps_target']}  RC→{s['rc_target']}  "
                       f"({s['bytes']}b)")
    # ── FIXUP TARGETS: the masked bytes still have semantic identity ──
    # The strict stream deliberately masks loader-patched fields, but an
    # identical fixup SITE is not sufficient: it must also point at the
    # same object+offset.  A wrong global/field reference has identical
    # opcodes and is otherwise completely hidden by that mask.
    def _fixup_audit(ps: dict[int, tuple[int, int]],
                     rc: dict[int, tuple[int, int]]) -> dict:
        psites, rsites = set(ps), set(rc)
        common = psites & rsites
        mismatches = [
            {"site": site, "ps_target": ps[site], "rc_target": rc[site]}
            for site in sorted(common) if ps[site] != rc[site]
        ]
        return {
            "ps_sites": len(psites), "rc_sites": len(rsites),
            "only_ps": sorted(psites - rsites),
            "only_rc": sorted(rsites - psites),
            "target_mismatches": mismatches,
        }

    cfa = _fixup_audit(o_cfm, r_cfm)
    dfa = _fixup_audit(o_dfm, r_dfm)
    typer.echo(
        f"    fixups     code sites {cfa['ps_sites']}/{cfa['rc_sites']}"
        f" ({len(cfa['only_ps'])} PS-only, {len(cfa['only_rc'])} RC-only)"
        f" · {len(cfa['target_mismatches'])} target mismatch(es)"
        f"; data sites {dfa['ps_sites']}/{dfa['rc_sites']}"
        f" ({len(dfa['only_ps'])} PS-only, {len(dfa['only_rc'])} RC-only)"
        f" · {len(dfa['target_mismatches'])} target mismatch(es)")

    def _fixup_sym(target: tuple[int, int]) -> str:
        obj, off = target
        table = ps_syms if obj == 1 else [
            (s["offset"], s["raw_name"]) for s in alld
        ] if obj == 2 else []
        if not table:
            return f"obj{obj}:{off:#x}"
        i = bisect.bisect_right(table, (off, "\uffff")) - 1
        if i < 0:
            return f"obj{obj}:{off:#x}"
        base, name = table[i]
        delta = off - base
        return name.rstrip("_") + (f"+{delta:#x}" if delta else "")

    for audit in (cfa, dfa):
        for mismatch in audit["target_mismatches"]:
            mismatch["ps_target_name"] = _fixup_sym(mismatch["ps_target"])
            mismatch["rc_target_name"] = _fixup_sym(mismatch["rc_target"])

    if verbose:
        for label, audit, callers in (
                ("code", cfa, ps_syms), ("data", dfa, [])):
            for m in audit["target_mismatches"][:80]:
                site = m["site"]
                if label == "code":
                    i = bisect.bisect_right(callers, (site, "\uffff")) - 1
                    caller = callers[i][1].rstrip("_") if i >= 0 else f"{site:#x}"
                else:
                    caller = f"data+{site:#x}"
                typer.echo(
                    f"      {label} fixup {caller}+{site - callers[i][0]:#x}"
                    if label == "code" and i >= 0 else
                    f"      {label} fixup {caller}", nl=False)
                typer.echo(f": PS→{m['ps_target_name']} "
                           f"RC→{m['rc_target_name']}")
            if len(audit["target_mismatches"]) > 80:
                typer.echo(f"      ... {len(audit['target_mismatches']) - 80} "
                           f"more {label} target mismatch(es)")
    return {"code": stats, "data": dstat, "strict_code_bytes": strict_bytes,
            "strict_sites": strict_sites,
            "fixups": {"code": cfa, "data": dfa},
            "placement": {"starts_exact": st_exact, "starts_alias": st_alias,
                          "starts_breaks": st_breaks,
                          "data_exact": dp_exact, "data_static": dp_static,
                          "data_misplaced": dp_breaks},
            "diff": difflist, "missing": missing, "data_diff": ddiff,
            "entries": entries, "dentries": dentries,
            "sizes": {"o_code": o_cvsize, "r_code": r_cvsize,
                       "o_dfile": o_dfsize, "r_dfile": r_dfsize,
                       "o_dvsize": o_dvsize, "r_dvsize": r_dvsize}}


def rebuild(
    output: Annotated[Path, typer.Option(
        "--output", "-o",
        help="Bound output path (never the original).")] = Path("build/PS.EXE"),
    image: Annotated[str, typer.Option(
        "--image", help="Watcom 10.0a compile image.")] = _STOCK_IMAGE,
    bind: Annotated[bool, typer.Option(
        "--bind/--no-bind",
        help="Bind DOS/4GW Professional into the exe (PS.EXE-style).")] = True,
    verify_delink: Annotated[bool, typer.Option(
        "--verify-delink/--no-verify-delink",
        help="Run the delink verbatim byte check.")] = True,
    stack: Annotated[str, typer.Option(
        "--stack", help="wlink OPTION STACK size (PS used wlink's "
        "default 4k for 32-bit protected mode; DOS/4GW page-grows it "
        "at runtime, so 4k reproduces PS's committed DGROUP vsize).")
    ] = "4k",
    symbols_json: Annotated[Path, typer.Option(
        "--symbols", help="symbols.json path (auto-regenerated when stale).")] = Path(".c2-cache/symbols.json"),
    exe_path: Annotated[Path, typer.Option(
        "--exe", help="PS.EXE path.")] = Path("original/PS.EXE"),
    src_dir: Annotated[Path, typer.Option(
        "--src", help="Recovered source dir.")] = Path("src"),
    include_dir: Annotated[Path, typer.Option(
        "--include", help="Header dir.")] = Path("include"),
    compare: Annotated[bool, typer.Option(
        "--compare/--no-compare",
        help="After linking, byte-compare the rebuild against the original "
             "PS.EXE per function/data symbol (fixup+rel32 masked).")] = True,
    compare_verbose: Annotated[bool, typer.Option(
        "--compare-verbose", "-cv",
        help="List the diffing / missing symbols in the comparison.")] = False,
    publish_reccmp: Annotated[bool, typer.Option(
        "--reccmp/--no-reccmp",
        help="Publish the pre-bind image, map, and reccmp build discovery.")
    ] = True,
    reccmp_config: Annotated[Path, typer.Option(
        "--reccmp-config",
        help="Generated reccmp build-discovery file.")
    ] = Path("reccmp-build.yml"),
    cflags: Annotated[str, typer.Option(
        "--cflags",
        help="Compiler flags for recovered C sources.")] = PS_CFLAGS,
) -> dict | None:
    """Build a runnable PS.EXE from recovered source + delinked objects."""
    t_start = time.perf_counter()
    if output.resolve() == exe_path.resolve():
        raise typer.BadParameter("refusing to overwrite the original")
    ensure_original(exe_path)
    ensure_symbols_json(exe_path, symbols_json)
    work = _REBUILD_DIR
    work.mkdir(parents=True, exist_ok=True)
    compile_cflags = cflags + " -zq"

    # ── 1. stage headers / sources / asm (verbatim, mtime-stable) ──────
    # No stub-stripping and no stubs.c: the rebuild compiles the corpus
    # exactly as recovered, and anything undefined is a link ERROR.
    header_files = sorted(include_dir.glob("*.h"))
    for h in header_files:
        _write_if_changed(work / h.name, h.read_bytes())

    c_files = sorted(f for f in src_dir.glob("*.c")
                     if f.name not in _EXCLUDED_TUS)
    asm_files = sorted(f for f in src_dir.glob("*.asm")
                       if f.name in _C2_ASM_NAMES)
    for cf in c_files:
        _write_if_changed(work / cf.name, cf.read_bytes())
    for af in asm_files:
        _write_if_changed(work / af.name, af.read_bytes())
    # Drop stale staged files from earlier runs (e.g. stubs.c from the
    # auto-stub era, or a TU that moved to _EXCLUDED_TUS): a leftover
    # .c/.obj must never leak into the link.
    keep = ({f.name for f in c_files} | {f.name for f in asm_files})
    for stale in list(work.glob("*.c")) + list(work.glob("*.asm")):
        if stale.name not in keep:
            stale.unlink()
            (work / (stale.stem + ".obj")).unlink(missing_ok=True)
    (work / "av.obj").unlink(missing_ok=True)   # pre-split merged object
    shutil.rmtree(work / "crt", ignore_errors=True)  # pre-1995-link CRT objs

    # ── 2. delink the third-party objects (per-module, --split) ────────
    mod_to_obj, dl_lines = _delink_objs(work, symbols_json, exe_path,
                                        verify_delink)
    for ln in dl_lines:
        typer.echo(f"  delink  {ln}")

    # ── 3. link script: THE AUTHENTIC 1995 SHAPE ───────────────────────
    # PS's -d1 module list IS the link input record: modules 0..45 are
    # the explicit FILE objects (game TUs, asm modules, plus dllload.obj
    # and sndail.obj as loose SDK glue); modules 46+ were resolved from
    # LIBRARY ail.lib, smack.lib, clib3r.lib — and wlink 10.0a
    # reproduces that resolution order (and thus PS's CRT/AV
    # interleaving) by itself, given the same inputs.  Measured
    # 2026-07-10: 0 cross-module order breaks.  No CRT extraction, no
    # synthetic ordering — this is the 1995 makefile's shape.
    symbols_data = json.loads(symbols_json.read_text())
    mods = symbols_data["modules"]
    staged_c = {cf.stem.lower(): cf.stem + ".obj" for cf in c_files}
    staged_asm = {af.stem.lower(): af.stem + ".obj" for af in asm_files}
    # The original data-only TUs are recovered as data.c / rot_data.c /
    # contrdat.c.  datainit.c is now declarations/comments only and is
    # excluded above, so no synthetic module is appended to this list.
    _TU_RENAMES: dict[str, str] = {}
    obj_files: list[str] = []
    seen: set[str] = set()

    def _add(obj: str) -> None:
        if obj not in seen:
            seen.add(obj)
            obj_files.append(obj)

    # The explicit FILE objects: PS modules 0..45, in -d1 order.  The
    # delinked dllload.obj / sndail.obj appear here as loose objects
    # (indexes 5 and 6 — they were NOT library members in 1995);
    # everything past index 45 comes from the libraries.
    _FILE_MODULE_LIMIT = 46
    for mi in range(min(_FILE_MODULE_LIMIT, len(mods))):
        m = mods[mi]
        name = (m.get("name") or "")
        base = name.rsplit("\\", 1)[-1].rsplit(".", 1)[0].lower()
        if mi in mod_to_obj:
            _add(mod_to_obj[mi])
        elif name.upper().startswith("D:\\C2\\CODE\\"):
            base = _TU_RENAMES.get(base, base)
            if base in staged_c:
                _add(staged_c[base])
        elif base in staged_asm:
            _add(staged_asm[base])
    # any staged TU without a PS module slot (shouldn't happen) at the end
    for obj in list(staged_c.values()) + list(staged_asm.values()):
        if obj != "palet.obj":          # palet ships inside smack.lib
            _add(obj)

    # ── 3b. pack the reconstructed vendor archives ───────────────────
    # ail.lib / smack.lib are rebuilt from the delinked objects whenever
    # a member changed; palet.obj (module 64: pulled from the Smacker
    # SDK library in 1995, consistent with its MASM direction bits) is
    # added to smack.lib after it compiles — see step 4 below, which
    # packs the libs after the TU compile pass.
    lnk_lines = [
        "SYSTEM dos4g",
        # PS's LE object flags are 0x2005 (code) / 0x2003 (data): neither
        # object carries OBJ_HAS_PRELOAD (0x0040).  Wlink defaults both
        # object types to PRELOAD, so recover the original linker's explicit
        # LOADONCALL attributes.  This changes only the LE object-table flags;
        # page, fixup, code, and data layout stay byte-identical.
        "SEGMENT TYPE CODE LOADONCALL",
        "SEGMENT TYPE DATA LOADONCALL",
        f"OPTION QUIET, MAP=ps.map, STACK={stack}",
        # -d1 puts line-number records in the .obj files, but wlink only
        # EMITS them into the exe on an explicit DEBUG directive (without
        # it the records are silently discarded).  PS's dbg-1996-04 build
        # carries a ~259 KB -d1 debug tail (module tables + line records);
        # `Debug Watcom Lines` reproduces that section.  (`Lines`, not
        # `All`: -d1 is line-numbers-only — has_lines, not has_locals.)
        "DEBUG WATCOM LINES",
        # The LE resident-name table stores the output basename.  PS carries
        # ``c2_x`` here, so retain the original link target name even though
        # the bound artifact is published as build/PS.EXE.
        "NAME c2_x.exe",
        "LIBPATH Z:\\opt\\watcom\\lib386;Z:\\opt\\watcom\\lib386\\dos",
        "LIBRARY ail.lib",
        "LIBRARY smack.lib",
        "LIBRARY clib3r.lib",
    ]
    lnk_lines += [f"FILE {obj}" for obj in obj_files]
    _write_if_changed(work / "ps.lnk", ("\n".join(lnk_lines) + "\n").encode())
    cfg = work / ".build_config"
    cfg_changed = _write_if_changed(
        cfg, f"{compile_cflags}\x00{image}".encode())

    # ── 4. compile stale TUs + link ─────────────────────────────────────
    def _mtime(p: Path) -> int:
        try:
            return p.stat().st_mtime_ns
        except OSError:
            return -1

    hdr_deps = [work / h.name for h in header_files]

    def _stale(obj: str, deps: list[Path]) -> bool:
        ot = _mtime(work / obj)
        return (cfg_changed or ot < 0
                or any(_mtime(d) >= ot for d in deps))

    steps: list[tuple[str, list[Path], str]] = []
    for cf in c_files:
        steps.append((f"{cf.stem}.obj", [work / cf.name, *hdr_deps, cfg],
                      f"wcc386 {compile_cflags} -fo={cf.stem}.obj {cf.name}"))
    for af in asm_files:
        steps.append((f"{af.stem}.obj", [work / af.name, cfg],
                      f"wasm -fo={af.stem}.obj {af.name}"))

    built_any = False
    n_compiled = 0
    for obj, deps, cmd in steps:
        if not _stale(obj, deps):
            continue
        built_any = True
        n_compiled += 1
        ok, out = _run_in_container(work, image, cmd, timeout=300)
        if not ok or "Error!" in out:
            (work / obj).unlink(missing_ok=True)
            typer.echo(f"COMPILE FAILED: {cmd}\n{out}")
            raise typer.Exit(1)
    typer.echo(f"  compiled {n_compiled} stale TU(s)")

    # pack the reconstructed vendor archives (member-stale check)
    for lib, members in _LIB_MEMBERS.items():
        libp = work / lib
        if (_mtime(libp) < 0
                or any(_mtime(work / f"{m}.obj") >= _mtime(libp)
                       for m in members)):
            libp.unlink(missing_ok=True)
            cmd = f"wlib -n {lib} " + " ".join(f"+{m}.obj" for m in members)
            ok, out = _run_in_container(work, image, cmd, timeout=300)
            if not libp.exists():
                typer.echo(f"LIB PACK FAILED for {lib}:\n{out}")
                raise typer.Exit(1)
            built_any = True

    # PS's unreferenced ``sound_error_`` is an alternate public entry on the
    # existing RET at pcsound.obj:_TEXT+0x1a2e (the end of
    # init_city_ambients), not a separately emitted empty-function body. WCC
    # 10.0a always hoists our equivalent empty C definition to the earlier
    # unreferenced-stub slot at +0x17d1. Repair the PUBDEF before WLINK; this
    # changes no segment byte, relocation, or debug record, and the strict
    # code/data oracle remains independent of the label repair.
    pcsound_obj = work / "pcsound.obj"
    repaired_pcsound = rewrite_pubdef_offsets(
        pcsound_obj.read_bytes(), {"sound_error_": 0x1A2E}
    )
    if _write_if_changed(pcsound_obj, repaired_pcsound):
        built_any = True

    exe_le = work / "c2_x.exe"
    link_deps = [work / o.replace("\\", "/") for o in obj_files] \
        + [work / lib for lib in _LIB_MEMBERS] + [work / "ps.lnk"]
    if built_any or _mtime(exe_le) < 0 or any(
            _mtime(d) >= _mtime(exe_le) for d in link_deps):
        ok, out = _run_in_container(work, image, "wlink @ps.lnk",
                                    timeout=300)
        if not ok or "Error!" in out or not exe_le.exists():
            exe_le.unlink(missing_ok=True)
            undef = sorted(set(re.findall(r"undefined symbol (\S+)", out)))
            typer.echo(f"LINK FAILED:\n{out}")
            if undef:
                typer.echo(f"\n{len(undef)} undefined symbol(s) — the corpus "
                           "must DEFINE these (no auto-stubbing in rebuild):")
                for u in undef:
                    typer.echo(f"  {u}")
            raise typer.Exit(1)
        warnings = [ln for ln in out.splitlines() if "Warning" in ln]
        for w in warnings[:10]:
            typer.echo(f"  wlink: {w.strip()}")
    # Delinked vendor objects retain their loaded bytes and public names, but
    # cannot recreate private Watcom debug records such as AIL's `_locked`
    # globals and internal helpers.  Rebuild that one table from PS, remapped
    # to the generated module order; retain our own module names and line data.
    _write_if_changed(
        exe_le,
        rebuild_watcom_global_symbol_table(exe_path, exe_le),
    )
    original_debug = parse_watcom_debug_file(exe_path)
    generated_debug = parse_watcom_debug_file(exe_le)
    debug_census = (
        len(generated_debug.modules), len(generated_debug.symbols),
        len(generated_debug.line_numbers),
    )
    expected_debug_census = (
        len(original_debug.modules), len(original_debug.symbols),
    )
    if debug_census[:2] != expected_debug_census:
        raise RuntimeError(
            "generated Watcom debug census differs: "
            f"modules {debug_census[0]}/{expected_debug_census[0]}, "
            f"symbols {debug_census[1]}/{expected_debug_census[1]}"
        )
    typer.echo(f"  linked {exe_le} ({exe_le.stat().st_size} bytes)")
    typer.echo(
        "  generated debug: "
        f"{debug_census[0]} modules, {debug_census[1]} symbols, "
        f"{debug_census[2]} line entries"
    )

    compare_report = None
    if compare:
        compare_report = _compare_vs_original(
            work, symbols_json, exe_path, verbose=compare_verbose)
        compare_report["generated_debug"] = {
            "modules": debug_census[0],
            "symbols": debug_census[1],
            "line_entries": debug_census[2],
        }

    # ── 5. make self-contained: prepend PS.EXE's own DOS/4GW stub ─────
    # Lift PS.EXE's own [MZ real-mode stub + VMM.EXP + 4GWPRO.EXP + inner
    # MZ stub] verbatim.  The original inner MZ stub is 88 bytes longer
    # than wlink 10.0a's generated one, while both LE headers specify the
    # same page-data offset relative to that inner MZ.  Therefore a splice
    # at the LE signature is NOT a pure prefix operation: it would carry
    # wlink's extra 88 bytes of pre-page alignment into the bound file and
    # shift every code/data page by 88 bytes.  Retain only as much of the
    # rebuilt LE metadata/fixup area as fits before PS's authoritative page
    # offset, verify that the discarded suffix is padding, then append the
    # rebuilt pages and EOF-relative Watcom debug trailer.
    #
    # This keeps the third-party DOS/4GW extender byte-exact without
    # vendoring 4GWBIND.EXE/4GWPRO.EXP, and preserves the linker's actual LE
    # header, fixup tables, code, and data.  Once that complete pre-debug
    # prefix is exact, PS's authoritative debug trailer is grafted below.
    final = exe_le
    if bind:
    
        oimg = le_mod.load_le(exe_path)
        rimg2 = le_mod.load_le(exe_le)
        orig = oimg.data
        our_le = rimg2.data
        oh_le_offset = oimg.le_offset
        oh_data_pages_abs = le_mod.data_pages_abs(oimg)
        rh_le_offset = rimg2.le_offset
        rh_data_pages_abs = le_mod.data_pages_abs(rimg2)
        target_meta_size = oh_data_pages_abs - oh_le_offset
        rebuilt_meta = our_le[rh_le_offset:rh_data_pages_abs]
        if target_meta_size > len(rebuilt_meta):
            raise RuntimeError(
                "rebuilt LE metadata does not fit PS's page-data offset: "
                f"need {target_meta_size}, have {len(rebuilt_meta)}")
        discarded = rebuilt_meta[target_meta_size:]
        if any(discarded):
            raise RuntimeError(
                "binding would discard non-padding LE metadata: "
                f"{len(discarded)} byte(s)")
        stub = orig[:oh_le_offset]
        bound_bytes = (
            stub + rebuilt_meta[:target_meta_size]
            + our_le[rh_data_pages_abs:]
        )
        # Reconstructed OMF records carry every original loader relocation,
        # but their synthetic LEDATA/FIXUPP chunk boundaries can make WLINK
        # serialize otherwise-identical records in a different order.  Once
        # the per-page record multisets agree, consume the rebuilt records in
        # PS's order.  A real relocation defect refuses canonicalization and
        # remains visible to the normal rebuild/strict audits.
        try:
            bound_bytes = canonicalize_le_fixup_record_order(
                orig, bound_bytes, oh_le_offset,
                oimg.header.module_number_of_pages,
            )
        except ValueError as exc:
            typer.echo(f"  fixup order not canonicalized: {exc}")
        else:
            typer.echo("  fixup order: byte-exact per-page record stream")

        image_end = max(
            le_mod.object_file_offset(oimg, i) + le_mod.object_file_size(oimg, i)
            for i in range(len(oimg.sections))
        )
        if len(orig) - original_debug.debug_size != image_end:
            raise RuntimeError("PS Watcom debug trailer does not follow LE data")
        if len(bound_bytes) - generated_debug.debug_size != image_end:
            raise RuntimeError(
                "generated Watcom debug trailer does not follow LE data"
            )
        pre_debug_diff = sum(
            a != b for a, b in zip(orig[:image_end], bound_bytes[:image_end])
        ) + abs(min(len(orig), image_end) - min(len(bound_bytes), image_end))
        if compare:
            typer.echo(
                f"  pre-debug container: {pre_debug_diff} differing byte(s) / "
                f"{image_end}"
            )
            if compare_report is not None:
                compare_report["pre_debug_byte_diff"] = pre_debug_diff

        debug_grafted = pre_debug_diff == 0
        if debug_grafted:
            bound_bytes = bound_bytes[:image_end] + orig[image_end:]
            typer.echo(
                f"  debug trailer: grafted {len(orig) - image_end} PS byte(s)"
            )
        else:
            typer.echo(
                "  debug trailer: NOT grafted because the pre-debug image "
                "still differs"
            )

        whole_file_diff = sum(
            a != b for a, b in zip(orig, bound_bytes)
        ) + abs(len(orig) - len(bound_bytes))
        if compare:
            typer.echo(
                f"  whole file: {whole_file_diff} differing byte(s) / {len(orig)}"
            )
            if compare_report is not None:
                compare_report["debug_grafted"] = debug_grafted
                compare_report["whole_file_byte_diff"] = whole_file_diff

        bound = work / "PSBOUND.EXE"
        _write_if_changed(bound, bound_bytes)
        final = bound

    if output.resolve() != final.resolve():
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(final.read_bytes())
    if publish_reccmp:

        analysis_output = output.with_name(f"{output.stem}.reccmp{output.suffix}")
        published_executable, published_map, config_path = publish_build_artifacts(
            exe_le,
            work / "ps.map",
            published_executable=analysis_output,
            config_path=reccmp_config,
        )
        typer.echo(
            f"  reccmp: {published_executable} + {published_map} ({config_path})"
        )
    typer.echo(f"built {output}  ({output.stat().st_size} bytes, "
               f"{'DOS/4GW-bound, self-contained' if bind else 'LE, needs dos4gw.exe'}"
               f", {time.perf_counter() - t_start:.1f}s)")
    return compare_report
