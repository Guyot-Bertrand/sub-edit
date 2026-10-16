#!/usr/bin/env python3
"""The subtitle fixtures on a known grid: make them, and check that they are what
the project says they are.

Phase 16 deduces a file's frame rate from its positions, by measuring whether
they fall on a grid. Testing it requires span and volume: the richest fixture of
the repository carried six timestamps, and two timestamps say nothing of a grid.
It takes files of several hundred cues, and a file of several hundred cues is
unreadable in a diff.

This script is what makes them **checkable rather than believed**: the table of
what each fixture is lives here, making follows from it, and `--check` rebuilds
each file to hold it against the one on disk.

    --check       (default) rebuild each fixture and compare it with the disk
    --generate    rewrite the fixtures from the table
    --measure     print each fixture's concentration on the eight candidates
    --weight      print the total weight, in bytes

**The positions are arithmetic, never a call to our commands.** An "already
shifted" file is `round(n × 1000 / R) + offset`; an "already transformed" file is
an affine map applied to the milliseconds. Going through `ShiftCommand` or the
frame rate conversion would prove subedit against subedit: a rounding error
shared by the operation and the deduction would pass green. What is not proven
this way — that *Gaupol's* shift preserves the grid — is a claim about Gaupol,
which nothing here needs.

**This script requires no tool.** Unlike `video-fixtures.sh`, which needs ffmpeg
and ffprobe, everything here is exact arithmetic: the rebuild is reproducible
everywhere, and `--check` can therefore compare byte by byte rather than ask a
third party.
"""

from __future__ import annotations

import math
import re
import sys
from collections.abc import Callable
from fractions import Fraction
from functools import partial
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
FIXTURE_DIR = REPO_ROOT / "src" / "test" / "data" / "grilles"

# The total weight allowed. It is generous — about twice the real weight — and its
# role is to catch an order of magnitude, not a byte: a set of fixtures that grows
# unseen is a silent debt.
WEIGHT_CEILING = 270000

# The eight standard frame rates, in the order `StandardFrameRate` enumerates
# them. The three NTSC ones are an integer rate divided by 1.001 and have no exact
# floating-point representation: that is why everything here goes through
# rationals.
STANDARD_RATES = {
    "23-976": Fraction(24000, 1001),
    "24": Fraction(24),
    "25": Fraction(25),
    "29-97": Fraction(30000, 1001),
    "30": Fraction(30),
    "50": Fraction(50),
    "59-94": Fraction(60000, 1001),
    "60": Fraction(60),
}

# Ten minutes, and the reason fits in one division. The 23.976 and 24 grids drift
# apart by one millisecond per second of film, and a frame lasts 41.7: the phase
# on the wrong candidate therefore makes **a full turn every forty-two seconds**.
# Ten minutes is fourteen turns, and the wrong candidate collapses beyond doubt.
#
# It is also what gives the short fixture its meaning, the other way round: ten
# seconds is a quarter of a turn, and the two candidates stay indistinguishable.
# The deduction must say so rather than decide.
FULL_EXTENT_SECONDS = 600
SHORT_EXTENT_SECONDS = 10

# The reading speed rule that computes a fixture's ends, and these are Gaupol's
# two defaults — `gaupol/config.py`, fifteen characters per second and a floor of
# a second and a half. They are set here because a fixture must say where its
# numbers come from; it does not claim to be the duration adjustment phase 10
# will write, only to produce off-grid ends in a plausible, reproducible way.
READING_SPEED = 15
SHORTEST_SHOWN_MS = 1500


class Lcg:
    """A linear congruential generator, written here rather than borrowed.

    The step between two cues must be **irregular and reproducible**. Without
    irregularity, the phases become periodic and the file at an absurd rate
    would come out concentrated on a candidate it does not touch; without
    reproducibility, `--check` no longer means anything.

    Python's `random` would offer the second guarantee but not the first in the
    long run: its sequence is stable in practice, but nothing promises it from
    one version to the next. Twelve lines written here are worth that promise,
    and they make the sequence reproducible by whoever reimplements the
    generator in another language.
    """

    def __init__(self, seed: int) -> None:
        self._state = seed

    def _next(self) -> int:
        self._state = (1103515245 * self._state + 12345) % (2**31)
        # The low-order bits of a congruential generator are the worst; take
        # the ones above them.
        return self._state >> 8

    def between(self, low: int, high: int) -> int:
        """An integer between `low` and `high`, both included."""
        return low + self._next() % (high - low + 1)


def frames_of(seconds: float, rate: Fraction) -> int:
    """How many frames fit in this duration, at this rate."""
    return int(Fraction(seconds).limit_denominator(1000) * rate)


def milliseconds_of(frame: int, rate: Fraction) -> int:
    """The position of this frame, in whole milliseconds.

    The rounding is to the nearest, half up, and it is written by hand: Python's
    `round` rounds to the even integer, which is not what subtitle software does.
    """
    exact = Fraction(frame * 1000) / rate
    return (2 * exact.numerator + exact.denominator) // (2 * exact.denominator)


def timestamp_of(milliseconds: int) -> str:
    """The SubRip form of a position: `01:23:45,678`."""
    hours, rest = divmod(milliseconds, 3600000)
    minutes, rest = divmod(rest, 60000)
    seconds, rest = divmod(rest, 1000)
    return f"{hours:02d}:{minutes:02d}:{seconds:02d},{rest:03d}"


def grid_cues(
    rate: Fraction,
    extent_seconds: float,
    seed: int,
    gap_seconds: tuple[float, float] = (2.0, 5.0),
    shortest_cue_seconds: float = 1.0,
) -> list[tuple[int, int]]:
    """The cues of a perfectly timed file, in frame numbers.

    A cue lasts from one to a few seconds and leaves a gap before the next: that
    is the density of a dialogue, and it is what gives the file an irregular
    step.

    `gap_seconds` exists for the short fixture, and for it alone. A small span
    and a dialogue of normal density give only eight cues, and eight points say
    nothing: the concentration of any candidate there is already `1/√8`, that
    is thirty-five per cent. Chance is not ambiguity.
    """
    steps = Lcg(seed)
    shortest_gap = frames_of(gap_seconds[0], rate)
    longest_gap = frames_of(gap_seconds[1], rate)
    shortest_cue = frames_of(shortest_cue_seconds, rate)
    breath = frames_of(shortest_cue_seconds / 2, rate)

    cues: list[tuple[int, int]] = []
    frame = frames_of(1.0, rate)
    last = frames_of(extent_seconds, rate)
    while frame <= last:
        gap = steps.between(shortest_gap, longest_gap)
        # The cue stops before the next one: the upper bound is the gap minus a
        # breath, never the gap itself. `max` rather than the bare subtraction:
        # at a tight density, the gap minus the breath can fall below the
        # shortest cue, and an upper bound below the lower bound would make an
        # empty draw.
        cues.append((frame, frame + steps.between(shortest_cue, max(shortest_cue, gap - breath))))
        frame += gap
    return cues


def render(lines: list[tuple[int, int, str]]) -> str:
    """A SubRip file, in UTF-8 with Unix line endings.

    One block per cue, separated by a blank line, and a blank line after the
    last: that is what the fixtures of `valides/` already carry.
    """
    blocks = []
    for index, (start, end, text) in enumerate(lines, start=1):
        blocks.append(f"{index}\n{timestamp_of(start)} --> {timestamp_of(end)}\n{text}\n")
    return "\n".join(blocks) + "\n"


def on_grid(rate: Fraction, extent: float, seed: int, offset: int = 0, **density
            ) -> list[tuple[int, int, str]]:
    """A file whose every position is on the grid, up to `offset`."""
    return [
        (
            milliseconds_of(start, rate) + offset,
            milliseconds_of(end, rate) + offset,
            f"Réplique {index}, image {start}.",
        )
        for index, (start, end) in enumerate(grid_cues(rate, extent, seed, **density), start=1)
    ]


def read_ends(rate: Fraction, extent: float, seed: int) -> list[tuple[int, int, str]]:
    """Starts on the grid, ends computed from a reading speed.

    **It is the shape of a real file, and no other fixture has it.** The
    thirteen others put both bounds on the grid; in a real file only the starts
    are there, because a *cue-out* is computed rather than placed on a frame.
    The phase 16 survey says so of the private corpus — starts at 100 without
    exception, ends from 55 to 100 — and until now it was an observation that
    nothing tracked carried.

    What it tests fits in one sentence: **the starts are enough**. The day the
    deduction started reading the ends, this fixture would fail and the
    thirteen others would stay green.

    The end stops one frame before the next start. Without that bound a
    subtitle could overlap the next one, which no real file does — and the
    fixture would stop being plausible for no gain.
    """
    lines = on_grid(rate, extent, seed)
    frame = milliseconds_of(1, rate)

    computed = []
    for index, (start, _, text) in enumerate(lines):
        # Rounded to the nearest, half up, as everywhere here.
        wanted = max((2000 * len(text) + READING_SPEED) // (2 * READING_SPEED), SHORTEST_SHOWN_MS)
        end = start + wanted
        if index + 1 < len(lines):
            end = min(end, lines[index + 1][0] - frame)
        computed.append((start, end, text))
    return computed


def no_grid(extent: float, seed: int) -> list[tuple[int, int, str]]:
    """Positions written in milliseconds, on no grid at all.

    **`grille-absurde.srt` is not this case**: it is regular on a non-standard
    rate, and it tests "none of the eight". This one tests "no grid", which is
    not the same question — it is the false positive's, and it only makes sense
    at the size of a real file.

    The roadmap claims that "failure is loud" and that a file without a grid
    stays under a few per cent on the eight candidates. It is the claim the
    whole use of the deduction rests on, and it was backed by three files of
    the private corpus that nobody else owns. This fixture makes it checkable.

    The noise floor is `1/√n`: at a hundred and seventy-six positions it is
    seven and a half per cent, and the partial band starts at fifty. The margin
    is comfortable, and it was checked before writing this generator — two
    thousand draws of a hundred and seventy-six arbitrary starts never lift the
    best of the eight above twenty-four.
    """
    steps = Lcg(seed)
    lines: list[tuple[int, int, str]] = []
    position = 1000
    last = int(extent * 1000)
    index = 1
    while position <= last:
        gap = steps.between(2000, 5000)
        lines.append((position,
                      position + steps.between(1000, gap - 500),
                      f"Réplique {index}, sans grille."))
        position += gap
        index += 1
    return lines


def retimed_tail(rate: Fraction, extent: float, seed: int) -> list[tuple[int, int, str]]:
    """Two thirds on the grid, one third retimed — an assembled file.

    The two-point transformation **destroys** the grid: it is the only one of
    Gaupol's three operations the deduction does not recover from. Applied to
    the last third only, it gives the partial case whose deviations are
    *grouped*, the one that looks like a resynchronised section.

    The affine map is written here, in milliseconds, rather than obtained by
    calling `TransformCommand`: that is the rule of this whole file.
    """
    lines = on_grid(rate, extent, seed)
    boundary = 2 * len(lines) // 3

    def moved(value: int) -> int:
        # A slope that is the ratio of no pair of standard rates, and an
        # intercept that is a multiple of no frame duration: nothing the
        # deduction looks for can land on it.
        return (value * 10007 + 1370000) // 10000

    return lines[:boundary] + [
        (moved(start), moved(end), f"Réplique {index}, hors grille.")
        for index, (start, end, _) in enumerate(lines[boundary:], start=boundary + 1)
    ]


def scattered_faults(rate: Fraction, extent: float, seed: int) -> list[tuple[int, int, str]]:
    """One position in five moved by several frames.

    It is the other face of the partial file: not a resynchronised section, but
    editing by hand, cue by cue.

    **The deviation is counted in frames, not in milliseconds.** A first version
    moved by nine milliseconds at most, believing it left the grid: at 25 frames
    per second a frame lasts forty, and nine milliseconds are only a fortieth of
    a turn. The position stayed practically on the grid, and the file came out
    at 92 — above the clean-grid threshold it is precisely meant to miss. A
    human who corrects a position in the table moves it by what they see, and
    its phase becomes arbitrary.
    """
    lines = on_grid(rate, extent, seed)
    frame = milliseconds_of(1, rate)
    drift = Lcg(seed + 1)
    faulty = []
    for index, (start, end, text) in enumerate(lines, start=1):
        if index % 5 != 0:
            faulty.append((start, end, text))
            continue
        nudge = drift.between(frame + 1, 10 * frame - 1)
        # An exact multiple of a frame's duration would land back on the grid,
        # and the deviation would not show.
        nudge += 1 if nudge % frame == 0 else 0
        nudge *= 1 if drift.between(0, 1) else -1
        faulty.append((start + nudge, end + nudge, f"Réplique {index}, hors grille."))
    return faulty


# name | what the fixture is | how it is built
#
# The seed differs from one fixture to the next so that no pair of files carries
# the same dialogue at two rates: two twin fixtures would read as one.
Fixture = Callable[[], list[tuple[int, int, str]]]

FIXTURES: list[tuple[str, str, Fixture]] = [
    *(
        (
            f"grille-{name}.srt",
            f"perfect grid at {name.replace('-', '.')} frames per second, ten minutes",
            partial(on_grid, rate, FULL_EXTENT_SECONDS, seed),
        )
        for seed, (name, rate) in enumerate(STANDARD_RATES.items(), start=1)
    ),
    (
        "grille-absurde.srt",
        "grid at 26.3 frames per second — silent on the eight candidates",
        partial(on_grid, Fraction(263, 10), FULL_EXTENT_SECONDS, 101),
    ),
    (
        "grille-24-decalee.srt",
        "perfect grid at 24, translated by 2,999 ms",
        partial(on_grid, Fraction(24), FULL_EXTENT_SECONDS, 102, offset=2999),
    ),
    (
        "grille-24-courte.srt",
        "perfect grid at 24, ten tight seconds — span too short",
        partial(
            on_grid,
            Fraction(24),
            SHORT_EXTENT_SECONDS,
            103,
            gap_seconds=(0.2, 0.35),
            shortest_cue_seconds=0.1,
        ),
    ),
    (
        "grille-24-fins-calculees.srt",
        "starts at 24, ends computed at 15 characters per second — off the grid",
        partial(read_ends, Fraction(24), FULL_EXTENT_SECONDS, 106),
    ),
    (
        "sans-grille.srt",
        "arbitrary milliseconds — no grid, and the deduction must stay silent",
        partial(no_grid, FULL_EXTENT_SECONDS, 107),
    ),
    (
        "melange-groupe.srt",
        "29.97 on two thirds, last third retimed — grouped deviations",
        partial(retimed_tail, Fraction(30000, 1001), FULL_EXTENT_SECONDS, 104),
    ),
    (
        "melange-disperse.srt",
        "25, one position in five moved — scattered deviations",
        partial(scattered_faults, Fraction(25), FULL_EXTENT_SECONDS, 105),
    ),
]

# --------------------------------------------------------------------------
# The instrument: what these fixtures give, and why they are these ones
# --------------------------------------------------------------------------
#
# `--measure` belongs to no gate and nothing depends on it. It exists so that
# the figures of the LISEZMOI can be **redone by a command** rather than
# reported: without it, they would have the defect held against the surveys of
# the private corpus — a number the reader has to believe.
#
# It reimplements the method phase 16 will write in C++, deliberately. It is not
# a gate and it is not the reference: the reference will be the production code
# and the constants of its tests. It is a second, independent computation, on
# tracked files — and two implementations that agree are worth more than one
# that vouches for itself.

TIMESTAMP = re.compile(r"(\d\d):(\d\d):(\d\d),(\d\d\d) -->")
CUE_OUT = re.compile(r"--> (\d\d):(\d\d):(\d\d),(\d\d\d)")


def starts_of(path: Path) -> list[int]:
    """The starts of a SubRip file, in milliseconds.

    The starts alone: wherever a grid exists, they are at 100 without
    exception, where the ends range from 55 to 100 — a *cue-out* is often
    computed by a reading speed rule, not placed on a frame.
    """
    return [
        ((int(h) * 60 + int(m)) * 60 + int(s)) * 1000 + int(ms)
        for h, m, s, ms in TIMESTAMP.findall(path.read_text(encoding="utf-8"))
    ]


def ends_of(path: Path) -> list[int]:
    """The ends of a SubRip file, in milliseconds.

    They serve no deduction — it reads only the starts — and that is precisely
    what they serve to show: the fixture with computed ends has them off the
    grid, and the thirteen others have them on it."""
    return [
        ((int(h) * 60 + int(m)) * 60 + int(s)) * 1000 + int(ms)
        for h, m, s, ms in CUE_OUT.findall(path.read_text(encoding="utf-8"))
    ]


def concentration(positions: list[int], rate: Fraction) -> float:
    """The concentration of the phases on the grid of `rate`, from 0 to 100.

    Each position becomes an angle — its phase on the grid — and the
    concentration is the length of the mean vector. A hundred: a perfect grid,
    whatever its phase. Near zero: no structure.

    No tolerance parameter, and insensitive to an offset: that is what sets it
    apart from a zero-phase grid search, which scored zero a perfect 24 frames
    per second translated by a millisecond and a half.
    """
    turns = [2 * math.pi * ((position * float(rate) / 1000.0) % 1.0) for position in positions]
    resultant = math.hypot(
        sum(math.cos(turn) for turn in turns), sum(math.sin(turn) for turn in turns)
    )
    return 100.0 * resultant / len(positions)


RED = "\033[31m"
GREEN = "\033[32m"
BOLD = "\033[1m"
RESET = "\033[0m"


def generate() -> int:
    print(f"{BOLD}making the grid fixtures{RESET}")
    FIXTURE_DIR.mkdir(parents=True, exist_ok=True)
    for name, description, fixture in FIXTURES:
        content = render(fixture())
        path = FIXTURE_DIR / name
        path.write_text(content, encoding="utf-8")
        cues = content.count(" --> ")
        # The weight is read on disk and not from `len(content)`: the cue text
        # carries accents, and a character there is not a byte. The table of the
        # LISEZMOI copied the wrong one of the two at least once.
        print(f"  {GREEN}✓{RESET} {name} — {description},"
              f" {cues} cues, {path.stat().st_size} bytes")
    return 0


def check() -> int:
    failures = 0
    for name, description, fixture in FIXTURES:
        path = FIXTURE_DIR / name
        if not path.is_file():
            print(
                f"  {RED}✗{RESET} {name} — missing;"
                " ./src/scripts/subtitle-fixtures.py --generate",
                file=sys.stderr,
            )
            failures += 1
            continue

        expected = render(fixture())
        actual = path.read_text(encoding="utf-8")
        if actual != expected:
            print(
                f"  {RED}✗{RESET} {name} — the file is no longer what the table says;"
                " ./src/scripts/subtitle-fixtures.py --generate",
                file=sys.stderr,
            )
            failures += 1
            continue

        print(f"  {GREEN}✓{RESET} {name} — {description}, {expected.count(' --> ')} cues")

    total = weight()
    if total > WEIGHT_CEILING:
        print(
            f"  {RED}✗{RESET} total weight {total} bytes, maximum {WEIGHT_CEILING}",
            file=sys.stderr,
        )
        failures += 1

    if failures:
        print(f"{RED}{failures} difference(s) between the fixtures and the table.{RESET}", file=sys.stderr)
        return 1

    print(f"  total weight: {total} bytes")
    return 0


def measure() -> int:
    print(f"{BOLD}concentration of each fixture on the eight candidates{RESET}")
    print("  " + " " * 28 + "  ".join(f"{name:>6s}" for name in STANDARD_RATES))
    for name, _, _ in FIXTURES:
        path = FIXTURE_DIR / name
        if not path.is_file():
            print(f"  {RED}✗{RESET} {name} — missing", file=sys.stderr)
            return 1
        positions = starts_of(path)
        scores = [concentration(positions, rate) for rate in STANDARD_RATES.values()]
        print(f"  {name:28s}" + "  ".join(f"{score:6.1f}" for score in scores))

    print()
    print(f"{BOLD}the ends, on the candidate the starts point to{RESET}")
    print(f"  {'':28s}{'candidate':>10s}{'starts':>9s}{'ends':>7s}")
    for name, _, _ in FIXTURES:
        path = FIXTURE_DIR / name
        starts = starts_of(path)
        best, score = max(
            ((label, concentration(starts, rate)) for label, rate in STANDARD_RATES.items()),
            key=lambda pair: pair[1],
        )
        held = concentration(ends_of(path), STANDARD_RATES[best])
        print(f"  {name:28s}{best:>10s}{score:9.1f}{held:7.1f}")
    return 0


def weight() -> int:
    return sum(
        (FIXTURE_DIR / name).stat().st_size
        for name, _, _ in FIXTURES
        if (FIXTURE_DIR / name).is_file()
    )


def main() -> int:
    argument = sys.argv[1] if len(sys.argv) > 1 else "--check"
    if argument == "--check":
        return check()
    if argument == "--generate":
        return generate()
    if argument == "--measure":
        return measure()
    if argument == "--weight":
        print(weight())
        return 0
    print(
        f"usage: {Path(sys.argv[0]).name} [--check|--generate|--measure|--weight]",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
