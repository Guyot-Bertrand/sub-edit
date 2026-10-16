#!/usr/bin/env python3
"""Scores an encoding detection, instead of believing it.

**A detection is a classifier, not a function.** It has a success rate, not a
right or wrong result, and "the detection works" without a figure is exactly
the kind of claim that three reviews in a row proved wrong — the icon that three
readers out of four accepted, the `.rpm` flawless under inspection and refused
at installation, the shortcuts measured on a platform no user has.

**The detector is an argument, and that is the whole point.** The scoping of
phase 8 (#288) has three paths — ICU, uchardet, hand-written tables — and the
choice between them is a choice of rate. Measuring it after choosing is choosing
without measuring. Each path therefore goes through the same corpus, without
this script having to know any of them:

    ./src/scripts/score-encoding-detection.py --detector 'uchardet {}'
    ./src/scripts/score-encoding-detection.py --detector 'file -b --mime-encoding {}'
    ./src/scripts/score-encoding-detection.py --detector './build/dev/bin/probe {}' --private

`{}` is replaced by the file's path; what the command writes on its standard
output is read as the name of the encoding.

## The two corpora, and what each can say

**The labelled corpus** — `src/test/data/encodages/`, made by
`encoding-fixtures.py`. The label comes with the file, for free: the table says
which encoding it was written in. It is the only one of the two that gives a
**rate**, and it also gives the detail of the confusions — confusing two Latin
encodings does not cost what confusing a Latin one and a Cyrillic one costs.

**The private corpus** — `src/data/`, which carries real files, and badly
encoded real files are precisely what a made-up corpus does not contain. **It
has no labels**, so it gives no rate: that would be inventing a truth. What it
gives is what can be checked without one — the distribution of the answers, how
many really decode the file from end to end, and where two detectors contradict
each other.

**It cannot name a file**, and that is not politeness: the repository's rule
forbids quoting the private corpus, and a tool that *can* do it ends up doing
it. Nothing here carries a path to the output.

## Where the detection stops knowing — `--by-length`

The labelled corpus gives **one** rate, over a dozen files. It does not say
*where* the case becomes hard, and issue #310 was born of that: the table needed
is "length → rate", and it can be made.

`--by-length` generates graded SubRip files, hands them to the same detector,
and returns two tables. Nothing is committed to the repository: the corpus is
disposable, it lives as long as the measurement. The seed is fixed, so the table
replays identically. The cues it draws from, accented text in each encoding,
are test data and live in `src/test/data/encodages/cliff-pools.json`.

## The journal, and its divergence — `--journal`

A score recorded once grows old without anyone learning it: that is the defect
of issue #311. `--journal` compares what has just been measured with what
`docs/mesures/detection-d-encodage.md` shows, and **treats the three situations
separately** — a score that drops fails, a score that rises invites a
re-record, an unchanged score stays quiet. `--record` rewrites the record.

**It is not a threshold.** The number compared is that of the last measurement,
not a bar someone set: a detection is a classifier, and setting a bar on a
measurement whose variance is unknown is what this project refuses elsewhere.

## What is not measured here

Time. A detection that takes ten milliseconds per file is not a problem in a
subtitle editor; the bench is there for the day the question arises.
"""

from __future__ import annotations

import argparse
import codecs
import collections
import datetime
import importlib.util
import json
import random
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LABELLED = REPO_ROOT / "src" / "test" / "data" / "encodages"
POOLS_FILE = LABELLED / "cliff-pools.json"
PRIVATE = REPO_ROOT / "src" / "data"
JOURNAL = REPO_ROOT / "docs" / "mesures" / "detection-d-encodage.md"

# The two markers of the block `--record` rewrites; the rest of the journal is
# prose written by hand, and it survives a re-record.
BLOCK_OPEN = "<!-- generated record: do not edit by hand -->"
BLOCK_CLOSE = "<!-- end of record -->"

# The anchor `--journal` reads back in that block.
RECORDED = re.compile(r"^\s*labelled corpus\s*:\s*(\d+)\s*/\s*(\d+)\s*$", re.MULTILINE)

GREEN = "\033[32m"
RED = "\033[31m"
BOLD = "\033[1m"
RESET = "\033[0m"

# What the detectors write, and `codecs` cannot read back as is.
ALIASES = {
    "utf-16": "utf-16",
    "utf-16le": "utf-16-le",
    "utf-16be": "utf-16-be",
    "us-ascii": "ascii",
    "unknown-8bit": "?",
    "binary": "?",
    "": "?",
    "none": "?",
}


def canonical(name):
    """The name Python gives this encoding, or `?` when it knows none.

    The detectors do not write the same words for the same encoding —
    `WINDOWS-1252`, `cp1252`, `windows-1252`. The comparison therefore goes
    through the canonical name rather than the string returned.
    """
    cleaned = name.strip().lower()
    if cleaned in ALIASES:
        return ALIASES[cleaned]
    try:
        return codecs.lookup(cleaned).name
    except LookupError:
        return cleaned or "?"


def labelled_fixtures():
    """The fixtures and their encoding, read from the table that makes them."""
    spec = importlib.util.spec_from_file_location(
        "encoding_fixtures", REPO_ROOT / "src" / "scripts" / "encoding-fixtures.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return [(LABELLED / name, encoding, bom) for name, encoding, bom, *_ in module.FIXTURES]


# The shape of an encoding name, and nothing else. Letters, digits, hyphens,
# dots and underscores, a few dozen characters at most.
ENCODING_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,31}$")


def detect(command, path):
    """What the detector says of this file, or `?` when it says nothing.

    **No shell, and the answer is filtered.** Both guards exist for the same
    reason, and they were paid for: the first version passed the command to the
    shell with the path substituted in plain text, and the names of the private
    corpus carry spaces and brackets. `file` therefore received split
    arguments, complained about each one — and **its complaints, which quoted
    the names, were read as encodings and printed.**

    That is exactly what #290 asked to make impossible: "the tool must not be
    able to name a file, not merely refrain from it".

    The path is therefore an element of `argv` and not a piece of a command
    line, and **what comes out is refused unless it has the shape of an
    encoding name** — a path never has that shape. The error output is never
    read.
    """
    try:
        parts = shlex.split(command)
    except ValueError:
        return "?"

    argv = [str(path) if part == "{}" else part for part in parts]

    try:
        done = subprocess.run(argv, capture_output=True, text=True, timeout=30)
    except (OSError, subprocess.SubprocessError):
        return "?"

    if done.returncode != 0:
        return "?"

    said = done.stdout.strip().splitlines()[0].strip() if done.stdout.strip() else ""
    return canonical(said) if ENCODING_NAME.match(said) else "?"


def agrees(said, expected, has_bom):
    """Is the detector right?

    **`utf-16` without its byte order is a good answer when the file has a
    BOM.** The BOM carries the byte order, and a detector that returns `UTF-16`
    is not wrong: it left to the BOM what belongs to it.
    """
    if said == expected:
        return True
    return has_bom and said == "utf-16" and expected in ("utf-16-le", "utf-16-be")


def decodes(path, encoding):
    """Do these bytes decode entirely in this encoding?

    It is the only thing that can be checked without a label — and it is
    necessary without being sufficient: a single-byte encoding decodes almost
    anything, which is precisely where mojibake comes from.
    """
    if encoding == "?":
        return False
    try:
        path.read_bytes().decode(encoding)
        return True
    except (UnicodeDecodeError, LookupError):
        return False


def score_labelled(command):
    fixtures = labelled_fixtures()
    right = 0
    confusions = []

    print(f"{BOLD}labelled corpus — {len(fixtures)} fixtures{RESET}")
    for path, expected, has_bom in fixtures:
        if not path.is_file():
            print(f"  {RED}✗{RESET} {path.name} — missing;"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            continue
        said = detect(command, path)
        wanted = canonical(expected)
        if agrees(said, wanted, has_bom):
            right += 1
            print(f"  {GREEN}✓{RESET} {path.name:<22} {said}")
        else:
            confusions.append((path.name, wanted, said))
            print(f"  {RED}✗{RESET} {path.name:<22} {said}  instead of {wanted}")

    total = len(fixtures)
    part = f"{100 * right / total:.0f} %" if total else "—"
    print(f"\n  {right}/{total} — {part}")
    if confusions:
        print("\n  confusions:")
        for name, wanted, said in confusions:
            print(f"    {wanted} read as {said}   ({name})")
    return right, total, confusions


# The extensions under which a subtitle file presents itself.
#
# **There used to be two, and this walk made the corpus say what it does not
# say** — issue #340. The initialisation of phase 9 wrote that the private
# corpus "carries only .srt and .vtt", on the strength of this record; it could
# find nothing else. It also carries a MicroDVD and an Advanced SSA, real ones.
# That is the defect of #268, checking with the tool that does not count, once
# more.
#
# `.txt` is not among them, and its absence is a choice: it names TMPlayer and
# MPL2, and everything else. The only `.txt` files of the private corpus are
# notes and manual excerpts.
SUBTITLE_SUFFIXES = (".srt", ".vtt", ".sub", ".ass", ".ssa", ".lrc")


def report_private(commands):
    """What the private corpus can say, and it cannot say a rate."""
    files = sorted(p for p in PRIVATE.rglob("*")
                   if p.is_file() and p.suffix.lower() in SUBTITLE_SUFFIXES)
    if not files:
        print(f"\n{BOLD}private corpus — absent from this machine{RESET}")
        return

    print(f"\n{BOLD}private corpus — {len(files)} files, without labels{RESET}")
    print("  no rate: there is no truth to compare with, only what can be")
    print("  checked without one.")

    for command in commands:
        answers = collections.Counter()
        readable = 0
        for path in files:
            said = detect(command, path)
            answers[said] += 1
            if decodes(path, said):
                readable += 1
        print(f"\n  {BOLD}{command}{RESET}")
        for name, count in answers.most_common():
            print(f"    {count:3d}  {name}")
        print(f"    {readable}/{len(files)} answers under which the file"
              " decodes entirely")

    if len(commands) > 1:
        disagreements = 0
        for path in files:
            said = {detect(command, path) for command in commands}
            if len(said) > 1:
                disagreements += 1
        print(f"\n  the detectors contradict each other on {disagreements}"
              f" file(s) out of {len(files)}")


# ---------------------------------------------------------------------------
# The cliff: where the detection stops knowing — issue #310.
# ---------------------------------------------------------------------------

def load_pools():
    """The cues the cliff draws from, by encoding: the same as the fixtures', and
    a few more — a rate table needs draws, where a fixture is just one file.

    **They are test data, and they live with it**: accented text whose language
    is the point, in `src/test/data/encodages/cliff-pools.json`. The order of the
    keys is the order of the table's rows.
    """
    return json.loads(POOLS_FILE.read_text(encoding="utf-8"))


# What fills a file around the accented cues: English, which carries no high
# byte. The second table is the one that needs it.
FILLER = [
    "I don't know what you mean.",
    "The harbour was empty at that hour.",
    "Where did he go?",
    "It didn't last very long.",
    "She said the same thing again, quietly.",
    "Do you think they will come back?",
    "They told me it was settled.",
    "The hotel had been closed since February.",
]

# How many draws per table row. Sixty gives a rate to within a percentage
# point without the measurement lasting more than a few seconds.
DRAWS = 60

# The seed, written rather than drawn: a table that cannot be replayed
# identically is a testimony, not a measurement.
SEED = 310


def script_of(character):
    """The script of a character, roughly — what tells one alphabet from another
    for a reader, not for Unicode.

    The codes `0x80–0x9f` have their own family, and it is not a subtlety: that
    is where the Latin-1 / CP1252 confusion becomes visible. A typographic
    apostrophe read as Latin-1 comes back as a control character, which is a
    real loss where the rest of that confusion is none.
    """
    code = ord(character)
    if code < 0x80:
        return "ascii"
    if code < 0xA0:
        return "control"
    if code < 0x250 or 0x1E00 <= code < 0x1F00:
        return "latin"
    if 0x370 <= code < 0x400:
        return "greek"
    if 0x400 <= code < 0x530:
        return "cyrillic"
    if code >= 0x2E80:
        return "ideographs"
    return "other"


def scripts_of(text):
    return [script_of(character) for character in text if ord(character) >= 0x80]


def subrip(lines):
    """A SubRip file, in the form the project writes."""
    parts = []
    for index, text in enumerate(lines, start=1):
        start = index * 2
        parts.append(f"{index}\n"
                     f"00:{start // 60:02d}:{start % 60:02d},000"
                     f" --> 00:{start // 60:02d}:{(start + 1) % 60:02d},000\n"
                     f"{text}\n\n")
    return "".join(parts)


def measure(command, directory, pools, encoding, subtitles, accented):
    """The rate of a detector on files of `subtitles` cues, of which `accented`
    carry accented text and the rest English.

    Three rates, and they do not say the same thing. **Exact** — the answer
    names the encoding written in. **Text** — the file read back under the
    answer is the file written, which forgives a confusion without consequence.
    **Script** — the text read back keeps the alphabet of the text written,
    which forgives only mojibake within the same family.
    """
    # The seed is a string and not a tuple: the `hash` of a string varies from
    # one process to the next, and the table would stop replaying identically.
    draw = random.Random(f"{SEED}-{encoding}-{subtitles}-{accented}")
    pool = pools[encoding]
    exact = same_text = same_script = 0
    sizes = []
    highs = []
    answers = collections.Counter()

    for round_number in range(DRAWS):
        lines = [draw.choice(FILLER) for _ in range(subtitles)]
        for where in draw.sample(range(subtitles), accented):
            lines[where] = draw.choice(pool)
        text = subrip(lines)
        data = text.encode(encoding)
        sizes.append(len(data))
        highs.append(sum(1 for byte in data if byte >= 0x80))

        path = directory / f"{encoding}-{subtitles}-{accented}-{round_number}.srt"
        path.write_bytes(data)
        said = detect(command, path)
        path.unlink()
        answers[said] += 1

        if said == canonical(encoding):
            exact += 1
        try:
            read_back = data.decode(said)
        except (UnicodeDecodeError, LookupError):
            continue
        if read_back == text:
            same_text += 1
        if scripts_of(read_back) == scripts_of(text):
            same_script += 1

    return {
        "bytes": sum(sizes) // DRAWS,
        "high": sum(highs) // DRAWS,
        "exact": exact,
        "text": same_text,
        "script": same_script,
        "answers": answers,
    }


# The two tables, and they do not attack the problem from the same end.
#
# **By length** — the whole file is accented, and it grows. It is the table #310
# asks for, and it shows the cliff: a lone cue carries only three high bytes.
#
# **With rare text** — the file is long and almost all ASCII, and only a few
# cues carry accents. It is the other end, and the one that forbids setting a
# threshold on length: a forty-kilobyte file may give nothing to weigh.
BY_LENGTH = [1, 2, 3, 5, 8, 13, 21]
WHEN_RARE = [(40, 1), (40, 3), (200, 1), (200, 3), (200, 10), (600, 1), (600, 10)]


def print_table(command, directory, pools, title, rows):
    print(f"{BOLD}{title}{RESET}")
    print(f"  {'written in':<12} {'cues':>9} {'accented':>10} {'bytes':>7}"
          f" {'≠ASCII':>7} {'exact':>7} {'text':>7} {'script':>9}   answers")
    for encoding, subtitles, accented in rows:
        found = measure(command, directory, pools, encoding, subtitles, accented)
        top = ", ".join(f"{count}×{name}"
                        for name, count in found["answers"].most_common(3))
        print(f"  {encoding:<12} {subtitles:>9} {accented:>10} {found['bytes']:>7}"
              f" {found['high']:>7}"
              f" {100 * found['exact'] / DRAWS:>6.0f}%"
              f" {100 * found['text'] / DRAWS:>6.0f}%"
              f" {100 * found['script'] / DRAWS:>8.0f}%   {top}")
    print()


def report_cliff(command):
    """The two graded tables, on a disposable corpus."""
    pools = load_pools()
    print(f"{BOLD}the cliff — generated corpus, seed {SEED}, {DRAWS} draws"
          f" per row{RESET}\n")
    with tempfile.TemporaryDirectory(prefix="cliff-") as name:
        directory = Path(name)
        print_table(command, directory, pools, "by length — the whole file is accented",
                    [(encoding, count, count)
                     for encoding in pools for count in BY_LENGTH])
        print_table(command, directory, pools,
                    "with rare text — a long file, almost all ASCII",
                    [(encoding, subtitles, accented)
                     for encoding in pools for subtitles, accented in WHEN_RARE])


# ---------------------------------------------------------------------------
# The journal, and its divergence — issue #311.
# ---------------------------------------------------------------------------


def journal_text():
    if not JOURNAL.is_file():
        print(f"{RED}✗{RESET} journal not found: {JOURNAL}", file=sys.stderr)
        sys.exit(1)
    return JOURNAL.read_text(encoding="utf-8")


def recorded_score(text):
    """The score the journal shows, or the script stops.

    **A missing or duplicated anchor stops**, rather than letting it through:
    that is what `check-coverage.sh` learnt to do with its own ratchet. A
    journal rewritten by hand on which nothing can be read any more would make
    the comparison silent, hence green.
    """
    found = RECORDED.findall(text)
    if len(found) != 1:
        print(f"{RED}✗{RESET} 'labelled corpus: N/M' appears {len(found)} times in"
              f" {JOURNAL.relative_to(REPO_ROOT)}, exactly one expected", file=sys.stderr)
        print("  fix the file by hand before running again.", file=sys.stderr)
        sys.exit(1)
    return int(found[0][0]), int(found[0][1])


def version_of():
    """The current number, read where it is authoritative."""
    for line in (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "VERSION":
            return parts[1]
    print(f"{RED}✗{RESET} unreadable version in CMakeLists.txt", file=sys.stderr)
    sys.exit(1)


def rewrite_journal(right, total, confusions):
    """Rewrites the journal's one generated block, and leaves the prose intact."""
    text = journal_text()
    opened = text.find(BLOCK_OPEN)
    closed = text.find(BLOCK_CLOSE)
    if opened < 0 or closed < opened:
        print(f"{RED}✗{RESET} record markers not found in"
              f" {JOURNAL.relative_to(REPO_ROOT)}", file=sys.stderr)
        sys.exit(1)

    body = [f"{BLOCK_OPEN}\n\n", f"    labelled corpus: {right}/{total}\n\n",
            f"Recorded on version {version_of()}, on"
            f" {datetime.date.today().isoformat()}.\n\n"]
    if confusions:
        body.append("| Fixture | Written in | Read as |\n| :------ | :--------- | :------ |\n")
        for name, wanted, said in confusions:
            body.append(f"| `{name}` | `{wanted}` | `{said}` |\n")
    else:
        body.append("No confusion.\n")
    body.append("\n")

    written = text[:opened] + "".join(body) + text[closed:]
    if written == text:
        print(f"{GREEN}✓{RESET} record already up to date: nothing to re-record")
        return
    JOURNAL.write_text(written, encoding="utf-8")
    print(f"{GREEN}✓{RESET} record written: {right}/{total}")


def compare_to_journal(right, total, confusions, record):
    """The three situations, treated separately.

    The number compared is that of the last measurement and not a bar someone
    set — which is what makes the coverage ratchet a ratchet and not a
    threshold. The comparison goes through cross products rather than counts:
    adding a fixture changes the denominator, and two raw counts no longer
    compare.
    """
    if record:
        rewrite_journal(right, total, confusions)
        return 0

    kept, over = recorded_score(journal_text())
    # The `make` target rather than the full line: it builds the detector before
    # running it, which a bare invocation of this script does not.
    command = "make score-record"

    if right * over < kept * total:
        print(f"{RED}✗{RESET} the detection went back: {right}/{total} recognised,"
              f" against {kept}/{over} in the record", file=sys.stderr)
        if confusions:
            print("\n  current confusions:", file=sys.stderr)
            for name, wanted, said in confusions:
                print(f"    {wanted} read as {said}   ({name})", file=sys.stderr)
        print(f"\n  compare with {JOURNAL.relative_to(REPO_ROOT)} to see what moved.",
              file=sys.stderr)
        print(f"  fix the detection, or re-record deliberately:\n    {command}",
              file=sys.stderr)
        return 1

    if (right, total) != (kept, over):
        print(f"{GREEN}✓{RESET} the detection improves: {right}/{total} recognised,"
              f" against {kept}/{over} in the record")
        print(f"  record it:\n    {command}")
        return 0

    print(f"{GREEN}✓{RESET} {right}/{total} recognised, as in the record")
    return 0


def main():
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument("--detector", action="append", required=True,
                        help="detection command, '{}' for the path; repeatable")
    parser.add_argument("--private", action="store_true",
                        help="adds the report on the private corpus, in aggregates")
    parser.add_argument("--by-length", action="store_true",
                        help="generates a graded corpus and returns the 'length → rate' table")
    parser.add_argument("--journal", action="store_true",
                        help="compares the score with the record in docs/mesures/")
    parser.add_argument("--record", action="store_true",
                        help="rewrites that record from the measurement; implies --journal")
    options = parser.parse_args()

    # **A single detector when the journal is at stake.** The record carries the
    # score of the project's detection; comparing two with a single figure would
    # mean nothing.
    if (options.journal or options.record) and len(options.detector) != 1:
        parser.error("--journal and --record take a single --detector")

    right = total = 0
    confusions = []
    for command in options.detector:
        print(f"{BOLD}detector: {command}{RESET}\n")
        got, seen, missed = score_labelled(command)
        right, total = right + got, total + seen
        confusions += missed
        print()

    # **An empty corpus is not a score of zero, it is an absence of
    # measurement**, and making it green would be the worst defect of this script.
    if total == 0:
        print(f"{RED}✗{RESET} the labelled corpus is empty", file=sys.stderr)
        return 1

    if options.by_length:
        for command in options.detector:
            report_cliff(command)

    # The private corpus is never read by a gate: it is absent from any machine
    # that does not have it, and `--private` remains a manual gesture.
    if options.private:
        report_private(options.detector)

    if options.journal or options.record:
        return compare_to_journal(right, total, confusions, options.record)

    return 0


if __name__ == "__main__":
    sys.exit(main())
