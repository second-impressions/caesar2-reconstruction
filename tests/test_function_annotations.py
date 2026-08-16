"""Corpus checks for the reccmp annotations and function summaries."""

from __future__ import annotations

from collections import Counter
from pathlib import Path
import re

from reccmp.parser import DecompParser
from reccmp.parser.marker import is_marker_exact
from reccmp.parser.node import ParserFunction


SOURCE_ROOT = Path("src")
EXPECTED_DOS_FUNCTIONS = 1448
EXPECTED_WINDOWS_FUNCTIONS = 1398
EXPECTED_REORDERED = {
    ("C2WIN", 0x004B4A6F),
    ("C2", 0x32409),
    ("C2WIN", 0x004792EC),
    ("C2", 0x4DC94),
    ("C2", 0x52E40),
    ("C2", 0x2AF8E),
    ("C2", 0x2550B),
    ("C2WIN", 0x0044B7E1),
    ("C2", 0x283FA),
    ("C2WIN", 0x004841EB),
    ("C2WIN", 0x00483FD3),
    ("C2WIN", 0x00484208),
    ("C2WIN", 0x004841FD),
    ("C2WIN", 0x004841CE),
    ("C2WIN", 0x0048ED84),
    ("C2WIN", 0x004AA98D),
    ("C2WIN", 0x004AEABE),
}

DECOMP_LOG = re.compile(
    r"(?i)(?:byte[- ]exact|\bresidue\b|\bregalloc\b|\brover\b|"
    r"\ballocator\b|\bcodegen\b|\bdecompil|\bghidra\b|\bwatcom\b|"
    r"win-oracle|\boracle\b|\brule\s+\d+|\b2026-\d\d-\d\d|"
    r"\bPS\b|\bCAESAR2\.EXE\b|source[- ]shape|load-bearing|\bICF\b|"
    r"tail[- ]merge|\bepilogue\b|\bsource order\b|\bdebug symbols\b|"
    r"address order proves|hauled head|source position|symbol boundary|"
    r"\bdiff(?:erence|ing)?\b)"
)


def _parse(path: Path):
    parser = DecompParser()
    parser.reset_and_set_filename(path)
    parser.read(path.read_text(encoding="utf-8"))
    parser.finish()
    return parser.to_result()


def test_every_marker_is_canonical_and_attached_to_its_definition():
    dos_count = 0
    windows_count = 0

    for path in SOURCE_ROOT.glob("*.c"):
        lines = path.read_text(encoding="utf-8").splitlines()
        for index, line in enumerate(lines):
            if not line.startswith("// FUNCTION:"):
                continue

            assert is_marker_exact(line), f"non-canonical marker at {path}:{index + 1}"
            if line.startswith("// FUNCTION: C2WIN "):
                windows_count += 1
                continue

            assert line.startswith("// FUNCTION: C2 ")
            dos_count += 1
            assert index > 0 and lines[index - 1].startswith("// ")
            assert not lines[index - 1].startswith("// FUNCTION:")

            signature = index + 1
            while lines[signature].startswith("// FUNCTION:"):
                signature += 1
            assert "(" in lines[signature], (
                f"marker is not adjacent to a definition at {path}:{index + 1}"
            )

    assert dos_count == EXPECTED_DOS_FUNCTIONS
    assert windows_count == EXPECTED_WINDOWS_FUNCTIONS


def test_annotations_parse_without_syntax_alerts_and_mark_all_aliases_folded():
    functions: list[ParserFunction] = []
    for path in SOURCE_ROOT.glob("*.c"):
        result = _parse(path)
        assert not result.alerts, f"{path}: {result.alerts}"
        functions.extend(
            token for token in result.tokens if isinstance(token, ParserFunction)
        )

    assert Counter(function.module for function in functions) == {
        "C2": EXPECTED_DOS_FUNCTIONS,
        "C2WIN": EXPECTED_WINDOWS_FUNCTIONS,
    }
    assert not any(function.lookup_by_name for function in functions)
    assert {
        (function.module, function.offset)
        for function in functions
        if function.is_reordered
    } == EXPECTED_REORDERED

    dos_addresses = Counter(
        function.offset for function in functions if function.module == "C2"
    )
    for function in functions:
        if function.module != "C2":
            continue
        assert function.is_folded == (dos_addresses[function.offset] > 1)


def test_function_summaries_contain_no_decompilation_log_residue():
    for path in SOURCE_ROOT.glob("*.c"):
        lines = path.read_text(encoding="utf-8").splitlines()
        for index, line in enumerate(lines):
            if not line.startswith("// FUNCTION: C2 "):
                continue
            summary = []
            cursor = index - 1
            while cursor >= 0 and lines[cursor].startswith("// "):
                summary.append(lines[cursor][3:])
                cursor -= 1
            text = " ".join(reversed(summary))
            assert text
            assert len(summary) <= 3
            assert len(text) <= 300
            assert DECOMP_LOG.search(text) is None, (
                f"decompilation log above {path}:{index + 1}: {text}"
            )


def test_source_comments_contain_no_decompilation_log_residue():
    for path in SOURCE_ROOT.glob("*.c"):
        for index, line in enumerate(path.read_text(encoding="utf-8").splitlines()):
            stripped = line.lstrip()
            if not stripped.startswith(("//", "/*", "*")):
                continue
            if stripped.startswith("// FUNCTION:"):
                continue
            assert DECOMP_LOG.search(stripped) is None, (
                f"decompilation log at {path}:{index + 1}: {stripped}"
            )
