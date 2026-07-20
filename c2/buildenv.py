"""Shared Watcom build-environment helpers.

Extracted from the retired per-function byte oracle
(``c2.commands.decomp_verify``) when the diagnostic burn-down tooling was
removed after the corpus reached byte-exact (2026-07).  These are the
pieces the surviving reconstruction toolchain (``c2 rebuild``,
``c2 delink``, ``c2 disasm``) still needs:

* the proven compiler flags (``PS_CFLAGS``) and toolchain image
  (``_STOCK_IMAGE``),
* the podman container runner (``_run_in_container``),
* the DOS-FAT-bucket-aware staging writer (``_write_if_changed``),
* the masked byte comparison (``_compare_bytes``) used by the
  final-link audit.
"""

from __future__ import annotations

import os
import shlex
import subprocess
import uuid
from pathlib import Path

import capstone

_CS = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
_CS.detail = False

_STOCK_IMAGE = "localhost/watcom-10.0a-wibo"

# The canonical PS.EXE compile flags (Watcom 10.0a; default OptSize=50,
# unsigned char).  Proven settled — per-flag byte-level proofs in the
# watcom10.0a sibling repo (docs/watcom-10.0a-flags.md).
# -dPLATFORM_DOS=1 selects the DOS platform in include/c2_target.h; it is
# a preprocessor define only (codegen-neutral — the header defaults to the
# same platform without it).
PS_CFLAGS = "-bt=dos -mf -4r -s -d1 -dPLATFORM_DOS=1"

def _run_in_container(
    work: Path,
    image: str,
    command: str,
    timeout: int = 120,
) -> tuple[bool, str]:
    """Run a single DOS command inside the container.

    Returns (success, filtered_output).

    Notes:
      * The container is given an explicit ``--name`` so we can
        force-kill it on Python-side timeout — ``--rm`` alone does
        not stop a running container when ``podman run`` is killed
        from outside (the container keeps running headless until
        its work completes), which previously left zombies behind.
      * ``wmake`` *interactively prompts* "Should this file be
        deleted [Yes/No]?" when a compile target's command fails.
        That prompt blocks forever in our headless setup.  Callers
        that invoke wmake should pass the ``-e`` option (erase
        failed targets without prompting).
    """
    container_name = f"c2vrf_{uuid.uuid4().hex[:12]}"
    cmd = [
        "podman", "run", "--rm",
        "--name", container_name,
        # owner pid label kept for compatibility with external reapers.
        "--label", f"c2_owner_pid={os.getpid()}",
        "-v", f"{work}:/src",
        image,
        # wibo shim convention: argv, not a single command string
        *shlex.split(command),
    ]
    try:
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=timeout
        )
    except subprocess.TimeoutExpired:
        # ``podman run`` died (Python killed it) but the container
        # itself is still running.  Force-stop and clean up.
        subprocess.run(
            ["podman", "kill", container_name],
            capture_output=True, timeout=10,
        )
        subprocess.run(
            ["podman", "rm", "-f", container_name],
            capture_output=True, timeout=10,
        )
        raise
    combined = result.stdout + "\n" + result.stderr
    filtered = "\n".join(
        ln for ln in combined.splitlines()
        if not any(skip in ln for skip in (
            "Running dosemu2", "recommended with", "dosemu -s", "DOSEMU",
        ))
    ).strip()
    return result.returncode == 0, filtered


def _write_if_changed(path: Path, content: bytes) -> bool:
    """Write *content* to *path* only when it differs from the existing
    file.  Returns True if a write happened (mtime now newer).

    DOS 2-SECOND-BUCKET STALENESS FIX (root-caused 2026-07-09): wmake
    runs INSIDE dosemu, whose redirector presents Linux mtimes truncated
    to absolute 2-second DOS FAT buckets, and wmake treats an EQUAL
    timestamp as up-to-date.  A changed .c staged < 2 s after its .obj
    was written -- same even-second bucket -- was therefore silently
    NOT recompiled (the long-standing "rarely goes stale, wrong diff
    count" build-cache bug; deterministic repro: obj@T+0.2 c@T+1.8
    skips, c@T+2.1 rebuilds).  Fix: after a real write, if any sibling
    .obj lives in the same-or-later DOS bucket, bump this file's mtime
    to the start of the NEXT bucket (< 2 s into the future -- harmless,
    and strictly newer in DOS time).  Sibling scan: this file's own
    .obj for sources; ALL .obj for shared deps (headers / makefile,
    which every rule depends on)."""
    if path.exists() and path.read_bytes() == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)
    objs = ([path.with_suffix(".obj")] if path.suffix in (".c", ".asm")
            else list(path.parent.glob("*.obj")))
    try:
        latest = max((o.stat().st_mtime for o in objs if o.exists()),
                     default=None)
        if latest is not None:
            my_bucket = int(path.stat().st_mtime) // 2
            obj_bucket = int(latest) // 2
            if my_bucket <= obj_bucket:
                t = (obj_bucket + 1) * 2
                os.utime(path, (t, t))
    except OSError:
        pass
    return True


# ── Masked byte comparison ───────────────────────────────────────────────────

def _rel_call_jmp_disp_mask(code: bytes) -> set[int]:
    """Return byte offsets within `code` belonging to the displacement field
    of a cross-function relative call/jump.

    Masks:
      - `E8 rel32` (call)        — always cross-function; 4 disp bytes.
      - `E9 rel32` (jmp)         — always cross-function; 4 disp bytes.
      - `0F 8x rel32` (Jcc long) — only if target lies outside the function,
        which happens when Watcom's linker tail-merges shared epilogues.

    These bytes hold link-time-resolved displacements that differ between
    PS.EXE and the small testbench because the call target lands at a
    different physical address — even though the compiler-emitted
    instruction is identical. Masking them eliminates purely link-
    positional noise from the byte-diff counter.
    """
    mask: set[int] = set()
    n = len(code)
    for insn in _CS.disasm(code, 0):
        # E8/E9 rel32 (call/jmp) — always cross-function.
        if insn.size == 5 and insn.bytes[0] in (0xE8, 0xE9):
            for k in range(1, 5):
                mask.add(insn.address + k)
            continue
        # EB rel8 (short jmp) and 7x rel8 (short Jcc): mask only if target
        # lies outside the function (cross-function tail-call/epilogue jump).
        if insn.size == 2 and (insn.bytes[0] == 0xEB or 0x70 <= insn.bytes[0] <= 0x7F):
            disp = int.from_bytes(insn.bytes[1:2], "little", signed=True)
            target = insn.address + insn.size + disp
            if target < 0 or target >= n:
                mask.add(insn.address + 1)
            continue
        # 0F 8x rel32 (long Jcc): mask only if target lies outside function
        # (Watcom's linker tail-merges shared epilogues across functions).
        if insn.size == 6 and insn.bytes[0] == 0x0F and 0x80 <= insn.bytes[1] <= 0x8F:
            disp = int.from_bytes(insn.bytes[2:6], "little", signed=True)
            target = insn.address + insn.size + disp
            if target < 0 or target >= n:
                for k in range(2, 6):
                    mask.add(insn.address + k)
    return mask


def _compare_bytes(
    orig: bytes,
    recomp: bytes,
    orig_off: int,
    recomp_off: int,
    orig_fix: set[int],
    recomp_fix: set[int],
) -> list[int]:
    """Return list of offsets where bytes differ.

    Masks both LE-fixup bytes (loader-patched absolute addresses) and
    relative-call/jmp displacement bytes (link-positional noise).
    """
    n = min(len(orig), len(recomp))
    rel_orig   = _rel_call_jmp_disp_mask(orig[:n])
    rel_recomp = _rel_call_jmp_disp_mask(recomp[:n])
    diffs: list[int] = []
    for i in range(n):
        if i in rel_orig or i in rel_recomp:
            continue
        o = 0 if (orig_off + i)   in orig_fix   else orig[i]
        r = 0 if (recomp_off + i) in recomp_fix else recomp[i]
        if o != r:
            diffs.append(i)
    return diffs
