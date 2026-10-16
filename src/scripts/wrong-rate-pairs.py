#!/usr/bin/env python3
"""The conversions made at the wrong frame rate: a file on a grid, converted by a
wrong ratio, and the truth of what sets it right.

The search of phase 14 (D11, #386) tries the fifty-six ordered pairs of the eight
standard frame rates and lets the deduction judge. Testing it requires knowing what was
laid down: a fixture drawn from what the search finds would only prove that it agrees
with itself. This script **lays down** the wrong conversion, in exact arithmetic, and
writes alongside the ratio that undoes it.

    --check       (default) rebuild each case and compare it with the disk
    --generate    rewrite the cases from the table

Each case holds two files: `fichier.srt` and `verite.txt`, which carries the ratio to
apply to the positions to put them back on the grid (`6/5`), `aucun` ("none") when
nothing fits — a clean file, one without a grid, or one converted by a ratio outside the
set —, or `egalite` ("tie") when two ratios fit equally well.

**The positions are arithmetic**, never a call to our commands: converting with
`ConvertFrameRateCommand` would prove subedit against subedit. Each position is
`round(n × 1000 / R)` for a frame `n`, then `round(position × f)` for the wrong ratio
`f` — the two roundings a real converted file goes through.
"""

from __future__ import annotations

import sys
from fractions import Fraction
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DIRECTORY = REPO_ROOT / "src" / "test" / "data" / "conversions-fausses"

LENGTH_MS = 600_000
SHORT_MS = 10_000


class Steps:
    """The congruential generator of `subtitle-fixtures.py`: an irregular step."""

    def __init__(self, seed: int) -> None:
        self._state = seed

    def between(self, low: int, high: int) -> int:
        self._state = (1103515245 * self._state + 12345) % (2**31)
        return low + self._state % (high - low + 1)


def rounded(value: Fraction) -> int:
    """To the nearest, half up."""
    return (value + Fraction(1, 2)).__floor__()


def frame_starts(rate: Fraction, span_ms: int, seed: int) -> list[int]:
    # A short span is dense, like `grille-24-courte`: ten positions at least, without which
    # the deduction returns no verdict.
    low, high = (150, 400) if span_ms <= SHORT_MS else (2000, 5000)
    steps = Steps(seed)
    starts: list[int] = []
    moment = Fraction(steps.between(low, high), 1000)
    while moment * 1000 < span_ms:
        frame = rounded(moment * rate)
        starts.append(rounded(Fraction(frame * 1000) / rate))
        moment += Fraction(steps.between(low, high), 1000)
    return starts


def stamp(milliseconds: int) -> str:
    return (
        f"{milliseconds // 3600000:02d}:{milliseconds // 60000 % 60:02d}:"
        f"{milliseconds // 1000 % 60:02d},{milliseconds % 1000:03d}"
    )


def srt(starts: list[int]) -> str:
    return "\n".join(
        f"{rank}\n{stamp(start)} --> {stamp(start + 1500)}\nLine {rank}.\n"
        for rank, start in enumerate(starts, 1)
    )


FPS_24 = Fraction(24)
FPS_25 = Fraction(25)
FPS_30 = Fraction(30)
FPS_23976 = Fraction(24000, 1001)

# The case's name, the file's grid, the wrong ratio laid on it (or `None`: none), the span,
# and the truth: the ratio that sets it right, `aucun` or `egalite`.
# A wrong conversion rarely has a single repair: a grid at 28.8 frames per second goes back
# equally well onto 24 and onto 30 with a ratio of the set, and a file whose times are all
# doubled stays on the 25 grid — the deduction tells neither apart —, so the search then
# proposes nothing. The cases with a single truth are those where a file already at the target
# rate was converted — the most common gesture —, and a second one that enumerating the 320
# combinations (grid × ratio) finds alone of its kind.
CASES: list[tuple[str, Fraction, Fraction | None, int, str]] = [
    ("propre-24", FPS_24, None, LENGTH_MS, "aucun"),
    ("faux-25-vers-24", FPS_24, Fraction(25, 24), LENGTH_MS, "24/25"),
    ("faux-25-vers-60", Fraction(60), Fraction(5, 12), LENGTH_MS, "12/5"),
    ("egalite-24-vers-25", FPS_25, Fraction(24, 25), LENGTH_MS, "egalite"),
    ("egalite-50-vers-30", FPS_30, Fraction(5, 3), LENGTH_MS, "egalite"),
    ("egalite-28-8", FPS_24, Fraction(5, 6), LENGTH_MS, "egalite"),
    ("egalite-23976", FPS_25, Fraction(1001, 1000), LENGTH_MS, "egalite"),
    ("egalite-courte", FPS_24, Fraction(25, 24), SHORT_MS, "egalite"),
    ("hors-ensemble", FPS_24, Fraction(21, 20), LENGTH_MS, "aucun"),
    ("sans-grille", Fraction(263, 10), None, LENGTH_MS, "aucun"),
]


def build(rate: Fraction, wrong: Fraction | None, span_ms: int, seed: int) -> str:
    starts = frame_starts(rate, span_ms, seed)
    if wrong is not None:
        starts = [rounded(start * wrong) for start in starts]
    return srt(starts)


def expected() -> dict[Path, str]:
    files: dict[Path, str] = {}
    for seed, (name, rate, wrong, span_ms, truth) in enumerate(CASES, 1):
        files[DIRECTORY / name / "fichier.srt"] = build(rate, wrong, span_ms, seed)
        files[DIRECTORY / name / "verite.txt"] = truth + "\n"
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
