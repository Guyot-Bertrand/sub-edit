#!/usr/bin/env python3
"""Counts, on a subtitle corpus, what Gaupol's four duration constraints violate
and where they contradict each other — issue #371.

`adjust_durations` carries four: minimum duration, maximum duration, minimum
gap between subtitles, reading speed in characters per second. The roadmap
calls them "potentially contradictory"; *potentially* is the word from before
the measurement, and this is the measurement.

Two tables, and the second is the one that existed nowhere.

**What is violated as is** — one count per constraint, on the file's positions
as they are written.

**What cannot be satisfied together** — the contradictions, which depend on no
algorithm because they bear on the room available: only the end moves in
Gaupol, so a subtitle's room is what separates its start from the next one's,
minus the required gap.

    room = next.start − start − gap

    minimum against gap       room < minimum
    speed against gap         room < length / speed
    speed against maximum     length / speed > maximum

None of the three is settled by an order of resolution: they are the subtitles
for which **every answer violates one**.

    ./src/scripts/measure-duration-constraints.py [directory] [options]

**It enters no gate, and nothing depends on it**: it is an instrument, not a
check. The corpus that makes sense here is `src/data/`, private to each machine
and ignored by git — hence no record in `docs/mesures/`: a figure that cannot
be replayed is a testimony, not a measurement. On a machine without a corpus,
it says so and stops.

**What its parsing is worth, and how to check it again.** The positions are read
by a walk written here rather than by the project's reader — as
`measure-mentions.py` does, and for the same reason: an instrument run by hand
must not require a built tree. The price is that it can be wrong, and a first
version of this walk was wrong — it read ",050" as five hundred milliseconds.
`--against` therefore compares the walk with the project's reader, file by
file:

    ./src/scripts/measure-duration-constraints.py --against ./build/dev/bin/subedit-cli

**What it does not do**, and deliberately: apply the adjustment. The predicted
table is worth something because it does not reimplement `adjustDurations`: it
counts what the **definitions** say, on the written positions, and does not know
in which order the core resolves. If it did, it would stop being a witness
independent of the core's.

## The cross-check with `subedit-cli adjust` — issue #560, which holds the deferral of #407

The core states what it sacrifices (`counts.sacrificed.{speed,minimum,gap}`);
this script **predicts** the same three counts from the definitions. If they
differ, one of the two is wrong. The correspondence, established by rereading
`adjustDurations` and this walk:

    room = next.start − start − gap                 (integer milliseconds)

    sacrificed.minimum  ↔  room < minimum           (a next one exists)
                           or maximum < minimum     (the core counts it here)
    sacrificed.speed    ↔  room < need   OR   need > maximum
                           — the UNION of the two pairs, never their sum: a
                           subtitle in both counts once
    sacrificed.gap      ↔  room < 0

    need = length × 1000 / speed, ROUNDED to the millisecond

- **The need is rounded, as the core rounds it** (`readingTimeOf`): without
  that, "Hi" at 15 characters per second needs 133.33 ms and a room of 133 ms
  would count as too small here, enough there. It is the first divergence the
  fixture showed, and it was this script that was wrong.
- **"Next" is the next one in the file**, on both sides: the core does not sort
  on reading. A subtitle with no next one has neither room nor gap to keep.
- **The room does not depend on the written duration.** A subtitle whose end
  precedes its start ("out of scope" for the durations *as is*) still counts for
  what it is: the core treats it like the others.
- **The maximum** is set on both sides (`--maximum 6`): the core does not turn
  it on by default and the count does not name it — it is always held — but it
  decides the speed count. The counts coincide if the maximum is off or at least
  equal to the minimum; otherwise the core counts a minimum that the script sees
  too, and the correspondence above says so.
- **One reservation, on a case no sound file produces**: if the minimum and the
  speed are off and an end precedes its start *and* the room is negative, the
  core does not count the gap, because the end was not raised. With the minimum
  active — the default — the end is always raised to the start.

Two checks, which are the two halves of the proof:

    # the script, against the counts written by hand (no binary required)
    ./src/scripts/measure-duration-constraints.py --check-fixtures

    # the core, against the script, on a corpus
    ./src/scripts/measure-duration-constraints.py [directory] --crosscheck ./build/dev/bin/subedit-cli

The first runs in `make check-local`; the core, for its part, is held to the
same expectations by an end-to-end case (`CLI-ADJUST-06`). The second is an
observation: it says **globally** whether the two counts agree on a corpus, and
a private corpus is never named.
"""

import argparse
import json
import math
import os
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_CORPUS = REPO_ROOT / "src" / "data"

# The two formats this walk reads. The seven others need the project's reader
# — and for MicroDVD, a frame rate: a file counted in frames has no duration
# until someone has said at what rate. The files left aside are counted and
# reported.
EXTENSIONS = {".srt", ".vtt"}

# A timestamp, with or without its hours: WebVTT omits them under one hour.
STAMP = r"(?:(\d{1,3}):)?(\d{1,2}):(\d{2})[,.](\d{1,3})"
CUE = re.compile(STAMP + r"\s*-->\s*" + STAMP)

# What is not visible text. Both forms, because a real corpus mixes the
# vocabularies and an Advanced SSA brace ends up copied into a SubRip more
# often than one would think.
TAG = re.compile(r"</?[a-zA-Z][^>]*>|\{[^}]*\}")

# The defaults are those of `gaupol/config.py`. The maximum is **inactive** by
# default there, and it is set here anyway: a column turned off says nothing,
# and knowing how many subtitles would exceed six seconds is information even
# when nobody uses it.
DEFAULT_MINIMUM = 1.5
DEFAULT_MAXIMUM = 6.0
DEFAULT_GAP = 0.0
DEFAULT_SPEED = 15.0

BOLD = "\033[1m"
RESET = "\033[0m"


def decoded(raw: bytes) -> str:
    """Returns the file's text, whatever its encoding — a real corpus mixes
    several, and refusing to read some would skew the proportions."""
    for encoding in ("utf-8-sig", "cp1252"):
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError:
            continue
    return raw.decode("latin-1", "replace")


def milliseconds(hours: str, minutes: str, seconds: str, fraction: str) -> int:
    """A timestamp in milliseconds.

    The fraction is written on one to three digits: ".5" is five hundred
    milliseconds, not five. Padding it on the right is the only right reading."""
    thousandths = int(fraction.ljust(3, "0"))
    return ((int(hours or 0) * 60 + int(minutes)) * 60 + int(seconds)) * 1000 + thousandths


def cues_of(path: Path) -> list[tuple[int, int, int]]:
    """The file's cues: start, end, and length of the visible text.

    A deliberately rough parse — a timestamp line, and everything that follows
    up to the blank line is text. It holds for SubRip as for WebVTT, and the
    point is to count durations, not to rewrite a reader that exists. It is the
    choice `measure-mentions.py` made before this one.

    **The length counts the line break**, because `get_text_length` counts it:
    Gaupol removes the tags and takes the `len` of what remains."""
    content = decoded(path.read_bytes()).replace("\r\n", "\n").replace("\r", "\n")
    lines = content.split("\n")

    cues: list[tuple[int, int, int]] = []
    index = 0
    while index < len(lines):
        found = CUE.search(lines[index])
        if found is None:
            index += 1
            continue

        fields = found.groups()
        start = milliseconds(*fields[0:4])
        end = milliseconds(*fields[4:8])

        index += 1
        body: list[str] = []
        while index < len(lines) and lines[index].strip() and CUE.search(lines[index]) is None:
            body.append(lines[index])
            index += 1

        cues.append((start, end, len(TAG.sub("", "\n".join(body)))))
    return cues


class Tally:
    """What a walk counts, and what adds up from one file to the next."""

    FIELDS = (
        "subtitles",
        "too_short",
        "too_long",
        "gap_too_small",
        "too_fast",
        "minimum_against_gap",
        "speed_against_gap",
        "speed_against_maximum",
        "contradicted",
        "not_ordered",
        "sacrificed_speed",
        "sacrificed_minimum",
        "sacrificed_gap",
    )

    def __init__(self) -> None:
        for field in self.FIELDS:
            setattr(self, field, 0)

    def add(self, other: "Tally") -> None:
        for field in self.FIELDS:
            setattr(self, field, getattr(self, field) + getattr(other, field))


def rounded(value: float) -> int:
    """To the nearest, halves away from zero: the core's `llround`.

    Python's `round()` rounds halves to even, which is not the same function and
    has no reason to be that of a cross-checking instrument."""
    return int(math.floor(value + 0.5)) if value >= 0 else -int(math.floor(-value + 0.5))


def milliseconds_of(seconds: float) -> int | None:
    """A bound in seconds, in milliseconds — nothing when it is zero, this
    script's convention for "do not apply it"."""
    return rounded(seconds * 1000) if seconds else None


def needed_of(length: int, speed: float) -> int | None:
    """The duration `length` characters need, to the millisecond.

    **Rounded as the core rounds it** (`readingTimeOf`): comparing it in floats
    would call "too short" a room of 133 ms for a need of 133.33 ms that the core
    takes as 133."""
    if not speed or not length:
        return None
    return rounded(length * 1000 / speed)


def census(cues: list[tuple[int, int, int]], limits: argparse.Namespace) -> Tally:
    """The census of one file.

    A constraint set to zero does not apply — that is this script's convention,
    and it is also what Gaupol does by accident for the two durations: its
    `minimum and …` is false when the minimum is zero.

    Everything is in integer milliseconds: those of the core, and a count that
    depends on a floating-point rounding does not compare with its own."""
    tally = Tally()
    minimum = milliseconds_of(limits.minimum)
    maximum = milliseconds_of(limits.maximum)
    gap = rounded(limits.gap * 1000)

    for position, (start, end, length) in enumerate(cues):
        tally.subtitles += 1

        needed = needed_of(length, limits.speed)
        following = cues[position + 1][0] if position + 1 < len(cues) else None
        room = following - start - gap if following is not None else None

        # **What the adjustment would sacrifice**, from the definitions alone. The
        # room does not depend on the written duration: these three counts are
        # made before sorting out what is "out of scope" for the durations as is.
        if minimum is not None and ((room is not None and room < minimum)
                                    or (maximum is not None and maximum < minimum)):
            tally.sacrificed_minimum += 1
        if needed is not None and ((room is not None and room < needed)
                                   or (maximum is not None and needed > maximum)):
            tally.sacrificed_speed += 1
        if room is not None and room < 0:
            tally.sacrificed_gap += 1

        duration = end - start
        if duration <= 0:
            # An end before its start has no duration to compare, and counting a
            # negative duration as "too short" would drown the real figure.
            tally.not_ordered += 1
            continue

        if minimum is not None and duration < minimum:
            tally.too_short += 1
        if maximum is not None and duration > maximum:
            tally.too_long += 1
        if needed is not None and duration < needed:
            tally.too_fast += 1

        contradicted = False
        if needed is not None and maximum is not None and needed > maximum:
            tally.speed_against_maximum += 1
            contradicted = True

        if following is not None:
            if following - end < gap:
                tally.gap_too_small += 1
            if minimum is not None and room < minimum:
                tally.minimum_against_gap += 1
                contradicted = True
            if needed is not None and room < needed:
                tally.speed_against_gap += 1
                contradicted = True

        if contradicted:
            tally.contradicted += 1

    return tally


def timestamp(total: int) -> str:
    """A number of milliseconds, as `inspect` writes it."""
    hours, rest = divmod(total, 3600000)
    minutes, rest = divmod(rest, 60000)
    seconds, thousandths = divmod(rest, 1000)
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}.{thousandths:03d}"


def confront(files: list[Path], binary: str) -> int:
    """Compares this script's walk with the project's reader.

    Three numbers per file, and they are those `inspect` already writes: how
    many subtitles, where the first starts, where the last ends. They do not
    prove that each position is right; they prove that the split into cues and
    the reading of a timestamp are, which is exactly where a hand-written walk
    goes wrong."""
    span = re.compile(r"span:\s*(\S+) -> (\S+)")
    count = re.compile(r"subtitles:\s*(\d+)")

    diverging = 0
    for path in files:
        cues = cues_of(path)
        if not cues:
            continue
        # In English, whatever the machine's locale: the words of `inspect` read below follow the
        # language now that French is installed (#668); `LANGUAGE=en` names the source first.
        told = subprocess.run([binary, "inspect", str(path)],
                              capture_output=True, text=True, check=False,
                              env={**os.environ, "LANGUAGE": "en"}).stdout
        counted = count.search(told)
        bounds = span.search(told)
        if counted is None or bounds is None:
            print(f"  the reader said nothing about {path}", file=sys.stderr)
            diverging += 1
            continue

        mine = (len(cues), timestamp(cues[0][0]), timestamp(max(end for _, end, _ in cues)))
        theirs = (int(counted.group(1)), bounds.group(1), bounds.group(2))
        if mine != theirs:
            print(f"  {path}\n    here   {mine}\n    reader {theirs}", file=sys.stderr)
            diverging += 1

    if diverging:
        print(f"{diverging} file(s) read differently from the project's reader",
              file=sys.stderr)
    else:
        print(f"{len(files)} files, read as the project's reader reads them")
    return diverging


def sacrificed_of(tally: Tally) -> tuple[int, int, int]:
    """The three counts `adjust` declares, in the order of its object."""
    return (tally.sacrificed_speed, tally.sacrificed_minimum, tally.sacrificed_gap)


FIXTURES = REPO_ROOT / "src" / "test" / "data" / "durees"
EXPECTED = REPO_ROOT / "src" / "test" / "data" / "attendus" / "json"


def limits_of(constraints: dict) -> argparse.Namespace:
    """The settings an expected line declares, as this script reads them.

    **The expectation carries its own settings** (`constraints`), so that a line
    cannot be compared with settings other than those it was written for. A
    constraint turned off (`null`) is a bound at zero here."""
    speed = constraints["speed"]
    return argparse.Namespace(
        speed=float(speed["cps"]) if speed else 0.0,
        minimum=(constraints["minimum_ms"] or 0) / 1000.0,
        maximum=(constraints["maximum_ms"] or 0) / 1000.0,
        gap=(constraints["gap_ms"] or 0) / 1000.0)


def check_fixtures() -> int:
    """Compares this script with the counts **written by hand** for the fixture.

    It is the half of the proof that requires no binary: the expectations of
    `src/test/data/attendus/json/recoupement-*.jsonl` are computed on paper from
    the definitions, and the same end-to-end test compares them with the core.
    If both tools agree with them, they agree with each other."""
    expectations = sorted(EXPECTED.glob("recoupement-*.jsonl"))
    if not expectations:
        print(f"no cross-check expectation under {EXPECTED}", file=sys.stderr)
        return 1

    wrong = 0
    checked = 0
    for expectation in expectations:
        for number, line in enumerate(expectation.read_text(encoding="utf-8").splitlines(), 1):
            record = json.loads(line)
            path = FIXTURES / Path(record["file"]).name
            cues = cues_of(path)
            tally = census(cues, limits_of(record["constraints"]))
            counts = record["counts"]
            expected = (counts["sacrificed"]["speed"], counts["sacrificed"]["minimum"],
                        counts["sacrificed"]["gap"])
            checked += 1
            if sacrificed_of(tally) != expected or tally.subtitles != counts["subtitles"]:
                wrong += 1
                print(f"  {expectation.name}:{number} {path.name}\n"
                      f"    expected by hand {expected}\n"
                      f"    predicted here   {sacrificed_of(tally)}", file=sys.stderr)

    if wrong:
        print(f"{wrong} count(s) out of {checked} differ from those written by hand",
              file=sys.stderr)
        return 1
    print(f"{checked} fixture files, predicted as the counts written by hand")
    return 0


def arguments_of(limits: argparse.Namespace) -> list[str]:
    """This script's settings, told to `subedit-cli adjust`.

    Zero turns a bound off here, and `off` does the same there; the maximum has
    no `off`: not giving it turns it off."""
    flags = ["--speed", f"{limits.speed:g}" if limits.speed else "off",
             "--minimum", f"{limits.minimum:g}" if limits.minimum else "off",
             "--gap", f"{limits.gap:g}"]
    if limits.maximum:
        flags += ["--maximum", f"{limits.maximum:g}"]
    return flags


def crosscheck(files: list[Path], binary: str, limits: argparse.Namespace) -> int:
    """Compares the three predicted counts with those the core declares.

    One file at a time, `--dry-run`: nothing is written. **The result is stated
    globally** — how many files agree, how many diverge, and the sum of each
    count on both sides — because a private corpus is not named. The
    divergences are detailed file by file on the error output, for whoever looks
    for which of the two is wrong: it is an instrument, and its output is not a
    document."""
    agree = 0
    diverge: list[tuple[Path, tuple[int, int, int], tuple[int, int, int]]] = []
    refused = 0
    predicted_total = [0, 0, 0]
    core_total = [0, 0, 0]

    for path in files:
        cues = cues_of(path)
        if not cues:
            continue
        mine = sacrificed_of(census(cues, limits))
        run = subprocess.run([binary, "--format", "json", "adjust", "--dry-run",
                              *arguments_of(limits), str(path)],
                             capture_output=True, text=True, check=False)
        try:
            record = json.loads(run.stdout.splitlines()[0])
        except (IndexError, json.JSONDecodeError):
            record = {"ok": False}
        if not record.get("ok"):
            refused += 1
            continue

        sacrificed = record["counts"]["sacrificed"]
        theirs = (sacrificed["speed"], sacrificed["minimum"], sacrificed["gap"])
        for rank in range(3):
            predicted_total[rank] += mine[rank]
            core_total[rank] += theirs[rank]
        if mine == theirs:
            agree += 1
        else:
            diverge.append((path, mine, theirs))

    names = ("speed", "minimum", "gap")
    print(f"{agree + len(diverge)} files compared"
          f" — {agree} agree, {len(diverge)} diverge, {refused} refused by the core")
    print("  sum of the counts      " + "  ".join(f"{name:>8}" for name in names))
    print("  predicted by script    " + "  ".join(f"{count:8d}" for count in predicted_total))
    print("  declared by the core   " + "  ".join(f"{count:8d}" for count in core_total))

    for path, mine, theirs in diverge:
        print(f"  {path}\n    script {mine}\n    core   {theirs}", file=sys.stderr)
    return 1 if diverge else 0


def share(part: int, whole: int) -> str:
    return f"{100 * part / whole:.1f} %" if whole else "—"


def report(total: Tally, per_file: list[tuple[str, Tally]], limits: argparse.Namespace) -> None:
    def line(label: str, count: int, whole: int) -> None:
        print(f"    {label:<34}{count:8d}   {share(count, whole):>7}")

    counted = total.subtitles - total.not_ordered

    print()
    print(f"{BOLD}violated as is{RESET}   — over {counted} subtitles whose duration compares")
    line("shorter than the minimum", total.too_short, counted)
    line("longer than the maximum", total.too_long, counted)
    line("gap to the next one too small", total.gap_too_small, counted)
    line("slower than the speed", total.too_fast, counted)

    print()
    print(f"{BOLD}contradictory{RESET}   — no end satisfies both")
    line("minimum against gap", total.minimum_against_gap, counted)
    line("speed against gap", total.speed_against_gap, counted)
    line("speed against maximum", total.speed_against_maximum, counted)
    line("at least one of the three", total.contradicted, counted)

    print()
    print(f"{BOLD}sacrificed (predicted){RESET}   — what `adjust` would declare,"
          " from the definitions")
    line("speed (the union of the two pairs)", total.sacrificed_speed, total.subtitles)
    line("minimum", total.sacrificed_minimum, total.subtitles)
    line("gap (negative room)", total.sacrificed_gap, total.subtitles)

    if total.not_ordered:
        print()
        print(f"{BOLD}out of scope{RESET}")
        line("end before or on the start", total.not_ordered, total.subtitles)

    print()
    print("A contradiction is not an edge case: it is a subtitle for which every end")
    print("violates at least one constraint. What the adjustment sacrifices, it counts")
    print("itself; `--crosscheck` compares its three counts with those predicted above")
    print("(see the header of this script).")

    print()
    print("  contradicted   subtitles   file")
    for name, tally in sorted(per_file, key=lambda pair: -pair[1].contradicted):
        counted_here = tally.subtitles - tally.not_ordered
        if tally.contradicted:
            print(f"  {share(tally.contradicted, counted_here):>12}"
                  f"   {tally.subtitles:9d}   {name}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("corpus", nargs="?", default=str(DEFAULT_CORPUS),
                        help="directory to walk (default: src/data)")
    parser.add_argument("--minimum", type=float, default=DEFAULT_MINIMUM,
                        help="minimum duration in seconds, zero not to apply it")
    parser.add_argument("--maximum", type=float, default=DEFAULT_MAXIMUM,
                        help="maximum duration in seconds, zero not to apply it")
    parser.add_argument("--gap", type=float, default=DEFAULT_GAP,
                        help="minimum gap between two subtitles, in seconds")
    parser.add_argument("--speed", type=float, default=DEFAULT_SPEED,
                        help="reading speed in characters per second, zero not to apply it")
    parser.add_argument("--against", metavar="SUBEDIT_CLI",
                        help="compares this script's walk with the project's reader, "
                             "and measures nothing")
    parser.add_argument("--check-fixtures", action="store_true",
                        help="compares this script with the counts written by hand for the "
                             "versioned fixture, and measures nothing")
    parser.add_argument("--crosscheck", metavar="SUBEDIT_CLI",
                        help="compares the three predicted counts with those `subedit-cli "
                             "adjust --dry-run` declares, file by file")
    limits = parser.parse_args()

    if limits.check_fixtures:
        return check_fixtures()

    root = Path(limits.corpus)
    if not root.is_dir():
        print(f"corpus not found: {root}", file=sys.stderr)
        return 1

    read = sorted(path for path in root.rglob("*")
                  if path.is_file() and path.suffix.lower() in EXTENSIONS)
    skipped = sorted(path for path in root.rglob("*")
                     if path.is_file() and path.suffix.lower() not in EXTENSIONS)

    if not read:
        print(f"no SubRip or WebVTT file under {root}", file=sys.stderr)
        return 1

    if limits.against:
        return 1 if confront(read, limits.against) else 0

    if limits.crosscheck:
        return crosscheck(read, limits.crosscheck, limits)

    total = Tally()
    per_file: list[tuple[str, Tally]] = []
    for path in read:
        cues = cues_of(path)
        if not cues:
            continue
        tally = census(cues, limits)
        total.add(tally)
        per_file.append((str(path.relative_to(root)), tally))

    print(f"{BOLD}{len(per_file)} files, {total.subtitles} subtitles{RESET}"
          f" — {len(skipped)} file(s) left aside, which this walk does not read")
    print(f"minimum {limits.minimum} s · maximum {limits.maximum} s"
          f" · gap {limits.gap} s · speed {limits.speed} char/s")
    print("the defaults are Gaupol's, which however leaves the maximum inactive")

    report(total, per_file, limits)
    return 0


if __name__ == "__main__":
    sys.exit(main())
