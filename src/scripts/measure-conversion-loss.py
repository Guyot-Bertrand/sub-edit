#!/usr/bin/env python3
"""What a conversion loses, measured rather than believed.

[ADR 0009](../../docs/adr/0009-texte-en-chaine-brute.md) has announced since
phase 1 that a structured model serves as a pivot "only during a conversion
between formats, where the loss is unavoidable and **accepted**". *Accepted*
assumes it is known, and nothing said it.

    --binary PATH     the subedit-cli to measure (default: build/dev/bin/subedit-cli)
    --journal         compares the measurement with the record in docs/mesures/conversion.md
    --record          rewrites that record from the measurement; implies --journal
    --diff            shows, for each altered file, what does not come back

## What is measured, and why it is a round trip

A conversion cannot be judged from the outside: the resulting file is in
another format, and there is nothing to compare it with. **The starting format
is therefore the yardstick.** A file leaves as `A`, goes through `B`, comes back
as `A`, and what does not come back is what `B` could not carry.

That is exact, and it is so only thanks to a property the harness of #338 holds
elsewhere: **a corpus file is already what our writer produces**, byte for
byte. Without it, a gap between departure and arrival could come from the
layout rather than from the trip, and the measurement would no longer say
anything.

## The number, and what it is worth

    intact round trips: N/M

**It knowingly mixes two things** — what a format *cannot* carry, and what our
conversion loses without being forced to. Separating them would mean declaring
the seventy-two cells by hand, and a declaration written by hand goes stale in
silence: that is the defect `check-installation.sh` names for the manual, and
that #289 fixed for the corpus.

What the number guarantees is narrower and stands on its own: **a loss does not
get worse without anyone seeing it.** What it is worth in principle is read
beside it, in prose, in the record — and that is where the degradation policy
is decided.

## What is not read

**The private corpus.** It is absent from any machine that does not have it, so
a gate that read it would say two things depending on the workstation. It does
carry two formats of the phase; what to do with them is the business of #340.
"""

from __future__ import annotations

import argparse
import datetime
import difflib
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
JOURNAL = REPO_ROOT / "docs" / "mesures" / "conversion.md"
DEFAULT_BINARY = REPO_ROOT / "build" / "dev" / "bin" / "subedit-cli"

# The directories walked, in the order the record names them. `formats/`
# carries the same scene in the nine formats — it is what makes the matrix
# comparable from one format to the next; `valides/` carries files that do not
# resemble each other, and it is what finds today what the scene does not show:
# SubRip coordinates, a WebVTT header, cell settings.
CORPUS = ("valides", "formats")

BLOCK_OPEN = "<!-- generated record: do not edit by hand -->"
BLOCK_CLOSE = "<!-- end of record -->"
RECORDED = re.compile(r"^\s*intact round trips\s*:\s*(\d+)\s*/\s*(\d+)\s*$", re.MULTILINE)

# The name `inspect` prints, and the value `--to` expects. Both come from the
# binary; what does not is their correspondence, and that is why a name unknown
# on either side **stops** instead of being ignored. A format shipped without
# its line here would be half measured, in silence.
TARGET_OF = {
    "SubRip": "srt",
    "WebVTT": "vtt",
    "SubViewer 2": "subviewer2",
    "Sub Station Alpha": "ssa",
    "Advanced SSA": "ass",
    "MPL2": "mpl2",
    "MicroDVD": "microdvd",
    "TMPlayer": "tmplayer",
    "LRC": "lrc",
}

GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def fail(message):
    print(f"{RED}✗{RESET} {message}", file=sys.stderr)
    sys.exit(1)


def run(binary, arguments):
    """Runs the binary without going through a shell.

    **Never a shell**, and #290 says why: paths can carry spaces and brackets,
    and a recomposed command line splits them.
    """
    # In English, whatever the machine's locale: `format_of` reads the words of `inspect`, which
    # follow the language now that French is installed (#668), and `LANGUAGE=en` names the source
    # language first (spec 15, D4).
    return subprocess.run([str(binary), *arguments], capture_output=True, text=True, check=False,
                          env={**os.environ, "LANGUAGE": "en"})


def targets(binary):
    """The formats `--to` accepts, read from the binary's help.

    **Derived rather than written.** A list of formats copied here would stay at
    two the day the binary accepted nine, and the measurement would be silently
    partial.
    """
    help_text = run(binary, ["convert", "--help"]).stdout
    found = re.search(r"--to TEXT:\{([^}]*)\}", help_text)
    if found is None:
        fail(f"'--to' not found in the help of {binary}")
    return [name.strip() for name in found.group(1).split(",") if name.strip()]


def format_of(binary, path):
    """The format `inspect` recognises, or nothing when it does not open."""
    done = run(binary, ["--quiet", "inspect", str(path)])
    if done.returncode != 0:
        return None
    # **`(.+?)` and not `(\S+)`**: "SubViewer 2" carries a space, and the
    # original pattern could not read it. It then returned "no format", hence a
    # file counted among those that do not open — a silent false negative,
    # found the day the first format with a compound name arrived.
    found = re.search(r"^\s*format:\s*(.+?)\s*$", done.stdout, re.MULTILINE)
    return found.group(1) if found else None


def corpus_files():
    found = []
    for directory in CORPUS:
        base = REPO_ROOT / "src" / "test" / "data" / directory
        for path in sorted(base.iterdir()):
            if path.is_file() and path.suffix != ".md":
                found.append(path)
    return found


# What a pair returns when the conversion refuses to make it.
#
# **A refusal is not a loss**, and counting it as one would say the opposite of
# what it is. `convert --to microdvd` refuses a timed file with no grid and no
# `--frame-rate`: the only other way out would be to invent a rate and move
# every cue. The pair cannot be measured; it is not lost.
REFUSED = object()


def round_trip(binary, path, home, via, work):
    """Sends the file to `via`, brings it back to `home`, and returns what comes back.

    Returns `REFUSED` when either conversion refuses.
    """
    outward = work / f"outward.{via}"
    back = work / f"back.{home}"
    for target, source, output in ((via, path, outward), (home, outward, back)):
        done = run(binary, ["--quiet", "convert", "--to", target,
                            "--output", str(output), str(source)])
        if done.returncode != 0:
            return REFUSED
    return back.read_bytes()


def measure(binary, known):
    """Each readable file, sent into each of the other formats."""
    trips = []
    unreadable = 0
    refused = []

    with tempfile.TemporaryDirectory() as scratch:
        work = Path(scratch)
        for path in corpus_files():
            name = format_of(binary, path)
            if name is None:
                unreadable += 1
                continue
            if name not in TARGET_OF:
                fail(f"'{name}' is read by the binary and missing from TARGET_OF:"
                     " complete the table before measuring")
            home = TARGET_OF[name]
            if home not in known:
                fail(f"'{home}' is not a value of --to: the table and the binary"
                     " no longer say the same thing")

            original = path.read_bytes()
            for via in known:
                if via == home:
                    continue
                came_back = round_trip(binary, path, home, via, work)
                if came_back is REFUSED:
                    refused.append((path, via))
                    continue
                trips.append((path, name, via, came_back == original,
                              None if came_back == original else came_back))

    return trips, unreadable, refused


def relative(path):
    return path.relative_to(REPO_ROOT / "src" / "test" / "data").as_posix()


def first_difference(path, came_back):
    """The first line of the original that does not come back, as it is written.

    **A line, not a count.** A number of lines that move does not read; the line
    itself tells whoever looks at it what was lost, in the vocabulary of the
    starting format.
    """
    before = path.read_bytes().decode("utf-8", "replace").splitlines()
    after = came_back.decode("utf-8", "replace").splitlines()
    for line in difflib.unified_diff(before, after, n=0, lineterm=""):
        if line.startswith("-") and not line.startswith("---"):
            return line[1:]
    # The original loses nothing: what moved is an addition, an invented end
    # for instance. The first added line is returned, so as not to stay silent.
    for line in difflib.unified_diff(before, after, n=0, lineterm=""):
        if line.startswith("+") and not line.startswith("+++"):
            return line[1:]
    return ""


def show_diff(path, came_back):
    before = path.read_bytes().decode("utf-8", "replace").splitlines()
    after = came_back.decode("utf-8", "replace").splitlines()
    for line in difflib.unified_diff(before, after, fromfile=relative(path),
                                     tofile=f"{relative(path)} (back)", lineterm=""):
        print(f"    {line}")


def matrix(trips):
    """The ordered pairs, counted: the matrix the phase will fill."""
    cells = {}
    for _, name, via, intact, _ in trips:
        home = TARGET_OF[name]
        kept, total = cells.get((home, via), (0, 0))
        cells[(home, via)] = (kept + int(intact), total + 1)
    return cells


def report(trips, unreadable, refused):
    kept = sum(1 for *_, intact, _ in trips if intact)
    total = len(trips)
    print(f"    intact round trips: {kept}/{total}")
    if refused:
        print(f"    {len(refused)} pair(s) the conversion refuses to make")
    if unreadable:
        print(f"    {unreadable} file(s) of the corpus do not open yet")
    return kept, total


def journal_text():
    if not JOURNAL.is_file():
        fail(f"journal not found: {JOURNAL}")
    return JOURNAL.read_text(encoding="utf-8")


def recorded_count(text):
    """The count the journal shows, or the script stops.

    **A missing or duplicated anchor stops**, as for the detection score: a
    journal rewritten by hand on which nothing can be read any more would make
    the comparison silent, hence green.
    """
    found = RECORDED.findall(text)
    if len(found) != 1:
        fail(f"'intact round trips: N/M' appears {len(found)} times in"
             f" {JOURNAL.relative_to(REPO_ROOT)}, exactly one expected")
    return int(found[0][0]), int(found[0][1])


def version_of():
    for line in (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "VERSION":
            return parts[1]
    fail("unreadable version in CMakeLists.txt")
    return ""


def rewrite_journal(trips, unreadable, refused, known, kept, total):
    """Rewrites the journal's one generated block, and leaves the prose intact."""
    text = journal_text()
    opened = text.find(BLOCK_OPEN)
    closed = text.find(BLOCK_CLOSE)
    if opened < 0 or closed < opened:
        fail(f"record markers not found in {JOURNAL.relative_to(REPO_ROOT)}")

    body = [f"{BLOCK_OPEN}\n\n", f"    intact round trips: {kept}/{total}\n\n",
            f"Recorded on version {version_of()}, on"
            f" {datetime.date.today().isoformat()}.\n\n"]

    cells = matrix(trips)
    names = sorted({TARGET_OF[name] for _, name, *_ in trips} | set(known))
    body.append("| From \\ Through | " + " | ".join(f"`{name}`" for name in names) + " |\n")
    body.append("| :--- | " + " | ".join("---:" for _ in names) + " |\n")
    for home in names:
        row = [f"| `{home}` "]
        for via in names:
            if home == via:
                row.append("| — ")
            elif (home, via) in cells:
                right, over = cells[(home, via)]
                row.append(f"| {right}/{over} ")
            else:
                row.append("| · ")
        body.append("".join(row) + "|\n")
    body.append("\n")

    lost = [(path, name, via, came_back)
            for path, name, via, intact, came_back in trips if not intact]
    if lost:
        body.append("| File | Through | First line that does not come back |\n")
        body.append("| :--- | :------ | :---------------------------------- |\n")
        for path, _, via, came_back in lost:
            line = first_difference(path, came_back).replace("|", "\\|")
            body.append(f"| `{relative(path)}` | `{via}` | `{line}` |\n")
    else:
        body.append("No round trip altered.\n")
    body.append("\n")

    if refused:
        body.append(f"**{len(refused)} pair(s) the conversion refuses to make** — a timed"
                    " file to a frame-based format, with no grid and no frame rate given."
                    " A refusal is not a loss, and so does not enter the count"
                    " above.\n\n")
    if unreadable:
        body.append(f"{unreadable} file(s) of the corpus do not open yet and"
                    " enter no measurement.\n\n")

    written = text[:opened] + "".join(body) + text[closed:]
    if written == text:
        print(f"{GREEN}✓{RESET} record already up to date: nothing to re-record")
        return 0
    JOURNAL.write_text(written, encoding="utf-8")
    print(f"{GREEN}✓{RESET} record written: {kept}/{total}")
    return 0


def compare_to_journal(trips, unreadable, refused, known, kept, total, record):
    """The three situations, separately — the mechanics of `make score`."""
    if record:
        return rewrite_journal(trips, unreadable, refused, known, kept, total)

    was, over = recorded_count(journal_text())
    command = "make conversion-record"

    # Cross products rather than raw counts: a file added to the corpus changes
    # the denominator, and two counts no longer compare.
    if kept * over < was * total:
        print(f"{RED}✗{RESET} the conversion loses more: {kept}/{total} intact,"
              f" against {was}/{over} in the record", file=sys.stderr)
        for path, _, via, intact, came_back in trips:
            if not intact:
                print(f"    {relative(path)} through {via}:"
                      f" {first_difference(path, came_back)}", file=sys.stderr)
        print(f"\n  compare with {JOURNAL.relative_to(REPO_ROOT)} to see what moved.",
              file=sys.stderr)
        print(f"  fix the conversion, or re-record deliberately:\n    {command}",
              file=sys.stderr)
        return 1

    if (kept, total) != (was, over):
        print(f"{GREEN}✓{RESET} the conversion loses less: {kept}/{total} intact,"
              f" against {was}/{over} in the record")
        print(f"  record it:\n    {command}")
        return 0

    print(f"{GREEN}✓{RESET} {kept}/{total} intact round trips, as in the record")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--binary", default=str(DEFAULT_BINARY),
                        help="the subedit-cli to measure")
    parser.add_argument("--journal", action="store_true",
                        help="compares the measurement with the record")
    parser.add_argument("--record", action="store_true",
                        help="rewrites the record from the measurement; implies --journal")
    parser.add_argument("--diff", action="store_true",
                        help="shows what does not come back, file by file")
    options = parser.parse_args()

    binary = Path(options.binary)
    if not binary.is_file() or not os.access(binary, os.X_OK):
        fail(f"binary not found or not executable: {binary}\n"
             "  build it: cmake --build --preset dev --target subedit-cli")

    known = targets(binary)
    trips, unreadable, refused = measure(binary, known)
    if not trips:
        fail("no round trip to measure: the corpus or the binary returns nothing")

    kept, total = report(trips, unreadable, refused)

    if options.diff:
        for path, _, via, intact, came_back in trips:
            if not intact:
                print(f"\n  {relative(path)}, through {via}:")
                show_diff(path, came_back)

    if options.journal or options.record:
        return compare_to_journal(trips, unreadable, refused, known, kept, total,
                                  options.record)
    return 0


if __name__ == "__main__":
    sys.exit(main())
