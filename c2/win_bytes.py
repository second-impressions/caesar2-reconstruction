"""Byte-level engine for verifying the decomp source against CAESAR2.EXE.

The Windows analogue of the Watcom/PS.EXE path in
``c2.commands.decomp_verify``.  The DOS target is built by Watcom into one LE
executable and compared against ``PS.EXE``; the Windows target is built by
**MSVC 4.0 /Od** (the proven CAESAR2.EXE toolchain — see
``docs/windows-builds-fingerprint.md``) *per translation unit* into COFF
objects, and each function's bytes are compared against ``CAESAR2.EXE``.

Pipeline (mirrors the DOS one, fixup-masking and all)::

    compile TU (msvc-4.00-wibo)  ->  COFF .obj
      -> parse the symbol table for each function's .text range
      -> mask its DIR32/REL32 relocation slots (link-patched bytes)
      -> locate it in CAESAR2.EXE (func-map.json win_va, or a // WIN: annotation,
         or a map-independent masked search across .text)
      -> compare (raw byte diff = the oracle; structural diff = the workable
         figure, since /Od stack-slot shuffling makes raw byte-diff noisy).

A masked search HIT anywhere in ``.text`` *is* a byte-exact certificate
(map-independent), so a stale/approximate ``func-map`` entry never produces a
false "diff".

See ``docs/windows-dual-target-feasibility.md`` for the why.
"""
from __future__ import annotations

from collections import Counter
import json
import struct
import subprocess
import tempfile
import uuid
from dataclasses import dataclass, field, replace
from functools import lru_cache
from pathlib import Path
from typing import Optional

import capstone

# ── Paths & toolchain ─────────────────────────────────────────────────────────
_REPO = Path(__file__).resolve().parent.parent
# The Windows witness (build A) is fetched beside PS.EXE by c2 fetch-original.
WIN_EXE = _REPO / "original/CAESAR2.EXE"
FUNC_MAP = _REPO / "data/windows/func-map.json"
FUNC_ABSENT = _REPO / "data/windows/func-absent.json"
IMPORT_THUNKS = _REPO / "data/windows/import-thunks.json"
MSVC_CRT_CENSUS = _REPO / "data/windows/msvc-crt-census.json"
RELOCATION_SYMBOLS = _REPO / "data/windows/relocation-symbols.json"
DECOMP = _REPO
SRC_DIR = DECOMP / "src"
OBJ_DIR = DECOMP / "_objs"          # gitignored scratch for transient .obj

# The proven CAESAR2.EXE build config (data/windows-builds/ghidra-recreate.md).
MSVC_IMAGE = "ghcr.io/second-impressions/msvc-4.00-wibo:latest"
MSVC_FLAGS = ["/nologo", "/c", "/Od", "/Zp1", "/I", "include",
              "/DPLATFORM_WINDOWS=1", "/D__pascal=", "/D__far="]

_CS = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_CS.detail = True   # disp_offset/disp_size for reloc-in-displacement detection

# COFF relocation types that patch 4 bytes at link time.
_IMAGE_REL_I386_DIR32 = 6
_IMAGE_REL_I386_REL32 = 20


# ── CAESAR2.EXE (.text) ───────────────────────────────────────────────────────
@dataclass(frozen=True)
class WinImage:
    data: bytes
    text: bytes
    text_va0: int          # absolute VA of .text[0]
    image_base: int
    sections: tuple[tuple[bytes, int], ...] = ()

    def func_bytes(self, win_va: int, n: int) -> bytes:
        """``n`` bytes of CAESAR2.EXE starting at absolute VA ``win_va``."""
        off = win_va - self.text_va0
        if off < 0 or off + n > len(self.text):
            return b""
        return self.text[off : off + n]

    def find_bytes(self, needle: bytes) -> list[int]:
        """Return every loaded-section VA containing ``needle``."""
        if not needle:
            return []
        hits: list[int] = []
        for contents, va0 in self.sections:
            offset = contents.find(needle)
            while offset != -1:
                hits.append(va0 + offset)
                offset = contents.find(needle, offset + 1)
        return hits


@lru_cache(maxsize=1)
def load_win_image(path: Path = WIN_EXE) -> WinImage:
    d = path.read_bytes()
    e = struct.unpack_from("<I", d, 0x3C)[0]
    coff = e + 4
    nsec = struct.unpack_from("<H", d, coff + 2)[0]
    optsz = struct.unpack_from("<H", d, coff + 16)[0]
    image_base = struct.unpack_from("<I", d, coff + 20 + 28)[0]
    secoff = coff + 20 + optsz
    sections: list[tuple[bytes, int]] = []
    text = b""
    text_va0 = 0
    for i in range(nsec):
        b = secoff + i * 40
        nm = d[b : b + 8].rstrip(b"\0")
        vsize, vaddr, rawsz, rawptr = struct.unpack_from("<IIII", d, b + 8)
        contents = d[rawptr : rawptr + rawsz]
        sections.append((contents, image_base + vaddr))
        if nm == b".text":
            text = contents
            text_va0 = image_base + vaddr
    if not text:
        raise RuntimeError(f"no .text section in {path}")
    return WinImage(d, text, text_va0, image_base, tuple(sections))


# ── func-map.json (ps_name -> win_va) ─────────────────────────────────────────
@dataclass(frozen=True)
class WinMapEntry:
    win_va: int
    confidence: str
    src: str


@lru_cache(maxsize=1)
def load_func_map(path: Path = FUNC_MAP) -> dict[str, WinMapEntry]:
    rows = json.loads(path.read_text())
    out: dict[str, WinMapEntry] = {}
    for r in rows:
        out[r["ps_name"]] = WinMapEntry(
            int(r["win_va"], 16), r.get("confidence", "?"), r.get("src", ""))
    return out


@lru_cache(maxsize=1)
def load_absent_map(path: Path = FUNC_ABSENT) -> dict[str, dict]:
    """Functions known not to have a counterpart in the Windows build."""
    if not path.exists():
        return {}
    return {row["ps_name"]: row for row in json.loads(path.read_text())}


def absent_from_win(name: str, tu: Optional[str] = None) -> Optional[dict]:
    row = load_absent_map().get(name)
    if row is None:
        return None
    if tu is not None and row.get("src") != f"{tu}.c":
        return None
    return row


# ── // WIN: 0xADDR source annotations (override the map) ───────────────────────
@lru_cache(maxsize=256)
def win_annotations(tu: str) -> dict[str, int]:
    """``{function_name: win_va}`` for any ``// WIN: 0xADDR`` annotation that
    immediately precedes a definition in ``decomp/src/<tu>.c``.

    Mirrors the ``// FUNCTION: C2 0xADDR`` convention on the DOS side; lets the
    Windows address live in-tree and override / extend ``func-map.json``.
    """
    import re

    p = SRC_DIR / f"{tu}.c"
    if not p.exists():
        return {}
    lines = p.read_text(errors="replace").splitlines()
    win_re = re.compile(r"^\s*//\s*(?:WIN:|FUNCTION:\s*C2WIN)\s*(0x[0-9a-fA-F]+)")
    def_re = re.compile(r"^[A-Za-z_].*\b([A-Za-z_]\w*)\s*\(")
    out: dict[str, int] = {}
    pending: Optional[int] = None
    candidate: Optional[str] = None
    for ln in lines:
        m = win_re.match(ln)
        if m:
            pending = int(m.group(1), 16)
            candidate = None
            continue
        if pending is not None:
            stripped = ln.strip()
            if candidate is not None:
                if "{" in stripped:
                    out[candidate] = pending
                    pending = None
                    candidate = None
                elif ";" in stripped:
                    candidate = None
                continue
            d = def_re.match(ln)
            if d:
                candidate = d.group(1)
                if "{" in stripped:
                    out[candidate] = pending
                    pending = None
                    candidate = None
                elif ";" in stripped:
                    candidate = None
                continue
            if (stripped and not ln.lstrip().startswith("//")
                    and not stripped.startswith("#")):
                pending = None
    return out


@lru_cache(maxsize=1)
def all_win_annotations() -> dict[str, int]:
    """Collect source-owned Windows mappings across every recovered TU.

    Relocations only carry a symbol name, not the callee's source file.  The
    per-TU annotation lookup therefore cannot resolve calls into recovered
    Windows-only modules unless the annotations are also indexed globally.
    """
    candidates: dict[str, set[int]] = {}
    for path in sorted(SRC_DIR.glob("*.c")):
        for name, address in win_annotations(path.stem).items():
            candidates.setdefault(name, set()).add(address)
    return {
        name: next(iter(addresses))
        for name, addresses in candidates.items()
        if len(addresses) == 1
    }


def win_va_for(name: str, tu: Optional[str] = None) -> Optional[tuple[int, str]]:
    """Resolve a function's CAESAR2.EXE VA: ``// WIN:`` annotation first, then
    ``func-map.json``.  Returns ``(win_va, confidence)`` or ``None``."""
    if tu:
        ann = win_annotations(tu).get(name)
        if ann is not None:
            return ann, "annotation"
    else:
        ann = all_win_annotations().get(name)
        if ann is not None:
            return ann, "annotation"
    ent = load_func_map().get(name)
    if ent is not None:
        return ent.win_va, ent.confidence
    return None


# Confidence tiers that are placeholders / not a real CAESAR2.EXE location.
_WIN_SENTINELS = {0x401384}


def win_hint(name: str, tu: Optional[str] = None) -> dict:
    """Cheap (no-compile) CAESAR2.EXE mapping hint for a function -- a pure
    func-map / ``// WIN:`` lookup, for surfacing in worklist / dossier /
    decomp-verify.  Returns ``{available, win_va, confidence}``; ``available``
    is False when there is no mapping (or only the 0x401384 placeholder).
    A True hint means ``c2 win-verify <fn>`` / ``c2 win-decompile <fn>`` (the
    second byte oracle + MSVC /Od source view) are usable for this function.
    """
    unlinked = absent_from_win(name, tu)
    if unlinked is not None:
        status = unlinked.get("status", "absent")
        return {"available": False, "win_va": None,
                "confidence": f"known-{status}"}
    res = win_va_for(name, tu)
    if res is None or res[0] in _WIN_SENTINELS:
        return {"available": False, "win_va": None, "confidence": None}
    return {"available": True, "win_va": f"0x{res[0]:08x}", "confidence": res[1]}


# ── COFF object parsing ───────────────────────────────────────────────────────
@dataclass
class CompiledTU:
    tu: str
    text: bytes                                  # .text section bytes
    funcs: list[tuple[str, int, int]]            # (name, start, end) sorted
    reloc: set[int]                              # patched .text byte offsets
    errors: list[str] = field(default_factory=list)
    relocations: list["CoffRelocation"] = field(default_factory=list)
    data_symbols: dict[str, bytes] = field(default_factory=dict)
    local_symbols: dict[str, int] = field(default_factory=dict)
    symbols: tuple["CoffSymbol", ...] = ()

    def func_code(self, name: str) -> Optional[tuple[bytes, set[int]]]:
        for n, s, e in self.funcs:
            if n == name:
                mask = {i - s for i in self.reloc if s <= i < e}
                return self.text[s:e], mask
        return None

    def func_relocations(self, name: str) -> list["CoffRelocation"]:
        for n, s, e in self.funcs:
            if n == name:
                return [
                    CoffRelocation(r.offset - s, r.symbol, r.type, r.addend)
                    for r in self.relocations if s <= r.offset < e
                ]
        return []

    def func_start(self, name: str) -> Optional[int]:
        for function_name, start, _end in self.funcs:
            if function_name == name:
                return start
        return None


@dataclass(frozen=True)
class CoffRelocation:
    offset: int
    symbol: str
    type: int
    addend: int


@dataclass(frozen=True)
class CoffSymbol:
    """One primary COFF symbol emitted by MSVC 4.0.

    In particular, a C tentative definition (``int value;``) is represented
    as an external symbol with section zero and a non-zero ``value`` holding
    its allocation size.  An unresolved extern has the same section and
    storage class but a zero value.  Keeping this distinction is essential
    when auditing reconstructed declarations: both look undefined to a
    conventional section-only symbol reader, but only the former asks the
    linker to allocate storage.
    """

    index: int
    name: str
    value: int
    section: int
    type: int
    storage_class: int
    auxiliary_records: int

    @property
    def is_external(self) -> bool:
        return self.storage_class == 2

    @property
    def is_common(self) -> bool:
        return self.is_external and self.section == 0 and self.value != 0

    @property
    def is_undefined(self) -> bool:
        return self.is_external and self.section == 0 and self.value == 0


def _compiler_data_symbols(
        section_data: dict[int, bytes],
        defined_symbols: dict[int, list[tuple[int, str]]]
        ) -> dict[str, bytes]:
    """Extract bounded MSVC string and numeric compiler constants."""
    out: dict[str, bytes] = {}
    for secnum, symbols in defined_symbols.items():
        contents = section_data[secnum]
        offsets = sorted({value for value, _name in symbols})
        for value, name in symbols:
            if name.startswith("$SG"):
                end = contents.find(b"\0", value)
                if end >= value:
                    out[name] = contents[value : end + 1]
            elif name.startswith("$T"):
                end = next((offset for offset in offsets if offset > value),
                           len(contents))
                if end > value:
                    out[name] = contents[value:end]
    return out


def _parse_coff(
        obj: bytes,
) -> tuple[bytes, list[tuple[str, int, int]], set[int],
           list[CoffRelocation], dict[str, bytes], dict[str, int],
           tuple[CoffSymbol, ...]]:
    nsec = struct.unpack_from("<H", obj, 2)[0]
    symptr, nsym = struct.unpack_from("<II", obj, 8)
    optsz = struct.unpack_from("<H", obj, 16)[0]
    so = 20 + optsz
    text = b""
    text_idx = None
    relptr = nrel = 0
    section_data: dict[int, bytes] = {}
    for i in range(nsec):
        b = so + i * 40
        nm = obj[b : b + 8].rstrip(b"\0")
        _vs, _va, rawsz, rawptr, rp = struct.unpack_from("<IIIII", obj, b + 8)
        nr = struct.unpack_from("<H", obj, b + 32)[0]
        section_data[i + 1] = obj[rawptr : rawptr + rawsz]
        if nm == b".text":
            text = section_data[i + 1]
            text_idx, relptr, nrel = i + 1, rp, nr
    if text_idx is None:
        return b"", [], set(), [], {}, {}, ()

    strtab = symptr + nsym * 18

    def symname(raw: bytes) -> str:
        if raw[:4] == b"\0\0\0\0":
            off = struct.unpack_from("<I", raw, 4)[0]
            end = obj.find(b"\0", strtab + off)
            return obj[strtab + off : end].decode("latin1")
        return raw.rstrip(b"\0").decode("latin1")

    fsyms: list[tuple[int, str]] = []
    symbol_names: dict[int, str] = {}
    local_symbols: dict[str, int] = {}
    defined_symbols: dict[int, list[tuple[int, str]]] = {}
    symbols: list[CoffSymbol] = []
    i = 0
    while i < nsym:
        b = symptr + i * 18
        value, secnum, typ, sclass, naux = struct.unpack_from("<IhHBB", obj, b + 8)
        name = symname(obj[b : b + 8])
        symbol_names[i] = name
        symbols.append(CoffSymbol(
            i, _coff_source_name(name), value, secnum, typ, sclass, naux))
        if secnum in section_data and 0 <= value < len(section_data[secnum]):
            defined_symbols.setdefault(secnum, []).append((value, name))
        if secnum == text_idx and name.startswith("$L"):
            local_symbols[name] = value
        # Function symbols: derived-type DT_FCN (high nibble 2), defined in .text.
        if secnum == text_idx and sclass == 2 and (typ & 0x20):
            fsyms.append((value, _coff_source_name(name)))
        i += 1 + naux

    data_symbols = _compiler_data_symbols(section_data, defined_symbols)

    reloc: set[int] = set()
    relocations: list[CoffRelocation] = []
    for r in range(nrel):
        va, si, ty = struct.unpack_from("<IIH", obj, relptr + r * 10)
        if ty in (_IMAGE_REL_I386_DIR32, _IMAGE_REL_I386_REL32):
            reloc.update(range(va, va + 4))
            addend = struct.unpack_from("<i", text, va)[0]
            relocations.append(CoffRelocation(
                va, symbol_names.get(si, f"<symbol {si}>"), ty, addend))

    fsyms.sort()
    funcs: list[tuple[str, int, int]] = []
    for k, (val, name) in enumerate(fsyms):
        end = fsyms[k + 1][0] if k + 1 < len(fsyms) else len(text)
        funcs.append((name, val, end))
    return (text, funcs, reloc, relocations, data_symbols, local_symbols,
            tuple(symbols))


def _run_msvc(tu: str, *, image: str = MSVC_IMAGE,
              source_path: Optional[Path] = None,
              include_paths: Optional[dict[str, Path]] = None
              ) -> tuple[Optional[bytes], list[str]]:
    """Compile ``decomp/src/<tu>.c`` with MSVC 4.0 /Od -> COFF .obj bytes.

    ``source_path`` and ``include_paths`` overlay files inside the container.
    They are used by real-position compiler probes without changing the
    working tree.
    """
    OBJ_DIR.mkdir(exist_ok=True)
    objname = f"_wv_{tu}_{uuid.uuid4().hex[:8]}.obj"
    objpath = OBJ_DIR / objname
    cmd = [
        "podman", "run", "--rm", "-v", f"{DECOMP}:/src",
    ]
    if source_path is not None:
        cmd.extend(["-v", f"{source_path.resolve()}:/src/src/{tu}.c:ro"])
    effective_includes = dict(include_paths or {})
    owned_data = DECOMP / "include" / f"c2_data_{tu}.h"
    if "c2_data.h" not in effective_includes and owned_data.exists():
        effective_includes["c2_data.h"] = owned_data
    target_name = f"c2_tu_prefix_{tu}.h"
    target_header = DECOMP / "include" / "windows" / target_name
    if target_header.exists() and target_name not in effective_includes:
        effective_includes[target_name] = target_header
    missing_mount_targets = []
    for name, path in effective_includes.items():
        if Path(name).name != name:
            raise ValueError(f"include overlay must be a basename: {name!r}")
        mount_target = DECOMP / "include" / name
        if not mount_target.exists():
            missing_mount_targets.append(mount_target)
        cmd.extend(["-v", f"{path.resolve()}:/src/include/{name}:ro"])
    cmd.extend([
        image, "cl.exe",
        *MSVC_FLAGS, f"/Fo_objs/{objname}", f"src/{tu}.c",
    ])
    try:
        try:
            r = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        except (subprocess.TimeoutExpired, FileNotFoundError) as exc:
            return None, [str(exc)]
    finally:
        # Podman creates empty bind-mount targets inside the already mounted
        # include tree when an experiment introduces a new logical header.
        for mount_target in missing_mount_targets:
            if mount_target.is_file() and mount_target.stat().st_size == 0:
                mount_target.unlink()
    if not objpath.exists():
        errs = [l for l in (r.stdout + r.stderr).splitlines()
                if "error" in l.lower() or "fatal" in l.lower()]
        return None, errs or ["compile produced no object"]
    data = objpath.read_bytes()
    objpath.unlink(missing_ok=True)
    return data, []


_TU_CACHE: dict[str, CompiledTU] = {}


def compile_tu(tu: str, *, cache: bool = True) -> CompiledTU:
    """Compile a TU (memoised within a process) into a ``CompiledTU``."""
    if cache and tu in _TU_CACHE:
        return _TU_CACHE[tu]
    obj, errs = _run_msvc(tu)
    if obj is None:
        ctu = CompiledTU(tu, b"", [], set(), errs)
    else:
        text, funcs, reloc, relocations, data_symbols, local_symbols, symbols = \
            _parse_coff(obj)
        ctu = CompiledTU(
            tu, text, funcs, reloc, relocations=relocations,
            data_symbols=data_symbols, local_symbols=local_symbols,
            symbols=symbols)
    if cache:
        _TU_CACHE[tu] = ctu
    return ctu


def compile_tu_source(
        tu: str, source: str, *,
        include_sources: Optional[dict[str, str]] = None) -> CompiledTU:
    """Compile an ephemeral replacement for one TU at its real tree position.

    Unlike ``compile_tu``, this is deliberately never cached.  The source is
    mounted read-only over ``/src/src/<tu>.c`` and removed after the compiler
    exits. Optional include replacements are mounted over ``/src/include``.
    Compiler-state experiments therefore cannot dirty reconstructed source.
    """
    path: Optional[Path] = None
    include_paths: dict[str, Path] = {}
    try:
        with tempfile.NamedTemporaryFile(
                mode="w", suffix=f"-{tu}.c", encoding="utf-8",
                delete=False) as f:
            f.write(source)
            path = Path(f.name)
        for name, contents in (include_sources or {}).items():
            with tempfile.NamedTemporaryFile(
                    mode="w", suffix=f"-{name}", encoding="utf-8",
                    delete=False) as f:
                f.write(contents)
                include_paths[name] = Path(f.name)
        obj, errs = _run_msvc(
            tu, source_path=path, include_paths=include_paths)
    finally:
        if path is not None:
            path.unlink(missing_ok=True)
        for include_path in include_paths.values():
            include_path.unlink(missing_ok=True)
    if obj is None:
        return CompiledTU(tu, b"", [], set(), errs)
    text, funcs, reloc, relocations, data_symbols, local_symbols, symbols = \
        _parse_coff(obj)
    return CompiledTU(
        tu, text, funcs, reloc, relocations=relocations,
        data_symbols=data_symbols, local_symbols=local_symbols,
        symbols=symbols)


# ── Masked search / comparison ────────────────────────────────────────────────
def masked_find(haystack: bytes, needle: bytes, mask: set[int]) -> list[int]:
    """Offsets in ``haystack`` where ``needle`` matches with ``mask`` bytes
    treated as wildcards.  Anchored on the longest unmasked run."""
    n = len(needle)
    best_start = best_len = run = run_start = 0
    for i in range(n):
        if i not in mask:
            if run == 0:
                run_start = i
            run += 1
            if run > best_len:
                best_start, best_len = run_start, run
        else:
            run = 0
    if best_len == 0:
        return []
    anchor = needle[best_start : best_start + best_len]
    hits: list[int] = []
    pos = haystack.find(anchor)
    while pos != -1:
        s = pos - best_start
        if 0 <= s and s + n <= len(haystack) and all(
            (i in mask) or haystack[s + i] == needle[i] for i in range(n)
        ):
            hits.append(s)
        pos = haystack.find(anchor, pos + 1)
    return hits


def _norm_op(op: str) -> str:
    """Operand string with absolute addresses + immediates wildcarded.

    Keeps mnemonic/register/memory *shape* (incl. ebp/esp displacement
    presence) so the structural compare is insensitive to which global a
    DIR32 points at or which exact constant an immediate holds -- the
    link-/layout-variant bytes -- while still distinguishing real opcode and
    addressing-mode divergence."""
    import re
    op = re.sub(r"0x[0-9a-f]+", "K", op)
    op = re.sub(r"\b\d+\b", "K", op)
    return op


def disasm_norm(code: bytes, mask: Optional[set[int]] = None):
    """``[(hexbytes, asm_text, norm_key)]`` for a code blob.

    Reloc-in-displacement canonicalisation: the unlinked .obj displacement
    holds the relocation addend, so capstone can print ``[eax]`` or
    ``[eax - 0x12c0]`` where the linked exe shows an absolute ``+ K``.
    When the mask overlaps the displacement field, normalise either form to
    the linked ``[… + K]`` shape."""
    import re
    out = []
    for ins in _CS.disasm(code, 0):
        off = ins.address
        raw = bytes(ins.bytes)
        hx = "".join(".." if (mask and (off + k) in mask) else f"{b:02x}"
                     for k, b in enumerate(raw))
        asm = f"{ins.mnemonic} {ins.op_str}".rstrip()
        key = f"{ins.mnemonic} {_norm_op(ins.op_str)}"
        if mask:
            try:
                do, ds = ins.disp_offset, ins.disp_size
            except Exception:  # noqa: BLE001
                do, ds = 0, 0
            if do and ds and any((off + do + k) in mask for k in range(ds)):
                if ins.disp == 0:
                    # A zero addend omits the displacement entirely.
                    key = re.sub(
                        r"\[([A-Za-z0-9* +\-]+)\]", r"[\1 + K]", key)
                else:
                    # A negative addend still resolves to an absolute symbol
                    # address in the linked executable.
                    key = re.sub(r" - K\]", " + K]", key)
        out.append((hx, asm, key))
    return out


def _struct_distance(a_keys: list[str], b_keys: list[str]) -> int:
    """Instruction-level edit distance under optimal alignment (difflib).

    Counts how many instructions truly diverge, NOT positional shifts -- so
    an inserted/removed instruction (e.g. an extra local's store) costs 1,
    not 'everything after it'."""
    import difflib
    sm = difflib.SequenceMatcher(a=a_keys, b=b_keys, autojunk=False)
    matched = sum(blk.size for blk in sm.get_matching_blocks())
    return max(len(a_keys), len(b_keys)) - matched


@dataclass
class FuncVerdict:
    name: str
    tu: str
    status: str  # "exact" | "diff" | "nomap" | "absent" | "discarded"
    size: int = 0
    byte_diff: int = 0                # raw masked byte diff at located va
    struct_diff: int = 0             # mnemonic+normed-operand mismatches
    insn_total: int = 0
    win_va: Optional[int] = None
    confidence: str = ""
    located_va: Optional[int] = None  # where the bytes actually matched/aligned


def verify_func(name: str, tu: str, *, win: Optional[WinImage] = None,
                ctu: Optional[CompiledTU] = None) -> FuncVerdict:
    """Verdict for one decompiled function vs CAESAR2.EXE."""
    win = win or load_win_image()
    ctu = ctu or compile_tu(tu)
    fc = ctu.func_code(name)
    if fc is None:
        return FuncVerdict(name, tu, "absent")
    code, mask = fc
    n = len(code)
    unlinked = absent_from_win(name, tu)
    if unlinked is not None:
        status = unlinked.get("status", "absent")
        return FuncVerdict(name, tu, status, n,
                           confidence=f"known-{status}")
    resolved = win_va_for(name, tu)
    win_va = resolved[0] if resolved else None
    conf = resolved[1] if resolved else ""

    # Authoritative exactness: a masked hit ANYWHERE in .text (map-independent).
    # Structural twins can produce several hits.  Prefer the mapped address when
    # it is one of them so downstream map verification does not misclassify an
    # exact, callgraph-proven twin as belonging to the first copy in .text.
    hits = masked_find(win.text, code, mask)
    if hits:
        hit_vas = [win.text_va0 + hit for hit in hits]
        if win_va in hit_vas:
            return FuncVerdict(name, tu, "exact", n, 0, 0, 0,
                               win_va, conf, win_va)
        # An explicit source annotation identifies this function even when a
        # relocation-masked structural twin exists elsewhere in .text.  Treat
        # that twin as evidence for the shape, not as an exact identity match.
        if win_va is None or conf != "annotation":
            return FuncVerdict(name, tu, "exact", n, 0, 0, 0,
                               win_va, conf, hit_vas[0])

    if win_va is None:
        return FuncVerdict(name, tu, "nomap", n, win_va=None, confidence=conf)

    # Diff: align at the mapped va and count raw + structural divergence.
    wbytes = win.func_bytes(win_va, n)
    byte_diff = sum(1 for i in range(min(n, len(wbytes)))
                    if i not in mask and code[i] != wbytes[i]) + abs(n - len(wbytes))
    ours = disasm_norm(code, mask)
    theirs = disasm_norm(wbytes)
    struct_diff = _struct_distance([r[2] for r in ours], [r[2] for r in theirs])
    return FuncVerdict(name, tu, "diff", n, byte_diff, struct_diff, len(ours),
                       win_va, conf, win_va)


def aligned_diff(v: "FuncVerdict",
                 *, ctu: Optional[CompiledTU] = None) -> list[dict]:
    """Aligned MSVC-of-our-source vs CAESAR2.EXE rows for a diff verdict.

    ``[{kind, ours, theirs}]`` where kind is ``equal`` | ``struct`` (a real
    shape divergence — inserted/removed/different-mnemonic) | ``slot`` (same
    instruction shape, only a displacement/immediate differs — the /Od
    stack-slot / global / immediate noise).  Shared by the CLI view and the
    pi tool.
    """
    import difflib

    ctu = ctu or compile_tu(v.tu)
    fc = ctu.func_code(v.name)
    if fc is None or v.win_va is None:
        return []
    code, mask = fc
    win = load_win_image()
    wbytes = win.func_bytes(v.win_va, len(code))
    ours = disasm_norm(code, mask)
    theirs = disasm_norm(wbytes)
    sm = difflib.SequenceMatcher(a=[r[2] for r in ours], b=[r[2] for r in theirs],
                                 autojunk=False)
    rows: list[dict] = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            for k in range(i2 - i1):
                o, t = ours[i1 + k], theirs[j1 + k]
                rows.append({"kind": "slot" if o[1] != t[1] else "equal",
                             "ours": o[1], "theirs": t[1]})
        else:
            for k in range(max(i2 - i1, j2 - j1)):
                o = ours[i1 + k] if i1 + k < i2 else None
                t = theirs[j1 + k] if j1 + k < j2 else None
                rows.append({"kind": "struct",
                             "ours": o[1] if o else "", "theirs": t[1] if t else ""})
    return rows


def tu_of(name: str) -> Optional[str]:
    """The source TU basename a function's DEFINITION lives in."""
    return _func_tu_index().get(name)


def _definition_names(source: str) -> list[str]:
    """Return column-zero function definitions, excluding multiline prototypes."""
    import re
    definition_re = re.compile(
        r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b([A-Za-z_]\w*)"
        r"\s*\([^;]*?\)(?=(?:\s*\#[^\n]*\n)*\s*\{)",
        re.MULTILINE,
    )
    return [match.group(1) for match in definition_re.finditer(source)]


@lru_cache(maxsize=1)
def _func_tu_index() -> dict[str, str]:
    """Map every function name to its TU by scanning real definitions."""
    idx: dict[str, str] = {}
    for p in sorted(SRC_DIR.glob("*.c")):
        for name in _definition_names(p.read_text(errors="replace")):
            idx.setdefault(name, p.stem)
    return idx


def decompiled_funcs(tu: str) -> list[str]:
    """Functions with a real body (compiled symbols) in a TU."""
    return [n for n, _s, _e in compile_tu(tu).funcs]


# ── Frame-slot census (the W2 witness; docs/root-cause-survey-2026-07-02.md) ──
#
# MSVC 4.0 /Od gives every named source local a distinct ``[ebp-N]`` frame
# slot.  Comparing CAESAR2.EXE's slot set against our own MSVC compile of the
# same function is therefore a census of the ORIGINAL's named-local set —
# the input that decides Watcom conflict membership / savings / spill sets.
# Reliability is gated by the mapping quality Q (the win func-map is fuzzy):
# on the PS-byte-exact corpus the slot-count census agrees 78.5 %; mismatches
# concentrate in low-Q mappings and genuine port drift.

_SLOT_RE = __import__("re").compile(r"\[ebp - (0x[0-9a-f]+|\d+)\]")
_SUB_RE = __import__("re").compile(r"^sub esp, (0x[0-9a-f]+|\d+)$")


def _slot_census(insns: list[str]) -> tuple[Optional[int], dict[int, dict]]:
    """``(frame_size, {disp: {widths, n_uses, first_use_asm}})`` for one side."""
    frame: Optional[int] = None
    slots: dict[int, dict] = {}
    for i, asm in enumerate(insns):
        if frame is None and i < 6:
            m = _SUB_RE.match(asm)
            if m:
                frame = int(m.group(1), 0)
        for m in _SLOT_RE.finditer(asm):
            disp = int(m.group(1), 0)
            width = "d"
            if "byte ptr" in asm:
                width = "b"
            elif "word ptr" in asm and "dword" not in asm:
                width = "w"
            rec = slots.setdefault(disp, {"widths": set(), "n_uses": 0, "first": asm})
            rec["widths"].add(width)
            rec["n_uses"] += 1
    return frame, slots


@dataclass
class CensusVerdict:
    name: str
    tu: str
    ok: bool                       # census computed at all
    quality: float = 0.0           # aligned-instruction match ratio (0..1)
    gate: str = ""                 # "usable" | "caution" | "mapping-suspect"
    frame_ours: Optional[int] = None
    frame_theirs: Optional[int] = None
    slots_ours: dict = field(default_factory=dict)
    slots_theirs: dict = field(default_factory=dict)
    delta: int = 0                 # len(theirs) - len(ours)
    note: str = ""


def census_func(name: str, tu: Optional[str] = None, *,
                win: Optional[WinImage] = None,
                ctu: Optional[CompiledTU] = None) -> CensusVerdict:
    """Frame-slot census of one function: our MSVC /Od build vs CAESAR2.EXE.

    ``delta > 0`` ⇒ the original has MORE named locals than our source
    (missing locals — the §13 named-local class); ``delta < 0`` ⇒ our source
    INVENTED locals the original lacks.  Only trust ``gate == "usable"``.
    """
    import difflib

    tu = tu or tu_of(name)
    if tu is None:
        return CensusVerdict(name, "?", False, note="unknown TU")
    win = win or load_win_image()
    ctu = ctu or compile_tu(tu)
    if ctu.errors:
        return CensusVerdict(name, tu, False,
                             note=f"TU fails MSVC compile: {ctu.errors[0]}")
    fc = ctu.func_code(name)
    if fc is None:
        return CensusVerdict(name, tu, False, note="no MSVC body")
    code, mask = fc
    resolved = win_va_for(name, tu)
    if not resolved:
        return CensusVerdict(name, tu, False, note="no win mapping")
    wbytes = win.func_bytes(resolved[0], len(code))
    ours = disasm_norm(code, mask)
    theirs = disasm_norm(wbytes)
    sm = difflib.SequenceMatcher(a=[r[2] for r in ours], b=[r[2] for r in theirs],
                                 autojunk=False)
    matched = sum(b.size for b in sm.get_matching_blocks())
    q = matched / max(len(ours), 1)
    gate = "usable" if q >= 0.85 else ("caution" if q >= 0.7 else "mapping-suspect")
    fo, so = _slot_census([r[1] for r in ours])
    ft, st = _slot_census([r[1] for r in theirs])
    return CensusVerdict(name, tu, True, q, gate, fo, ft, so, st,
                         len(st) - len(so))


# ── Rule 158: folded always-true-guard probe ─────────────────────────────────
# A source guard like ``kind >= 0 &&`` (kind unsigned) emits ZERO bytes under
# Watcom (constant-folded after flow-graph construction, where it still roots
# a CSE partition — docs/watcom-codegen-patterns.md Rule 158) but is LITERAL
# under MSVC /Od.  So the CAESAR2.EXE oracle shows it as a one-sided
# instruction run shaped like a zero-compare guard:
#
#     xor eax, eax              (uchar zext, optional)
#     mov al, byte ptr [ebp-X]
#     test eax, eax             (or cmp reg, 0)
#     jl <skip>                 (signed zero-relative jcc)
#
# ``theirs``-only run  ⇒ OUR source is MISSING the guard (add it).
# ``ours``-only run    ⇒ our source INVENTED a guard the original lacks.
#
# Ground truth: evolve_land_value (evolver.c) — a single ``kind >= 0 &&``
# token was the whole 247-byte PS diff; found ONLY via this witness.

_GUARD_JCC = ("jl", "jge", "jle", "jg", "js", "jns")


@dataclass
class GuardHit:
    """One suspected folded-guard site from the aligned win diff."""

    side: str          # "theirs" = CAESAR2-only (we're missing the guard)
    kind: str          # "zext0" (uchar zext + zero-test) | "cmp0"
    insns: list[str]   # the one-sided run
    after: list[str]   # next shared/CAESAR2 insns (the guarded condition)
    row: int           # index into aligned_diff rows (locality anchor)


def _guard_run_kind(insns: list[str]) -> Optional[str]:
    """Classify a one-sided run as a PURE zero-compare guard, else None.

    Shape: [optional zext/load insns] + zero-test + signed jcc, with the
    zero-test IMMEDIATELY feeding the final jcc and nothing but loads /
    register-zeroing in between (any arithmetic/store/call disqualifies —
    that's a genuinely missing/extra statement, not a folded guard).
    """
    import re as _re
    if not 2 <= len(insns) <= 6:
        return None
    last = insns[-1].split()[0] if insns[-1] else ""
    if last not in _GUARD_JCC:
        return None
    zt = insns[-2]
    m = _re.match(r"test (\w+), (\w+)$", zt)
    is_zero_test = bool(m and m.group(1) == m.group(2)) \
        or bool(_re.match(r"cmp \w+, 0$", zt)) \
        or bool(_re.match(r"cmp (dword|word|byte) ptr \[[^]]*\], 0$", zt))
    if not is_zero_test:
        return None
    # body (before the zero-test) may only be loads / register zeroing
    for t in insns[:-2]:
        if _re.match(r"xor (\w+), \1$", t):
            continue
        if _re.match(r"mov \w+, (byte |word |dword )?ptr \[", t) \
                or _re.match(r"mov \w+, \w+$", t) \
                or _re.match(r"movsx \w+, ", t) \
                or _re.match(r"movzx \w+, ", t):
            continue
        return None
    has_zext = any(_re.match(r"xor (\w+), \1$", t) for t in insns) and \
        any(_re.match(r"mov [a-d]l, byte ptr", t) for t in insns)
    return "zext0" if has_zext else "cmp0"


def guard_hits(v: "FuncVerdict") -> list[GuardHit]:
    """Scan the aligned win diff for folded-guard fingerprints (Rule 158)."""
    if v.status != "diff":
        return []
    rows = aligned_diff(v)
    hits: list[GuardHit] = []
    i = 0
    while i < len(rows):
        r = rows[i]
        side = None
        if r["kind"] == "struct" and r["theirs"] and not r["ours"]:
            side = "theirs"
        elif r["kind"] == "struct" and r["ours"] and not r["theirs"]:
            side = "ours"
        if side is None:
            i += 1
            continue
        j = i
        key = "theirs" if side == "theirs" else "ours"
        other = "ours" if side == "theirs" else "theirs"
        run: list[str] = []
        while j < len(rows) and rows[j]["kind"] == "struct" \
                and rows[j][key] and not rows[j][other]:
            run.append(rows[j][key])
            j += 1
        kind = _guard_run_kind(run)
        if kind:
            after = [rows[k]["theirs"] or rows[k]["ours"]
                     for k in range(j, min(j + 2, len(rows)))]
            hits.append(GuardHit(side, kind, run, after, i))
        i = j
    return hits


def guard_probe(name: str, tu: Optional[str] = None) -> dict:
    """Rule 158 probe for one function.  ``{available, hits:[…], note}``.

    ``hits`` entries: {side, kind, insns, after}.  ``side == "theirs"`` ⇒
    add the folded always-true guard (``x >= 0 &&`` on the matching
    condition — read ``after`` for the guarded compare); ``side == "ours"``
    ⇒ delete the guard our source invented.
    """
    tu = tu or tu_of(name)
    if tu is None:
        return {"available": False, "note": "unknown TU"}
    try:
        v = verify_func(name, tu)
    except Exception as exc:  # noqa: BLE001
        return {"available": False, "note": f"win verify failed: {exc}"}
    if v.status in ("absent", "discarded", "nomap"):
        return {"available": False, "note": f"win status: {v.status}"}
    if v.status == "exact":
        return {"available": True, "hits": [], "note": "win-exact"}
    hits = guard_hits(v)
    return {"available": True,
            "hits": [{"side": h.side, "kind": h.kind, "insns": h.insns,
                      "after": h.after} for h in hits],
            "note": f"{len(hits)} suspected folded-guard site(s)"}


# ── lever hints: actionable fingerprints in the aligned win diff ──────────────
#
# Each detector matches a divergence class with a KNOWN source-level lever
# (see docs/msvc-od-slot-allocation.md and docs/char-signedness-proof.md).
# They are deliberately conservative: a hint means "this lever is worth
# probing", never "this is certainly the fix".

@dataclass
class LeverHint:
    """One actionable lever suggestion from the aligned win diff."""

    kind: str          # signedness | jcc_mirror | slot_swap | frame_size
                       # | platform_call | imm_macro | stmt_order
    detail: str        # human-readable, actionable text
    row: int           # anchor index into aligned_diff rows


_JCC_MIRROR = {("jge", "jle"), ("jle", "jge"), ("jg", "jl"), ("jl", "jg"),
               ("ja", "jb"), ("jb", "ja"), ("jae", "jbe"), ("jbe", "jae")}

# Known per-platform immediate pairs (Watcom value, MSVC value) -> macro.
_IMM_MACROS = {
    (0x200, 0x8000): "O_BINARY (fcntl.h)",
    (0x100, 0x4000): "O_TEXT (fcntl.h)",
    (0x20, 0x100): "O_CREAT (fcntl.h)",
    (0x40, 0x200): "O_TRUNC (fcntl.h)",
    (0x10, 0x8): "O_APPEND (fcntl.h)",
}

_EBP_SLOT_RE = None  # compiled lazily


@lru_cache(maxsize=1)
def _globals_reverse() -> dict[int, str]:
    """win VA -> global name, from globals-map.json (best-effort)."""
    import json
    path = _REPO / "data" / "windows" / "globals-map.json"
    out: dict[int, str] = {}
    if path.exists():
        try:
            for e in json.loads(path.read_text()):
                out.setdefault(int(e["win_va"], 16), e["name"])
        except Exception:  # noqa: BLE001
            pass
    return out


@lru_cache(maxsize=1)
def _globals_forward() -> dict[str, tuple[int, str]]:
    """Global name -> (win VA, map tier), from globals-map.json."""
    path = _REPO / "data" / "windows" / "globals-map.json"
    out: dict[str, tuple[int, str]] = {}
    if path.exists():
        try:
            for e in json.loads(path.read_text()):
                out.setdefault(
                    e["name"], (int(e["win_va"], 16), e.get("tier", "?")))
        except Exception:  # noqa: BLE001
            pass
    return out


def _coff_source_name(symbol: str) -> str:
    """Undo the x86 COFF C decoration used by MSVC 4.0."""
    import re
    if symbol.startswith("__imp__"):
        symbol = symbol[7:]
    elif symbol.startswith("_imp__"):
        symbol = symbol[6:]
    name = symbol[1:] if symbol.startswith("_") else symbol
    return re.sub(r"@\d+$", "", name)


def _unique_symbol_addresses(
        candidates: list[tuple[str, int]], *, coff_names: bool = True
        ) -> dict[str, int]:
    """Keep symbol names with one independently evidenced Windows address."""
    grouped: dict[str, set[int]] = {}
    for symbol, address in candidates:
        name = _coff_source_name(symbol) if coff_names else symbol
        grouped.setdefault(name, set()).add(address)
    return {
        symbol: next(iter(addresses))
        for symbol, addresses in grouped.items()
        if len(addresses) == 1
    }


@lru_cache(maxsize=1)
def _import_symbol_maps(
        path: Path = IMPORT_THUNKS) -> tuple[dict[str, int], dict[str, int]]:
    """Return unambiguous ``(IAT, text thunk)`` maps by COFF source name."""
    if not path.exists():
        return {}, {}
    rows = json.loads(path.read_text())
    iat_candidates: list[tuple[str, int]] = []
    for caller in rows.get("text_import_callers", []):
        for imported in caller.get("imports", []):
            name = imported.get("name") or imported.get("resolved_name")
            if name:
                iat_candidates.append((name, int(imported["iat_va"], 16)))
    thunk_candidates: list[tuple[str, int]] = []
    for thunk in rows.get("text_import_thunks", []):
        name = thunk.get("name") or thunk.get("resolved_name")
        if name:
            thunk_candidates.append((name, int(thunk["win_va"], 16)))
    return (_unique_symbol_addresses(iat_candidates),
            _unique_symbol_addresses(thunk_candidates))


@lru_cache(maxsize=1)
def _crt_symbol_candidates(
        path: Path = MSVC_CRT_CENSUS
        ) -> tuple[dict[str, frozenset[int]], dict[str, frozenset[int]]]:
    """Static-CRT candidate addresses by raw linker name and C alias."""
    if not path.exists():
        return {}, {}
    rows = json.loads(path.read_text())
    raw_candidates: list[tuple[str, int]] = []
    alias_candidates: list[tuple[str, int]] = []
    for match in rows.get("function_matches", []):
        address = int(match["va"], 16)
        for identity in match.get("identities", []):
            raw_name = identity.get("name")
            if not raw_name:
                continue
            raw_candidates.append((raw_name, address))
            source_name = _coff_source_name(raw_name)
            alias_candidates.append((source_name, address))
            alias_candidates.append((source_name.lstrip("_"), address))
    def group(candidates: list[tuple[str, int]]) -> dict[str, frozenset[int]]:
        grouped: dict[str, set[int]] = {}
        for symbol, address in candidates:
            grouped.setdefault(symbol, set()).add(address)
        return {symbol: frozenset(addresses)
                for symbol, addresses in grouped.items()}

    return group(raw_candidates), group(alias_candidates)


@lru_cache(maxsize=1)
def _crt_symbol_maps(
        path: Path = MSVC_CRT_CENSUS) -> tuple[dict[str, int], dict[str, int]]:
    """Exact static-CRT maps by raw linker name and unambiguous C alias."""
    raw_candidates, alias_candidates = _crt_symbol_candidates(path)
    return (
        {name: next(iter(addresses))
         for name, addresses in raw_candidates.items() if len(addresses) == 1},
        {name: next(iter(addresses))
         for name, addresses in alias_candidates.items() if len(addresses) == 1},
    )


@lru_cache(maxsize=1)
def _link_symbol_corpus(path: Path = RELOCATION_SYMBOLS) -> dict[str, dict]:
    """Classified game/CRT link symbols recovered from exact native xrefs."""
    if not path.exists():
        return {}
    rows = json.loads(path.read_text())
    return {
        row["name"]: {
            **row,
            "addresses": frozenset(int(address, 16)
                                   for address in row["addresses"]),
        }
        for row in rows.get("symbols", [])
    }


def resolve_win_symbol(
        symbol: str, relocation_type: Optional[int] = None, *,
        include_link_corpus: bool = True
        ) -> Optional[tuple[int, str]]:
    """Resolve an external COFF relocation through the linker symbol maps.

    TU annotations locate recovered function bodies.  They do not necessarily
    name the externally linked implementation: Windows can retain an exact
    legacy/static body while callers bind a native replacement with the same
    source name.  Prefer the function map for that link identity, then fall
    back to annotations when no external mapping is available.
    """
    name = _coff_source_name(symbol)
    global_match = _globals_forward().get(name)
    if global_match is not None:
        return global_match[0], f"global:{global_match[1]}"
    external_match = load_func_map().get(name)
    if (external_match is not None
            and external_match.win_va not in _WIN_SENTINELS):
        return (external_match.win_va,
                f"function:{external_match.confidence}")
    iat, thunks = _import_symbol_maps()
    if (symbol.startswith(("__imp__", "_imp__"))
            or relocation_type == _IMAGE_REL_I386_DIR32):
        import_match = iat.get(name)
        if import_match is not None:
            return import_match, "import:iat-manifest"
    if relocation_type in (None, _IMAGE_REL_I386_REL32):
        thunk_match = thunks.get(name)
        if thunk_match is not None:
            return thunk_match, "import:thunk-manifest"
    crt_raw, crt_aliases = _crt_symbol_maps()
    crt_match = crt_raw.get(symbol)
    if crt_match is None:
        crt_match = crt_aliases.get(name)
    if crt_match is not None:
        return crt_match, "crt:archive-exact"
    if include_link_corpus:
        link_match = _link_symbol_corpus().get(name)
        if link_match is not None:
            if link_match["resolution"] == "unique":
                address = next(iter(link_match["addresses"]))
                return address, f"{link_match['ownership']}:mapped-xref"
            if link_match["resolution"] == "assigned":
                address = int(link_match["assigned_address"], 16)
                return address, f"{link_match['ownership']}:xref-assigned"
    function_match = win_va_for(name)
    if function_match is not None and function_match[0] not in _WIN_SENTINELS:
        return function_match[0], f"function:{function_match[1]}"
    return None


@dataclass(frozen=True)
class RelocationIdentity:
    function: str
    offset: int
    symbol: str
    type: str
    addend: int
    expected: Optional[int]
    actual: Optional[int]
    confidence: str
    equivalence: str = ""

    @property
    def status(self) -> str:
        if self.expected is None or self.actual is None:
            return "unresolved"
        if self.expected == self.actual:
            return "exact"
        return "equivalent" if self.equivalence else "mismatch"


def _commutative_relocation_equivalences(
        code: bytes, rows: list[RelocationIdentity]
        ) -> dict[int, str]:
    """Classify complete relocation permutations in simple arithmetic chains.

    MSVC C2 may reverse identifier operands after C1 has emitted relocation
    records.  Masked bodies then remain byte-exact while the relocation slots
    appear exchanged.  Accept only a complete same-register ``mov`` + ``add``
    chain, or a two-factor ``mov`` + ``imul`` chain, whose expected and linked
    target multisets are identical.
    """
    instructions = list(_CS.disasm(code, 0))
    row_by_instruction: dict[int, list[int]] = {}
    for row_index, row in enumerate(rows):
        for ins_index, instruction in enumerate(instructions):
            if instruction.address <= row.offset < (
                    instruction.address + instruction.size):
                row_by_instruction.setdefault(ins_index, []).append(row_index)
                break

    def relocated_memory_operand(ins_index: int, mnemonic: str,
                                 register: Optional[int] = None
                                 ) -> Optional[tuple[int, int]]:
        instruction = instructions[ins_index]
        indices = row_by_instruction.get(ins_index, [])
        if instruction.mnemonic != mnemonic or len(indices) != 1:
            return None
        if len(instruction.operands) != 2:
            return None
        destination, source = instruction.operands
        if destination.type != capstone.x86.X86_OP_REG:
            return None
        if register is not None and destination.reg != register:
            return None
        if source.type != capstone.x86.X86_OP_MEM:
            return None
        if source.mem.base != 0 or source.mem.index != 0:
            return None
        row_index = indices[0]
        if rows[row_index].type != "DIR32":
            return None
        return destination.reg, row_index

    equivalent: dict[int, str] = {}
    for ins_index in range(len(instructions)):
        first = relocated_memory_operand(ins_index, "mov")
        if first is None:
            continue
        register, first_row = first
        chain = [first_row]
        kind = ""
        next_index = ins_index + 1
        if next_index < len(instructions):
            factor = relocated_memory_operand(next_index, "imul", register)
            if factor is not None:
                chain.append(factor[1])
                kind = "commutative-multiply"
            else:
                while next_index < len(instructions):
                    term = relocated_memory_operand(next_index, "add", register)
                    if term is None:
                        break
                    chain.append(term[1])
                    next_index += 1
                if len(chain) > 1:
                    kind = "commutative-add"
        if not kind:
            continue
        chain_rows = [rows[row_index] for row_index in chain]
        if any(row.expected is None or row.actual is None
               for row in chain_rows):
            continue
        if Counter(row.expected for row in chain_rows) != Counter(
                row.actual for row in chain_rows):
            continue
        for row_index in chain:
            if rows[row_index].expected != rows[row_index].actual:
                equivalent[row_index] = kind
    return equivalent


def relocation_identities(
        name: str, tu: str, *, win: Optional[WinImage] = None,
        ctu: Optional[CompiledTU] = None) -> list[RelocationIdentity]:
    """Compare each linked relocation target with its named COFF symbol.

    Normal byte verification masks relocations because the linker owns their
    bytes.  This companion audit verifies the identity of every relocation
    whose source symbol has a Windows global/function mapping.
    """
    win = win or load_win_image()
    ctu = ctu or compile_tu(tu)
    mapped = win_va_for(name, tu)
    if mapped is None or mapped[0] in _WIN_SENTINELS:
        return []
    verdict = verify_func(name, tu, win=win, ctu=ctu)
    function_va = verdict.located_va or mapped[0]
    rows: list[RelocationIdentity] = []
    for relocation in ctu.func_relocations(name):
        resolved = resolve_win_symbol(relocation.symbol, relocation.type)
        expected = resolved[0] + relocation.addend if resolved else None
        confidence = resolved[1] if resolved else ""
        linked = win.func_bytes(function_va + relocation.offset, 4)
        actual: Optional[int] = None
        type_name = f"{relocation.type:#x}"
        if len(linked) == 4:
            if relocation.type == _IMAGE_REL_I386_DIR32:
                actual = struct.unpack("<I", linked)[0]
                type_name = "DIR32"
            elif relocation.type == _IMAGE_REL_I386_REL32:
                displacement = struct.unpack("<i", linked)[0]
                actual = function_va + relocation.offset + 4 + displacement
                type_name = "REL32"
        if resolved is None and actual is not None:
            raw_candidates, alias_candidates = _crt_symbol_candidates()
            candidates = raw_candidates.get(relocation.symbol)
            if candidates is None:
                candidates = alias_candidates.get(
                    _coff_source_name(relocation.symbol))
            if (candidates is not None
                    and actual - relocation.addend in candidates):
                expected = actual
                confidence = "crt:archive-xref"
        if resolved is None and actual is not None:
            link_match = _link_symbol_corpus().get(
                _coff_source_name(relocation.symbol))
            if link_match is not None:
                confidence = (f"{link_match['ownership']}:"
                              f"{link_match['resolution']}")
        if resolved is None and relocation.symbol.startswith("$L"):
            local_offset = ctu.local_symbols.get(relocation.symbol)
            function_offset = ctu.func_start(name)
            if local_offset is not None and function_offset is not None:
                expected = (function_va + local_offset - function_offset
                            + relocation.addend)
                confidence = "compiler-label:function-relative"
        if resolved is None and relocation.type == _IMAGE_REL_I386_DIR32:
            literal = ctu.data_symbols.get(relocation.symbol)
            if literal is not None:
                hits = win.find_bytes(literal)
                if len(hits) == 1:
                    expected = hits[0] + relocation.addend
                    confidence = "compiler-literal:unique"
                elif actual is not None and actual - relocation.addend in hits:
                    expected = actual
                    confidence = "compiler-literal:xref"
        rows.append(RelocationIdentity(
            name, relocation.offset, _coff_source_name(relocation.symbol),
            type_name, relocation.addend, expected, actual, confidence))
    code_and_mask = ctu.func_code(name)
    if code_and_mask is not None:
        equivalences = _commutative_relocation_equivalences(
            code_and_mask[0], rows)
        if equivalences:
            rows = [
                replace(row, equivalence=equivalences.get(index, ""))
                for index, row in enumerate(rows)
            ]
    return rows


def _global_label(addr: int) -> str:
    name = _globals_reverse().get(addr)
    return f"`{name}`" if name else f"global @ {addr:#x} (unmapped)"


def lever_hints(v: "FuncVerdict") -> list[LeverHint]:
    """Scan the aligned win diff for known source-lever fingerprints."""
    if v.status != "diff":
        return []
    return lever_hints_from_rows(aligned_diff(v), v.struct_diff)


def lever_hints_from_rows(rows: list[dict],
                          struct_diff: Optional[int]) -> list[LeverHint]:
    """Detector core over pre-computed aligned-diff rows (testable)."""
    import re

    hints: list[LeverHint] = []
    zext_al = re.compile(r"mov al, byte ptr \[(0x[0-9a-f]+)\]")
    ebp_slot = re.compile(r"\[ebp - (0x[0-9a-f]+|\d)\]")

    slot_swaps = 0
    slot_anchor = -1
    i = 0
    while i < len(rows):
        r = rows[i]
        o, t = r["ours"], r["theirs"]

        if r["kind"] == "struct":
            # signedness: our movsx-byte replaced by CAESAR2's zext pair
            if o.startswith("movsx") and "byte ptr" in o and t == "xor eax, eax":
                for r2 in rows[i + 1:i + 3]:
                    m = zext_al.search(r2["theirs"])
                    if m:
                        hints.append(LeverHint(
                            "signedness",
                            f"{_global_label(int(m.group(1), 16))} is "
                            "zero-extended in CAESAR2.EXE -> declare it "
                            "`unsigned char` (byte-neutral for Watcom)", i))
                        break
            # reverse: we zero-extend, CAESAR2 sign-extends
            if o == "xor eax, eax" and t.startswith("movsx") and "byte ptr" in t:
                m = re.search(r"\[(0x[0-9a-f]+)\]", t)
                if m:
                    hints.append(LeverHint(
                        "signedness",
                        f"{_global_label(int(m.group(1), 16))} is "
                        "SIGN-extended in CAESAR2.EXE -> declare it bare "
                        "`char` / `signed char`", i))
            # mirrored conditional jump
            om, tm = o.split(" ")[0] if o else "", t.split(" ")[0] if t else ""
            if (om, tm) in _JCC_MIRROR:
                hints.append(LeverHint(
                    "jcc_mirror",
                    f"`{om}` vs `{tm}`: comparison emission is mirrored -- a "
                    "likely identifier-name / scope-walk lever (see "
                    "docs/msvc-od-slot-allocation.md); confirm the source "
                    "comparison against the Win decompile before rewriting", i))
            # one-sided call runs: platform delta — only when the lone
            # run is a bare call (+ optional stack fixup), i.e. a missing
            # CALL rather than a missing statement block, and the rest of
            # the function is essentially aligned (near-miss).
            if o.startswith("call") and not t and (struct_diff or 99) <= 6:
                run_len = 1
                for r2 in rows[i + 1:i + 3]:
                    if r2["kind"] == "struct" and r2["ours"] and not r2["theirs"]:
                        run_len += 1
                    else:
                        break
                if run_len <= 2:
                    hints.append(LeverHint(
                        "platform_call",
                        "call present only in OUR build -> likely a "
                        "DOS-only call (screen refresh etc.); guard the "
                        "call site with `#if PLATFORM_DOS` if CAESAR2 "
                        "provably lacks it", i))
            if t.startswith("call") and not o and (struct_diff or 99) <= 6:
                run_len = 1
                for r2 in rows[i + 1:i + 3]:
                    if r2["kind"] == "struct" and r2["theirs"] and not r2["ours"]:
                        run_len += 1
                    else:
                        break
                if run_len <= 2:
                    hints.append(LeverHint(
                        "platform_call",
                        "call present only in CAESAR2.EXE -> Windows-only "
                        "call; check `c2 win-decompile` for what it does", i))
            # chained assignment: original `a = b = K` shows as
            # store-K + load-b + store-a; separate statements show as two
            # stores.  Watcom emits identical bytes for both forms, so
            # only the /Od oracle can recover which one the source used.
            if re.match(r"mov (dword|word|byte) ptr \[[^]]*\], ", o) \
                    and re.match(r"mov (e?[a-d]x|[a-d]l), (dword|word|byte) ptr \[", t):
                r2 = rows[i + 1] if i + 1 < len(rows) else None
                if r2 and r2["kind"] == "struct" and not r2["ours"] \
                        and re.match(r"mov (dword|word|byte) ptr \[[^]]*\], (e?[a-d]x|[a-d]l)$", r2["theirs"]):
                    hints.append(LeverHint(
                        "chain_assign",
                        "CAESAR2 re-loads the just-stored value -> the "
                        "original chained the assignment (`a = b = K;`); "
                        "our source uses separate statements", i))
            if re.match(r"mov (e?[a-d]x|[a-d]l), (dword|word|byte) ptr \[", o) \
                    and re.match(r"mov (dword|word|byte) ptr \[[^]]*\], ", t):
                r2 = rows[i + 1] if i + 1 < len(rows) else None
                if r2 and r2["kind"] == "struct" and not r2["theirs"] \
                        and re.match(r"mov (dword|word|byte) ptr \[[^]]*\], (e?[a-d]x|[a-d]l)$", r2["ours"]):
                    hints.append(LeverHint(
                        "chain_assign",
                        "OUR build re-loads the just-stored value -> our "
                        "source chains the assignment (`a = b = K;`); the "
                        "original used separate statements", i))
            # transposed statements: X;Y vs Y;X in a replace block
            if o and t and i + 1 < len(rows):
                r2 = rows[i + 1]
                if r2["kind"] == "struct" and r2["ours"] and r2["theirs"] \
                        and _norm_op(o) == _norm_op(r2["theirs"]) \
                        and _norm_op(r2["ours"]) == _norm_op(t) \
                        and o.split(" ")[0] != r2["ours"].split(" ")[0]:
                    hints.append(LeverHint(
                        "stmt_order",
                        "adjacent instructions transposed -> statement "
                        "order swap in the source", i))

        if r["kind"] in ("slot", "struct") and o and t:
            # per-platform immediate macro (open() flags & co)
            oi = re.findall(r"0x[0-9a-f]+", o)
            ti = re.findall(r"0x[0-9a-f]+", t)
            if oi and ti and o.split(" ")[0] == t.split(" ")[0]:
                for a, b in zip(oi, ti):
                    key = (int(a, 16), int(b, 16))
                    if key in _IMM_MACROS:
                        hints.append(LeverHint(
                            "imm_macro",
                            f"immediate {a} (Watcom) vs {b} (MSVC) -> the "
                            f"original source used {_IMM_MACROS[key]}", i))
            # frame size
            if o.startswith("sub esp,") and t.startswith("sub esp,"):
                om = re.fullmatch(r"sub esp,\s*(0x[0-9a-f]+|\d+)", o)
                tm = re.fullmatch(r"sub esp,\s*(0x[0-9a-f]+|\d+)", t)
                if om and tm:
                    oa, ta = int(om.group(1), 0), int(tm.group(1), 0)
                    if oa != ta:
                        n = abs(ta - oa) // 4
                        more = "CAESAR2 has" if ta > oa else "our build has"
                        hints.append(LeverHint(
                            "frame_size",
                            f"frame differs by {n} slot(s) ({more} more) -> "
                            "declared-but-unused or block-scoped local(s); "
                            "unused locals still get slots at /Od", i))
            # ebp slot swaps
            mo, mt = ebp_slot.search(o), ebp_slot.search(t)
            if mo and mt and mo.group(1) != mt.group(1):
                slot_swaps += 1
                if slot_anchor < 0:
                    slot_anchor = i
        i += 1

    if slot_swaps >= 2:
        hints.append(LeverHint(
            "slot_swap",
            "same instructions on different [ebp-N] slots -> local slot "
            "order differs; slots are keyed on IDENTIFIER NAMES and scope, "
            "not decl order (probe per docs/msvc-od-slot-allocation.md)",
            slot_anchor))
    return hints
