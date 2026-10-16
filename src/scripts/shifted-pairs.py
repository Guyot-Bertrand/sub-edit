#!/usr/bin/env python3
"""The shifted pairs: a main file, a translation laid at a known offset, and the
truth of that offset.

The detection of a shifted translation (phase 14, D10) looks for a constant offset
and must never propose one the file does not have. Testing it requires knowing what
was laid down: a fixture drawn from what the detection finds would only prove that
it agrees with itself. This script **lays down** the offset, writes it alongside
(`verite.txt`), and the detection is held against it.

    --check       (default) rebuild each case and compare it with the disk
    --generate    rewrite the cases from the table

Each case holds three files: `principal.srt`, `traduction.srt` and `verite.txt`,
which carries the offset laid down in milliseconds (positive when the translation
is late), or `aucun` ("none") when the lines have no constant offset — a drift, a
partial offset, or no offset at all.

**The positions are arithmetic**, never a call to our commands: shifting with
`ShiftCommand` would prove subedit against subedit.
"""

from __future__ import annotations

import sys
from collections.abc import Callable
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DIRECTORY = REPO_ROOT / "src" / "test" / "data" / "decalages"

# Start and end, in milliseconds. Gaps that do not repeat: a periodic main file
# would make two offsets equally good, and the detection would then rightly
# propose neither.
MAIN = [
    (1000, 3000), (4200, 5800), (8000, 11000), (12500, 14000),
    (17000, 19500), (20200, 21700), (25000, 28000), (29500, 31000),
    (34000, 36500), (37300, 38800), (42000, 45000), (46500, 48000),
]  # fmt: skip


def constant(milliseconds: int) -> Callable[[int, int], int]:
    return lambda _rank, _start: milliseconds


def drift(total: int) -> Callable[[int, int], int]:
    """A delay that grows from zero to `total` over the length of the file."""
    return lambda rank, _start: round(total * rank / (len(MAIN) - 1))


def partial(milliseconds: int) -> Callable[[int, int], int]:
    """The delay touches only the second half of the file."""
    return lambda rank, _start: milliseconds if rank >= len(MAIN) // 2 else 0


# The case's name, what the offset does to the line of rank n, and the truth.
CASES: list[tuple[str, Callable[[int, int], int], str]] = [
    ("sans-decalage", constant(0), "aucun"),
    ("constant-plus-2000", constant(2000), "2000"),
    ("constant-moins-1500", constant(-1500), "-1500"),
    ("constant-plus-2340", constant(2340), "2340"),
    ("derive", drift(3000), "aucun"),
    ("partiel", partial(2000), "aucun"),
]


def stamp(milliseconds: int) -> str:
    sign = "-" if milliseconds < 0 else ""
    value = abs(milliseconds)
    return f"{sign}{value // 3600000:02d}:{value // 60000 % 60:02d}:{value // 1000 % 60:02d},{value % 1000:03d}"


def srt(rows: list[tuple[int, int, str]]) -> str:
    return "\n".join(
        f"{rank}\n{stamp(start)} --> {stamp(end)}\n{text}\n"
        for rank, (start, end, text) in enumerate(rows, 1)
    )


def build(shift: Callable[[int, int], int], truth: str) -> dict[str, str]:
    main = [(s, e, f"Line {n}.") for n, (s, e) in enumerate(MAIN, 1)]
    translated = []
    for rank, (start, end) in enumerate(MAIN):
        late = shift(rank, start)
        translated.append((start + late, end + late, f"Ligne {rank + 1}."))
    return {
        "principal.srt": srt(main),
        "traduction.srt": srt(translated),
        "verite.txt": truth + "\n",
    }


def expected() -> dict[Path, str]:
    files: dict[Path, str] = {}
    for name, shift, truth in CASES:
        for file, content in build(shift, truth).items():
            files[DIRECTORY / name / file] = content
    return files


def main(argv: list[str]) -> int:
    wanted = expected()
    if "--generate" in argv:
        for path, content in wanted.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        return 0

    failures = []
    for path, content in wanted.items():
        if not path.exists():
            failures.append(f"missing: {path.relative_to(REPO_ROOT)}")
        elif path.read_text(encoding="utf-8") != content:
            failures.append(f"differs: {path.relative_to(REPO_ROOT)}")
    known = {path.parent.name for path in wanted}
    if DIRECTORY.exists():
        for entry in sorted(DIRECTORY.iterdir()):
            if entry.is_dir() and entry.name not in known:
                failures.append(f"unexpected: {entry.relative_to(REPO_ROOT)}")
    for failure in failures:
        print(failure, file=sys.stderr)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
