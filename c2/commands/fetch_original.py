"""``c2 fetch-original`` — download + extract the original PS.EXE from archive.org.

The reconstruction's ground truth is the **debug-symbol build** of
``PS.EXE`` (SHA-256 pinned in ``reccmp-project.yml``).  It ships on
seven of the CD releases preserved in the Impressions Games PC CD Image
Collection on archive.org:

    https://archive.org/details/20231129_20231129_0828

This command automates the manual dance: it downloads one of those CD
zips (default: the smallest, the Germany 1996-12-18 rerelease at
~132 MB), verifies the zip against its archive.org MD5, decompresses
the BIN/CUE CD image into a temporary plain ISO (stripping the raw
2352-byte sector framing on the fly), opens the ISO9660 filesystem with
pycdlib, extracts ``HD/PS.EXE``, verifies its SHA-256 against the
pinned expectation, and installs it at the git-excluded ``original/PS.EXE``.
All intermediates are temp files and are deleted afterwards.
"""

from __future__ import annotations

import hashlib
import io
import tempfile
import urllib.parse
import zipfile
from pathlib import Path
from typing import Annotated, BinaryIO, Optional

import pycdlib
import requests
import typer
from rich.progress import (BarColumn, DownloadColumn, Progress, TextColumn,
                           TransferSpeedColumn)

from c2.original import ensure_original, sha256_of
from c2.reccmp_project import WINDOWS_TARGET_ID, expected_original_hash

ARCHIVE_ITEM = "20231129_20231129_0828"
ARCHIVE_URL = f"https://archive.org/download/{ARCHIVE_ITEM}"

# The seven CD releases that ship the debug-symbol PS.EXE, as preserved
# in the archive.org item (name, zip size, zip md5, zip sha1).  Keep in
# sync with README.md "Getting the original PS.EXE".
CDS: dict[str, tuple[str, int, str, str]] = {
    "germany-1996-12-18": (
        "Caesar II (Germany) (Rerelease) (1996-12-18).zip",
        132189993, "b9f55ea4d2f6e5aec2e49be96ed6be25",
        "31d009b3870dfb01a7ea5b7f54e5deada19883a8"),
    "germany-1996-12-18-alt": (
        "Caesar II (Germany) (Rerelease) (1996-12-18) (Alt).zip",
        132190010, "7b758c0f9757039e0751cfeb6062ef8b",
        "40ce7948d388b7340bf071d7bae006c467f715bb"),
    "usa-1997-11-12": (
        "Caesar II (USA) (Rerelease) (1997-11-12).zip",
        359348782, "9f9ea78b83f67546352915489c4a9e93",
        "90417598441ed8cd58c7a8cc6e1e3863aa1224a6"),
    "usa-1996-08-29": (
        "Caesar II (USA) (Rerelease) (1996-08-29).zip",
        424795362, "5dd30aec3f2cb67f94441ab09180ad0d",
        "e46d4304a1fa460f6902b0e58d8cf1fd9257d40f"),
    "usa-1997-03-10": (
        "Caesar II (USA) (Rerelease) (1997-03-10).zip",
        427346354, "b0a5c283dc181460897c8ad31c82f13c",
        "7e8cf40b3ea4289d1b232ba15bafd5c336267ea2"),
    "europe-1997-09-12": (
        "Caesar II (Europe) (Rerelease) (1997-09-12).zip",
        427487808, "dfdfc4158f57ae48c5f0a54fee4bd856",
        "2618c825f3b11a84f848ee46e44d0c6dd940630e"),
    "italy-covermount": (
        "Caesar II (Italy) (Covermount).zip",
        435086930, "ee4d7d6fd3a980bf92ac6dab1500649e",
        "20309b95e16777e7133b5b702fe7ca4b61c28828"),
}
DEFAULT_CD = "germany-1996-12-18"

ISO_PS_EXE = "/HD/PS.EXE;1"     # path of the original inside the CD
# The Windows source witness (build A) rides on the hybrid CDs beside it.
ISO_CAESAR2_EXE = "/C2WIN95/HD/CAESAR2.EXE;1"

RAW_SECTOR = 2352               # raw CD sector (MODE1 or MODE2/XA framing)
ISO_SECTOR = 2048
_SYNC = b"\x00" + b"\xff" * 10 + b"\x00"


def _progress() -> Progress:
    return Progress(
        TextColumn("  {task.description}"),
        BarColumn(),
        DownloadColumn(),
        TransferSpeedColumn(),
    )


def strip_raw_sectors(src: BinaryIO, dst: BinaryIO, raw_size: int,
                      progress: Optional[Progress] = None) -> None:
    """Convert a raw 2352-byte-sector BIN stream into a plain 2048-byte ISO.

    Sniffs the sector framing from the first sector's sync + mode byte:

    * MODE1/2352  — 12 sync + 4 header, user data at offset 16;
    * MODE2/2352  — 12 sync + 4 header + 8 XA subheader, user data at
      offset 24 (Form 1; Caesar II CDs are CD-ROM XA — e.g. the Germany
      1996-12-18 cue says ``TRACK 01 MODE2/2352``).
    """
    if raw_size % RAW_SECTOR:
        raise ValueError(
            f"BIN size {raw_size} is not a multiple of {RAW_SECTOR}")
    task = (progress.add_task("converting BIN → ISO", total=raw_size)
            if progress else None)
    data_off: Optional[int] = None
    for _ in range(raw_size // RAW_SECTOR):
        raw = src.read(RAW_SECTOR)
        if len(raw) != RAW_SECTOR:
            raise EOFError("CD image ended mid-sector")
        if data_off is None:
            if raw[:12] != _SYNC:
                raise ValueError("no CD sector sync pattern in BIN")
            mode = raw[15]
            if mode == 1:
                data_off = 16
            elif mode == 2:
                data_off = 24
            else:
                raise ValueError(f"unsupported CD sector mode {mode}")
        dst.write(raw[data_off:data_off + ISO_SECTOR])
        if progress:
            progress.advance(task, RAW_SECTOR)


def extract_from_iso(iso_path: Path, iso_file: str) -> Optional[bytes]:
    """Extract one file from a plain ISO9660 image; None if it is absent."""
    iso = pycdlib.PyCdlib()
    iso.open(str(iso_path))
    try:
        buf = io.BytesIO()
        try:
            iso.get_file_from_iso_fp(buf, iso_path=iso_file)
        except pycdlib.pycdlibexception.PyCdlibInvalidInput:
            return None
        return buf.getvalue()
    finally:
        iso.close()


def extract_ps_exe_from_iso(iso_path: Path) -> bytes:
    """Extract HD/PS.EXE from a plain ISO9660 image."""
    data = extract_from_iso(iso_path, ISO_PS_EXE)
    if data is None:
        raise ValueError(f"{ISO_PS_EXE} is not on this CD")
    return data


def _download(url: str, dest: Path, expected_size: int,
              expected_md5: str) -> None:
    md5 = hashlib.md5()
    done = 0
    with _progress() as progress, dest.open("wb") as out:
        task = progress.add_task("downloading", total=expected_size)
        with requests.get(url, stream=True, timeout=120,
                          headers={"User-Agent": "c2-fetch-original/1"}) as resp:
            resp.raise_for_status()
            for chunk in resp.iter_content(chunk_size=1 << 20):
                out.write(chunk)
                md5.update(chunk)
                done += len(chunk)
                progress.advance(task, len(chunk))
    if done != expected_size:
        raise ValueError(f"download truncated: {done} of {expected_size} bytes")
    if md5.hexdigest() != expected_md5:
        raise ValueError(
            f"zip md5 mismatch: got {md5.hexdigest()}, expected {expected_md5}")


def _verify_zip(zip_path: Path, expected_size: int, expected_md5: str) -> bool:
    if zip_path.stat().st_size != expected_size:
        return False
    md5 = hashlib.md5()
    with zip_path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            md5.update(chunk)
    return md5.hexdigest() == expected_md5


def fetch_original(
    cd: Annotated[str, typer.Option(
        "--cd", help=f"CD release to fetch ({', '.join(CDS)}).")] = DEFAULT_CD,
    dest: Annotated[Path, typer.Option(
        "--dest", help="Where to install the original.")] = Path("original/PS.EXE"),
    zip_path: Annotated[Optional[Path], typer.Option(
        "--from-zip", help="Use an already-downloaded CD zip instead of "
                           "downloading (still hash-verified).")] = None,
    force: Annotated[bool, typer.Option(
        "--force", help="Overwrite an existing (e.g. wrong-hash) dest.")] = False,
    windows_dest: Annotated[Path, typer.Option(
        "--windows-dest", help="Where to install the Windows build A "
                               "CAESAR2.EXE when the CD carries it.")]
    = Path("original/CAESAR2.EXE"),
) -> None:
    """Download a Caesar II CD image from archive.org and extract the
    original debug-symbol PS.EXE to its expected location.

    Hybrid CDs (usa-1996-08-29, europe-1997-09-12, italy-covermount) also
    carry the Windows source witness, build A of CAESAR2.EXE; it is
    installed beside PS.EXE when its hash matches the pinned C2WIN target."""
    if cd not in CDS:
        raise typer.BadParameter(f"unknown --cd (choose from: {', '.join(CDS)})")
    name, zsize, zmd5, _zsha1 = CDS[cd]
    expected_windows = expected_original_hash(target_id=WINDOWS_TARGET_ID)

    have_windows = (windows_dest.is_file() and not force and
                    sha256_of(windows_dest) == expected_windows)
    if dest.is_file() and not force:
        if sha256_of(dest) == expected_original_hash():
            if have_windows or cd in ("germany-1996-12-18",
                                      "germany-1996-12-18-alt",
                                      "usa-1997-11-12", "usa-1997-03-10"):
                typer.echo(f"{dest} already present and hash-verified — nothing to do.")
                return
        else:
            typer.echo(f"{dest} exists but has the WRONG hash; rerun with --force "
                       "to replace it.", err=True)
            raise typer.Exit(1)

    tmp_zip: Optional[Path] = None
    tmp_iso: Optional[Path] = None
    try:
        if zip_path is None:
            url = f"{ARCHIVE_URL}/{urllib.parse.quote(name)}"
            typer.echo(f"fetching {name} ({zsize / (1 << 20):.0f} MiB) "
                       "from archive.org …")
            with tempfile.NamedTemporaryFile(
                    suffix=".zip", prefix="c2-cd-", delete=False) as tf:
                tmp_zip = Path(tf.name)
            _download(url, tmp_zip, zsize, zmd5)
            typer.echo("  zip md5 verified")
            zip_path = tmp_zip
        else:
            if not _verify_zip(zip_path, zsize, zmd5):
                raise typer.BadParameter(
                    f"{zip_path} does not match the pinned {name} "
                    f"(size/md5 mismatch)")
            typer.echo(f"using local {zip_path} (md5 verified)")

        with zipfile.ZipFile(zip_path) as zf:
            images = [i for i in zf.infolist()
                      if i.filename.lower().endswith((".bin", ".iso"))]
            if not images:
                raise ValueError("no .bin/.iso CD image inside the zip")
            image = max(images, key=lambda i: i.file_size)
            with tempfile.NamedTemporaryFile(
                    suffix=".iso", prefix="c2-cd-", delete=False) as tf:
                tmp_iso = Path(tf.name)
            with zf.open(image) as src, tmp_iso.open("wb") as dst:
                if image.filename.lower().endswith(".bin"):
                    with _progress() as progress:
                        strip_raw_sectors(src, dst, image.file_size, progress)
                else:
                    while chunk := src.read(1 << 20):
                        dst.write(chunk)

        data = extract_ps_exe_from_iso(tmp_iso)

        actual = hashlib.sha256(data).hexdigest()
        expected = expected_original_hash()
        if actual != expected:
            raise ValueError(
                f"extracted PS.EXE sha256 {actual} != expected {expected} "
                f"(wrong CD build?)")

        if not (dest.is_file() and sha256_of(dest) == expected):
            dest.parent.mkdir(parents=True, exist_ok=True)
            tmp_out = dest.with_suffix(".tmp")
            tmp_out.write_bytes(data)
            tmp_out.replace(dest)
            typer.echo(f"installed {dest} ({len(data):,} bytes, sha256 verified)")
        ensure_original(dest)

        windows = extract_from_iso(tmp_iso, ISO_CAESAR2_EXE)
        if windows is None:
            typer.echo(f"this CD carries no {ISO_CAESAR2_EXE}; the Windows "
                       "witness comes from usa-1996-08-29, europe-1997-09-12 "
                       "or italy-covermount")
        elif hashlib.sha256(windows).hexdigest() != expected_windows:
            typer.echo(f"this CD's CAESAR2.EXE is not build {WINDOWS_TARGET_ID} "
                       "(sha256 differs); not installed")
        elif not have_windows:
            windows_dest.parent.mkdir(parents=True, exist_ok=True)
            tmp_out = windows_dest.with_suffix(".tmp")
            tmp_out.write_bytes(windows)
            tmp_out.replace(windows_dest)
            typer.echo(f"installed {windows_dest} ({len(windows):,} bytes, "
                       "sha256 verified)")
    finally:
        for tmp in (tmp_zip, tmp_iso):
            if tmp is not None and tmp.exists():
                tmp.unlink()
