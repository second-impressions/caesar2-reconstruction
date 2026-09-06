"""``c2 win-verify`` -- byte-verify the decomp source against CAESAR2.EXE.

The Windows analogue of ``c2 decomp-verify``.  ``decomp-verify`` builds the
tree with Watcom and compares against the DOS ``PS.EXE``; ``win-verify``
builds each TU with **MSVC 4.0 /Od** (the proven CAESAR2.EXE toolchain) and
compares each function against the Windows ``CAESAR2.EXE`` build -- a second,
independent byte oracle on the SAME source tree.

Two figures per function (mirroring the dual oracle/shape split in
``decomp-verify``):

* **byte_diff** -- the raw masked byte diff (the ORACLE; 0 ⇒ byte-exact).
* **struct_diff** -- mnemonic + reloc/displacement-normalised operand
  mismatches (the WORKABLE figure: /Od shuffles stack slots, so raw byte
  diff is noisy -- structural diff isolates real shape divergence).

A function is **exact** when its compiled bytes match somewhere in
``.text`` under DIR32/REL32 masking (map-independent, so a stale func-map
entry never yields a false diff).

Results are cached at ``.c2-cache/win-verify.json`` (whole-tree, incremental
on changed TUs, full rebuild when a header changes) -- the Win mirror of
``.c2-cache/verify.json``.  ``c2 decomp-verify --target win`` is the thin
front door that dispatches into this engine; both render from the same cache.

Usage::

    c2 win-verify                       # whole-tree summary (cached)
    c2 win-verify totalXpercent         # one function's verdict (cache-or-verify)
    c2 win-verify -v find_enemy         # + the structural PS-vs-RC asm diff
    c2 win-verify --file pcsound.c      # every decompiled function in a TU
    c2 win-verify --files-only          # compact authoritative per-TU census
    c2 win-verify --diffing             # only the not-yet-exact functions
    c2 win-verify --json                # structured {summary,files,functions}
    c2 win-verify --no-cache            # force a fresh MSVC build

See also: ``c2 win-decompile`` (Ghidra /Od source-shape oracle),
``c2 decomp-verify`` (the DOS byte oracle), and
``docs/windows-dual-target-feasibility.md``.
"""
from __future__ import annotations

from pathlib import Path
from typing import Annotated, Optional

import typer
from rich.console import Console
from rich.markup import escape

from c2 import win_bytes as wb
from c2 import win_verify_cache as wvc

console = Console()


# ── per-function diff view ────────────────────────────────────────────────────
def _print_diff(v: "wb.FuncVerdict") -> None:
    """Aligned MSVC-of-our-source vs CAESAR2.EXE (difflib insert/del/replace).

    Two tiers of divergence are coloured distinctly:
      [yellow]≠[/] structural  -- different mnemonic/addressing shape (real),
      [cyan]·[/]  slot/imm noise -- same shape, only a displacement/immediate
                  differs (the /Od stack-slot shuffle; usually not a source bug).
    """
    if v.win_va is None:
        return
    console.print(
        f"\n[bold]{v.name}[/]  ({v.size} B)   "
        f"[dim]OUR SOURCE via MSVC /Od  │  CAESAR2.EXE @ {v.win_va:#x} "
        f"(map: {v.confidence})[/]")
    _MARK = {"equal": ("  ", None), "slot": ("· ", "cyan"), "struct": ("≠ ", "yellow")}
    for row in wb.aligned_diff(v):
        oa, ta = escape(row["ours"]), escape(row["theirs"])
        mark, col = _MARK[row["kind"]]
        if col:
            console.print(f"  [{col}]{mark}{oa:<34}[/] │ [{col}]{ta}[/]", highlight=False)
        else:
            console.print(f"  {mark}{oa:<34} │ {ta}", highlight=False)
    _print_lever_hints(v)


_HINT_COLOURS = {"signedness": "green", "jcc_mirror": "green",
                 "chain_assign": "green",
                 "slot_swap": "green", "frame_size": "green",
                 "imm_macro": "green", "stmt_order": "green",
                 "platform_call": "magenta"}


def _print_lever_hints(v: "wb.FuncVerdict") -> None:
    """Actionable lever suggestions under the aligned diff (see
    ``wb.lever_hints``).  Green = probeable source lever with a known
    mechanism; magenta = suspected platform delta (ifdef territory)."""
    try:
        hints = wb.lever_hints(v)
    except Exception:  # noqa: BLE001  (hints must never break the diff view)
        return
    for h in hints:
        col = _HINT_COLOURS.get(h.kind, "green")
        console.print(f"  [{col}]↳ {h.kind}[/]: {escape(h.detail)}",
                      highlight=False)


def _verdict_line(v: "wb.FuncVerdict") -> str:
    if v.status == "exact":
        return f"[green]✓[/] {v.name} — MSVC byte-exact vs CAESAR2.EXE ({v.size}b)"
    if v.status == "diff":
        return (f"[yellow]≠[/] {v.name} — diff: "
                f"{v.struct_diff}/{v.insn_total} struct  "
                f"[dim](win {v.win_va:#x}, map {v.confidence})[/]")
    if v.status == "nomap":
        return f"[dim]?[/] {v.name} — no CAESAR2.EXE mapping (compiles, {v.size}b)"
    if v.status == "discarded":
        return f"[dim]·[/] {v.name} — compiled, then discarded by the Windows linker"
    if v.confidence == "known-absent":
        return f"[dim]·[/] {v.name} — absent from CAESAR2.EXE ({v.size}b compiled)"
    return f"[red]·[/] {v.name} — no body in {v.tu}.c (stub / not decompiled)"


def _row_verdict_line(r: dict) -> str:
    """Same shape as ``_verdict_line`` but for a cache row (no rebuild)."""
    st = r["status"]
    name = r["name"]
    if st == "exact":
        return f"[green]✓[/] {name} — byte-exact vs CAESAR2.EXE ({r['size']}b)"
    if st == "diff":
        va = r.get("win_va")
        va_s = f"{va:#x}" if isinstance(va, int) else "?"
        return (f"[yellow]≠[/] {name} — diff: "
                f"{r['struct_diff']}/{r['insn_total']} struct  "
                f"[dim](win {va_s}, map {r.get('confidence','')})[/]")
    if st == "nomap":
        return f"[dim]?[/] {name} — no CAESAR2.EXE mapping"
    if st == "discarded":
        return f"[dim]·[/] {name} — compiled, then discarded by the Windows linker"
    if r.get("confidence") == "known-absent":
        return f"[dim]·[/] {name} — absent from CAESAR2.EXE"
    return f"[red]·[/] {name} — no body"


def _file_status(files: dict[str, dict]) -> dict[str, list[dict]]:
    """Classify cached TU summaries without re-counting function rows."""
    groups: dict[str, list[dict]] = {
        "open": [], "exact": [], "absent": [], "discarded": [], "no_bodies": [],
    }
    for tu in sorted(files):
        counts = files[tu]
        row = {"tu": tu, **counts}
        if counts.get("failed_tu") or counts.get("diff") or counts.get("nomap"):
            groups["open"].append(row)
        elif counts.get("exact"):
            groups["exact"].append(row)
        elif counts.get("absent"):
            groups["absent"].append(row)
        elif counts.get("discarded"):
            groups["discarded"].append(row)
        else:
            groups["no_bodies"].append(row)
    return groups


def _render_file_status(files: dict[str, dict], target_tus: Optional[set[str]]) -> None:
    """Render the authoritative compact per-TU Windows census."""
    selected = {tu: counts for tu, counts in files.items()
                if target_tus is None or tu in target_tus}
    groups = _file_status(selected)
    console.rule("[bold]Windows byte-exactness by TU")
    if groups["open"]:
        console.print("[bold yellow]open[/]")
        for row in groups["open"]:
            suffix = f", {row.get('absent', 0)} absent" if row.get("absent") else ""
            if row.get("failed_tu"):
                suffix += ", MSVC build failed"
            console.print(
                f"  {row['tu']}.c: [green]{row.get('exact', 0)} exact[/], "
                f"[yellow]{row.get('diff', 0)} diff[/], "
                f"[dim]{row.get('nomap', 0)} nomap{suffix}[/]")
    if groups["exact"]:
        console.print(
            "[bold green]all comparable bodies exact[/]: "
            + ", ".join(
                f"{row['tu']}.c ({row.get('exact', 0)})"
                + (f" +{row.get('absent', 0)} absent"
                   if row.get("absent") else "")
                + (f" +{row.get('discarded', 0)} link-discarded"
                   if row.get("discarded") else "")
                for row in groups["exact"]))
    if groups["absent"]:
        console.print(
            "[bold]Windows-absent only[/]: "
            + ", ".join(
                f"{row['tu']}.c ({row.get('absent', 0)})"
                for row in groups["absent"]))
    if groups["discarded"]:
        console.print(
            "[bold]Link-discarded only[/]: "
            + ", ".join(
                f"{row['tu']}.c ({row.get('discarded', 0)})"
                for row in groups["discarded"]))
    if groups["no_bodies"]:
        console.print(
            "[dim]no comparable bodies: "
            + ", ".join(f"{row['tu']}.c" for row in groups["no_bodies"])
            + "[/]")


# ── shared renderer: cache-backed summary / file / diffing ─────────────────────
def run(function: Optional[str] = None,
       file: Optional[str] = None,
       verbose: bool = False,
       diffing: bool = False,
       json_out: bool = False,
       no_cache: bool = False,
       files_only: bool = False,
       require_exact: bool = False,
       cb_tu=None) -> None:
    """Cache-backed ``win-verify`` entry point.

    Shared by the ``c2 win-verify`` command and ``c2 decomp-verify --target
    win``.  Resolves to cache rows for the project/`--file` views (rebuilding
    incrementally as needed); for a single function, uses the per-fn
    cache-or-verify path so a stale TU is rebuilt on demand."""
    if not wb.WIN_EXE.exists():
        console.print(f"[red]CAESAR2.EXE not found at {wb.WIN_EXE}[/]")
        raise typer.Exit(1)

    # ── single function: cache-or-verify, render verdict (+ optional diff) ──
    if function:
        tu = wb.tu_of(function)
        if tu is None:
            console.print(f"[red]no definition of {function!r} found in decomp/src[/]")
            raise typer.Exit(1)
        r = wvc.func_row_or_verify(function, force=no_cache)
        if json_out:
            import json
            out = {**r, "diff_rows": _diff_rows_if_diff(function, r)}
            typer.echo(json.dumps(out, indent=1))
            return
        console.print(_row_verdict_line(r))
        if verbose and r["status"] == "diff":
            v = _verdict_from_row(r)
            if v is not None:
                _print_diff(v)
        return

    # ── file or whole tree: refresh the cache, render from rows ──────────────
    if no_cache:
        cache, _ = wvc.refresh(force=True, cb=cb_tu)
    else:
        cache, _ = wvc.refresh(cb=cb_tu)
    files = cache.get("files", {})
    rows = cache.get("functions", [])
    target_tus: Optional[set[str]] = None
    if file:
        target_tus = {Path(file).stem}

    if require_exact:
        s = cache.get("summary", {})
        bad = {k: s.get(k, 0) for k in ("diff", "nomap", "failed_tu")}
        if any(bad.values()):
            console.print(f"[red]NOT byte-exact vs CAESAR2.EXE: {bad}[/]")
            for r in rows:
                if r["status"] in ("diff", "nomap"):
                    console.print("  " + _row_verdict_line(r))
            raise typer.Exit(1)

    if json_out:
        import json
        shown_files = {
            tu: counts for tu, counts in files.items()
            if target_tus is None or tu in target_tus
        }
        summary = cache.get("summary")
        if target_tus is not None:
            keys = ("exact", "diff", "nomap", "absent", "discarded", "failed_tu",
                    "compared")
            summary = {
                key: sum(counts.get(key, 0) for counts in shown_files.values())
                for key in keys
            }
        result = {"summary": summary, "files": shown_files}
        if not files_only:
            result["functions"] = [
                row for row in rows
                if target_tus is None or row["tu"] in target_tus
            ]
        typer.echo(json.dumps(result, indent=1))
        return

    if files_only:
        _render_file_status(files, target_tus)
        s = cache.get("summary", {})
        if target_tus is not None:
            selected = [counts for tu, counts in files.items()
                        if tu in target_tus]
            s = {
                key: sum(counts.get(key, 0) for counts in selected)
                for key in ("exact", "diff", "nomap", "absent", "discarded",
                            "failed_tu", "compared")
            }
        console.print(
            f"[green]{s.get('exact',0)} exact[/]  ·  "
            f"[yellow]{s.get('diff',0)} diff[/]  ·  "
            f"[dim]{s.get('nomap',0)} nomap · {s.get('absent',0)} absent · "
            f"{s.get('discarded',0)} link-discarded · "
            f"{s.get('failed_tu',0)} TU build-fail "
            f"({s.get('compared',0)} compared)[/]")
        return

    diff_rows = []
    tus_show = target_tus or set(files.keys())
    for tu in sorted(tus_show):
        f = files.get(tu, {})
        if f.get("failed_tu"):
            console.print(f"[red]✗ {tu}.c — MSVC compile failed:[/] {f.get('error','')}")
            continue
        tu_rows = [r for r in rows if r["tu"] == tu]
        fe = sum(1 for r in tu_rows if r["status"] == "exact")
        fd = sum(1 for r in tu_rows if r["status"] == "diff")
        fn = sum(1 for r in tu_rows if r["status"] == "nomap")
        if file:
            console.print(f"[bold]{tu}.c[/]: "
                          f"[green]{fe} exact[/], [yellow]{fd} diff[/], "
                          f"[dim]{fn} nomap[/]")
        for r in tu_rows:
            if r["status"] == "diff":
                diff_rows.append(r)
            if verbose and target_tus and r["status"] == "diff":
                v = _verdict_from_row(r)
                if v is not None:
                    _print_diff(v)

    if diffing or file:
        for r in sorted(diff_rows, key=lambda r: r["struct_diff"]):
            console.print("  " + _row_verdict_line(r))

    console.rule("[bold]win-verify summary")
    s = cache.get("summary", {})
    console.print(
        f"[green]{s.get('exact',0)} byte-exact vs CAESAR2.EXE[/]  ·  "
        f"[yellow]{s.get('diff',0)} diff[/]  ·  "
        f"[dim]{s.get('nomap',0)} no-map · {s.get('absent',0)} absent · "
        f"{s.get('discarded',0)} link-discarded · "
        f"{s.get('failed_tu',0)} TU build-fail[/]  "
        f"[dim]({s.get('compared',0)} compared)[/]")
    if not file and not diffing:
        console.print("[dim]→ c2 win-verify --diffing  (workable list) · "
                      "-v <fn>  (structural diff)[/]")


def _verdict_from_row(r: dict) -> Optional["wb.FuncVerdict"]:
    """Rebuild a FuncVerdict from a cache row for the verbose diff view."""
    if not wb.WIN_EXE.exists():
        return None
    tu = r.get("tu") or wb.tu_of(r["name"])
    if not tu:
        return None
    win = wb.load_win_image()
    ctu = wb.compile_tu(tu)
    fc = ctu.func_code(r["name"])
    if fc is None:
        return None
    code, mask = fc
    n = len(code)
    win_va = r.get("win_va")
    if not isinstance(win_va, int):
        return None
    wbytes = win.func_bytes(win_va, n)
    byte_diff = sum(1 for i in range(min(n, len(wbytes)))
                    if i not in mask and code[i] != wbytes[i]) + abs(n - len(wbytes))
    ours = wb.disasm_norm(code, mask)
    theirs = wb.disasm_norm(wbytes)
    struct_diff = wb._struct_distance([x[2] for x in ours], [x[2] for x in theirs])
    return wb.FuncVerdict(r["name"], tu, "diff", n, byte_diff, struct_diff,
                          len(ours), win_va, r.get("confidence", ""), win_va)


def _diff_rows_if_diff(name: str, r: dict) -> list[dict]:
    """Aligned diff rows for JSON output (only when the function diffs)."""
    if r.get("status") != "diff":
        return []
    v = _verdict_from_row(r)
    return wb.aligned_diff(v) if v is not None else []


def win_verify(
    function: Annotated[Optional[str], typer.Argument(
        help="function to verify (omit for a file/tree summary)")] = None,
    file: Annotated[Optional[str], typer.Option(
        "--file", "-f", help="verify every decompiled function in this TU")] = None,
    verbose: Annotated[bool, typer.Option(
        "--verbose", "-v", help="show the structural asm diff for diffs")] = False,
    diffing: Annotated[bool, typer.Option(
        "--diffing", help="summary: list only the not-yet-exact functions")] = False,
    files_only: Annotated[bool, typer.Option(
        "--files-only",
        help="compact authoritative per-TU status; omit function rows")] = False,
    json_out: Annotated[bool, typer.Option(
        "--json", help="emit {summary,files,functions} (or one fn record) as "
                       "JSON on stdout; suppresses textual rendering")] = False,
    no_cache: Annotated[bool, typer.Option(
        "--no-cache", help="force a fresh MSVC build (no .c2-cache reuse)")] = False,
    require_exact: Annotated[bool, typer.Option(
        "--require-exact",
        help="Exit non-zero unless every comparable function is byte-exact "
             "(the CI gate: any diff, unmapped function or TU build failure "
             "fails).")] = False,
) -> None:
    """Byte-verify the recovered functions against the Windows CAESAR2.EXE build."""
    run(function=function, file=file, verbose=verbose, diffing=diffing,
        json_out=json_out, no_cache=no_cache, files_only=files_only,
        require_exact=require_exact)
