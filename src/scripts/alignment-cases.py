#!/usr/bin/env python3
"""The alignment cases: a subtitle file laid at a known fault, what an engine would have heard, and the
truth of where each line is really said.

Re-timing a file on the audio (phase 23) must be held against what is **laid down**, never against what
the method finds: a fixture drawn from the method's own answer would only prove that it agrees with
itself. This script lays the fault down, writes the truth alongside, and the alignment is held against it.

    --check       (default) rebuild each case and compare it with the disk
    --generate    rewrite the cases from the table

Each case is a directory of three files:

- `given.srt`   what the person has: the lines, at the times the fault gives them;
- `heard.json`  what an engine would have heard in the audio, in the form `whisper-cli -ojf` writes it, **at
                the true times**, with the words of the true lines — and nothing for a line it did not hear;
- `truth.txt`   for each line of `given.srt`, in order, where it is really said: `start end` in
                milliseconds, or `-` when it is said nowhere (an extra line, or one the audio does not hold).

**The times are arithmetic**, never a call to our commands: shifting with `ShiftCommand` would prove
subedit against subedit. The words are made up, so that no film's dialogue is carried here.
"""

from __future__ import annotations

import json
import sys
from collections.abc import Callable
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DIRECTORY = REPO_ROOT / "src" / "test" / "data" / "alignment"

# What is really said, in order: start and end in milliseconds, and the words. Gaps that do not repeat, so
# that no two offsets fit equally well.
SPOKEN = [
    (1000, 3000, "The harbour was quiet at dawn."),
    (4200, 5800, "Nobody had lit the lamps yet."),
    (8000, 11000, "She carried the letter across the square."),
    (12500, 14000, "It was addressed to no one."),
    (17000, 19500, "Somewhere a bell began to ring."),
    (20200, 21700, "He listened, and did not move."),
    (25000, 28000, "The ferry left without its captain."),
    (29500, 31000, "Rain came in from the west."),
    (34000, 36500, "They counted the boats twice."),
    (37300, 38800, "One of them was missing."),
    (42000, 45000, "By noon the fog had lifted again."),
    (46500, 48000, "Then the letter was opened."),
]  # fmt: skip

# A line that is in the file and said nowhere, and a line that is said and that the engine did not hear.
EXTRA = "Subtitles by an unknown hand."
UNHEARD_RANK = 8

Fault = Callable[[int, int], int]


def constant(milliseconds: int) -> Fault:
    return lambda _rank, _start: milliseconds


def drift(total: int) -> Fault:
    """A delay that grows from zero to `total` over the length of the file."""
    return lambda rank, _start: round(total * rank / (len(SPOKEN) - 1))


def partial(milliseconds: int) -> Fault:
    """The delay touches only the second half of the file."""
    return lambda rank, _start: milliseconds if rank >= len(SPOKEN) // 2 else 0


# The case's name and the fault laid on the lines; the last two also differ in what is heard and what is given.
CASES: list[tuple[str, Fault, str]] = [
    ("offset-plus-2400", constant(2400), "all"),
    ("offset-minus-900", constant(-900), "all"),
    ("drift", drift(3000), "all"),
    ("partial", partial(2000), "all"),
    ("extra-line", constant(1800), "extra"),
    ("unheard-line", constant(1800), "unheard"),
]


def stamp(milliseconds: int) -> str:
    value = max(milliseconds, 0)
    return f"{value // 3600000:02d}:{value // 60000 % 60:02d}:{value // 1000 % 60:02d},{value % 1000:03d}"


def given_and_truth(fault: Fault, kind: str) -> tuple[str, str]:
    """The file the person has, and where each of its lines is really said."""
    lines: list[tuple[int, int, str, str]] = []
    for rank, (start, end, text) in enumerate(SPOKEN):
        delay = fault(rank, start)
        lines.append((start + delay, end + delay, text, f"{start} {end}"))
        if kind == "extra" and rank == 4:
            # Said nowhere: its own place in the file, between two lines that are.
            lines.append((start + delay + 2600, start + delay + 4200, EXTRA, "-"))
    srt = "".join(
        f"{number}\n{stamp(start)} --> {stamp(end)}\n{text}\n\n"
        for number, (start, end, text, _truth) in enumerate(lines, 1)
    )
    truth = "".join(f"{truth_}\n" for _s, _e, _t, truth_ in lines)
    return srt, truth


def heard(kind: str) -> str:
    """What an engine would have written for the audio: the words of the true lines, at the true times."""
    segments = []
    for rank, (start, end, text) in enumerate(SPOKEN):
        if kind == "unheard" and rank == UNHEARD_RANK:
            continue
        words = text.split()
        step = (end - start) / len(words)
        tokens = [token(" " + word, round(start + i * step), round(start + (i + 1) * step - 40))
                  for i, word in enumerate(words)]
        segments.append(
            {
                "timestamps": {"from": stamp(start), "to": stamp(end)},
                "offsets": {"from": start, "to": end},
                "text": " " + text,
                "tokens": [token("[_BEG_]", start, start), *tokens],
            }
        )
    document = {
        "systeminfo": "WHISPER : (written by alignment-cases.py)",
        "model": {"type": "tiny", "multilingual": True, "vocab": 51865},
        "params": {"model": "ggml-tiny.bin", "language": "auto", "translate": False},
        "result": {"language": "en"},
        "transcription": segments,
    }
    return json.dumps(document, indent=1) + "\n"


def token(text: str, start: int, end: int) -> dict:
    return {
        "text": text,
        "timestamps": {"from": stamp(start), "to": stamp(end)},
        "offsets": {"from": start, "to": end},
        "id": 0,
        "p": 0.9,
        "t_dtw": -1,
    }


def expected() -> dict[Path, str]:
    wanted: dict[Path, str] = {}
    for name, fault, kind in CASES:
        srt, truth = given_and_truth(fault, kind)
        wanted[DIRECTORY / name / "given.srt"] = srt
        wanted[DIRECTORY / name / "truth.txt"] = truth
        wanted[DIRECTORY / name / "heard.json"] = heard(kind)
    return wanted


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
