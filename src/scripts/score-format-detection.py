#!/usr/bin/env python3
"""What a format detection answers, and **where it stops knowing**.

    --detector 'CMD {}'   the detection to measure, repeatable; {} receives the path
    --private             adds the private corpus, from which no rate is drawn
    --journal             compares the measurement with the record in docs/mesures/
    --record              rewrites that record from the measurement; implies --journal

## Why the question arises now

**With two formats it was closed in advance**: a file carries the `WEBVTT`
signature or a comma arrow, and nothing else comes close. With nine, it is a
classification, and the collisions are real:

  .sub          designates MicroDVD **and** SubViewer 2
  .txt          designates TMPlayer, MPL2, and the rest of the world
  {0}{25}       differs from [0][25] only by the shape of the brackets
  SSA and ASS   share their whole header but a version and a section name

This is the reasoning of #290, carried over: **a detection is a classifier, not
a function.** The project's rule — nothing rather than a guess — has to be
checked on nine candidates, not believed.

## Three outcomes, and two of them are not worth the same

A **right** answer. A **wrong** answer, which opens a file as what it is not. A
**refusal**, which says it does not know. The refusal is the project's rule
applied; the confusion is what is wanted at no price. The record therefore
counts them apart, and that is what makes a rate say something here.

## The file name is not part of the question

The fixtures are called `scene.microdvd.sub`: their name **carries the
answer**, since the corpus has to be labelled. It is removed before the
detector sees them — each file is handed over under a neutral name, **keeping
its real extension**, so that `.sub` stays as ambiguous as it is in the wild.

If the scoping ever decides that the extension is admissible evidence, this is
where it will have to be said, and deliberately.
"""

from __future__ import annotations

import argparse
import collections
import datetime
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
CORPUS = REPO_ROOT / "src" / "test" / "data"
PRIVATE = REPO_ROOT / "src" / "data"
JOURNAL = REPO_ROOT / "docs" / "mesures" / "detection-de-format.md"

BLOCK_OPEN = "<!-- generated record: do not edit by hand -->"
BLOCK_CLOSE = "<!-- end of record -->"
RECORDED = re.compile(r"^\s*recognised formats\s*:\s*(\d+)\s*/\s*(\d+)\s*$", re.MULTILINE)

# The name a fixture carries, and the one the product prints. Both live here
# because nothing else brings them together: the first is a piece of a file
# name, the second comes out of `nameOf(SubtitleFormat)`.
#
# **An unknown slug stops the script.** A rendering dropped into `formats/`
# without its line here would be measured against nothing, and the rate would
# stay pretty.
NAMED = {
    "srt": "SubRip",
    "vtt": "WebVTT",
    "subviewer2": "SubViewer 2",
    "ssa": "Sub Station Alpha",
    "ass": "Advanced SSA",
    "microdvd": "MicroDVD",
    "mpl2": "MPL2",
    "tmplayer": "TMPlayer",
    "lrc": "LRC",
}

# The extensions under which a subtitle file presents itself, **`.txt`
# included**.
#
# **It used to be excluded, and the exclusion was written down**: `.txt` names
# MPL2 and TMPlayer and everything else, the only ones the private corpus
# carries are notes, and counting them measured deserved refusals. The argument
# holds for today's files; what it costs is greater than what it avoids.
#
# **The probe could not see an MPL2 or a TMPlayer**, so the journal's sentence —
# "the private corpus carries none" — was guaranteed by the filter rather than
# measured. That is the exact shape of the defect of #336: checking with the
# tool that does not count. And it is a filter **by extension**, in the one
# record whose whole value rests on the extension playing no part in the
# decision.
#
# What removing it costs is five more refusals, and a refusal is not an error:
# it says that we do not know, which is true of a notes file. The journal names
# them for what they are.
SUBTITLE_SUFFIXES = (".srt", ".vtt", ".sub", ".ass", ".ssa", ".lrc", ".txt")

# The shape of a format name, and nothing else will be read as an answer.
# This is the guard of #290, taken over as is: a detector that complains on its
# standard output must not be able to get a path printed.
FORMAT_NAME = re.compile(r"^[A-Za-z][A-Za-z0-9 +]{0,30}$")

BOLD = "\033[1m"
GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def fail(message):
    print(f"{RED}✗{RESET} {message}", file=sys.stderr)
    sys.exit(1)


def label_of(path):
    """The format a corpus file announces, by its name.

    `scene.microdvd.sub` carries its slug between the name and the extension;
    the others have only their extension, which is enough when it designates a
    single format.
    """
    parts = path.name.split(".")
    slug = parts[-2] if len(parts) > 2 else parts[-1]
    if slug not in NAMED:
        fail(f"'{slug}' is not a format the table knows, and {path.name}"
             " carries its name: complete NAMED before measuring")
    return NAMED[slug]


def labelled_files():
    """The labelled corpus: the scene in nine formats, and the valid files."""
    found = []
    for directory in ("formats", "valides"):
        for path in sorted((CORPUS / directory).iterdir()):
            if path.is_file() and path.suffix != ".md":
                found.append((path, label_of(path)))
    return found


# The two answers that are not a format name. **"Undecodable" is not a refusal
# to name**: the file was not read at all, and counting it as a missed
# detection would put on format detection what belongs to encoding detection.
REFUSED = "refused"
UNDECODABLE = "undecodable"


def detect(command, path, work):
    """What the detector says of this file, or why it says nothing.

    **The name is removed, the extension is kept.** The file is copied as
    `subject.<ext>`: the detector therefore no longer has the label the corpus
    carries, and keeps what a real-world file would offer it.

    **No shell, and the answer is filtered** — the two guards of #290, and they
    were paid for: a path handed to the shell was split there, and the
    detector's complaints, which quoted the names, were read as answers.
    """
    subject = work / f"subject{path.suffix}"
    shutil.copyfile(path, subject)

    try:
        parts = shlex.split(command)
    except ValueError:
        return REFUSED

    argv = [str(subject) if part == "{}" else part for part in parts]

    try:
        done = subprocess.run(argv, capture_output=True, text=True, timeout=30, check=False)
    except (OSError, subprocess.SubprocessError):
        return REFUSED

    if done.returncode == 3:
        return UNDECODABLE
    if done.returncode != 0:
        return REFUSED

    lines = done.stdout.strip().splitlines()
    said = lines[0].strip() if lines else ""
    return said if FORMAT_NAME.match(said) and said in NAMED.values() else REFUSED


def score(command):
    """The rate, and the two ways of failing, counted separately."""
    right = []
    refused = []
    confused = []

    with tempfile.TemporaryDirectory() as scratch:
        work = Path(scratch)
        for path, wanted in labelled_files():
            said = detect(command, path, work)
            if said == wanted:
                right.append((path, wanted))
                print(f"  {GREEN}✓{RESET} {path.name:<24} {said}")
            elif said in (REFUSED, UNDECODABLE):
                refused.append((path, wanted))
                print(f"  {YELLOW}·{RESET} {path.name:<24} {said}, expected {wanted}")
            else:
                confused.append((path, wanted, said))
                print(f"  {RED}✗{RESET} {path.name:<24} {said}  instead of {wanted}")

    return right, refused, confused


def report(right, refused, confused):
    total = len(right) + len(refused) + len(confused)
    print(f"\n    recognised formats: {len(right)}/{total}")
    print(f"    refused: {len(refused)}   —   confused: {len(confused)}")
    if confused:
        print(f"\n  {RED}the detection got it wrong, which is not the same as a refusal:{RESET}")
        for path, wanted, said in confused:
            print(f"    {path.name}: {said} instead of {wanted}")
    return len(right), total


def report_private(commands):
    """What the private corpus can say, and it cannot say a rate.

    **No file of it is named**, neither its title nor its file name: that is the
    repository's rule, and #290 learnt it the hard way.
    """
    files = sorted(p for p in PRIVATE.rglob("*")
                   if p.is_file() and p.suffix.lower() in SUBTITLE_SUFFIXES)
    if not files:
        print(f"\n{BOLD}private corpus — absent from this machine{RESET}")
        return

    print(f"\n{BOLD}private corpus — {len(files)} files, without labels{RESET}")
    print("  no rate: there is no truth to compare with, only the distribution")
    print("  of the answers.")

    with tempfile.TemporaryDirectory() as scratch:
        work = Path(scratch)
        for command in commands:
            answers = collections.Counter()
            for path in files:
                answers[detect(command, path, work)] += 1
            print(f"\n  {BOLD}{command}{RESET}")
            for name, count in answers.most_common():
                print(f"    {count:3d}  {name}")


def journal_text():
    if not JOURNAL.is_file():
        fail(f"journal not found: {JOURNAL}")
    return JOURNAL.read_text(encoding="utf-8")


def recorded_score(text):
    """The rate the journal shows, or the script stops.

    **A missing or duplicated anchor stops** rather than letting it through: a
    journal rewritten by hand on which nothing can be read any more would make
    the comparison silent, hence green.
    """
    found = RECORDED.findall(text)
    if len(found) != 1:
        fail(f"'recognised formats: N/M' appears {len(found)} times in"
             f" {JOURNAL.relative_to(REPO_ROOT)}, exactly one expected")
    return int(found[0][0]), int(found[0][1])


def version_of():
    for line in (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "VERSION":
            return parts[1]
    fail("unreadable version in CMakeLists.txt")
    return ""


def relative(path):
    return path.relative_to(CORPUS).as_posix()


def rewrite_journal(right, refused, confused):
    """Rewrites the journal's one generated block, and leaves the prose intact."""
    text = journal_text()
    opened = text.find(BLOCK_OPEN)
    closed = text.find(BLOCK_CLOSE)
    if opened < 0 or closed < opened:
        fail(f"record markers not found in {JOURNAL.relative_to(REPO_ROOT)}")

    total = len(right) + len(refused) + len(confused)
    body = [f"{BLOCK_OPEN}\n\n", f"    recognised formats: {len(right)}/{total}\n\n",
            f"Recorded on version {version_of()}, on"
            f" {datetime.date.today().isoformat()}.\n\n"]

    body.append("| | Files |\n| :--- | -------: |\n")
    body.append(f"| recognised | {len(right)} |\n")
    body.append(f"| refused, for want of a reader | {len(refused)} |\n")
    body.append(f"| **confused** | **{len(confused)}** |\n\n")

    if refused:
        body.append("Refused:\n\n")
        for path, wanted in refused:
            body.append(f"- `{relative(path)}` — {wanted}\n")
        body.append("\n")

    if confused:
        body.append("| File | Is | Read as |\n| :--- | :- | :------ |\n")
        for path, wanted, said in confused:
            body.append(f"| `{relative(path)}` | {wanted} | **{said}** |\n")
        body.append("\n")
    else:
        body.append("**No confusion**: the detection refuses, it does not get it wrong.\n\n")

    written = text[:opened] + "".join(body) + text[closed:]
    if written == text:
        print(f"{GREEN}✓{RESET} record already up to date: nothing to re-record")
        return 0
    JOURNAL.write_text(written, encoding="utf-8")
    print(f"{GREEN}✓{RESET} record written: {len(right)}/{total}")
    return 0


def compare_to_journal(right, refused, confused, record):
    """The three situations, separately — the mechanics of `make score`."""
    if record:
        return rewrite_journal(right, refused, confused)

    kept = len(right)
    total = kept + len(refused) + len(confused)
    was, over = recorded_score(journal_text())
    command = "make score-format-record"

    # **A confusion fails on its own**, whatever the rate. A format opened as
    # what it is not is what the project's rule forbids; a rate that rises while
    # a confusion appears must not buy it back.
    if confused:
        print(f"{RED}✗{RESET} the detection confuses {len(confused)} file(s):"
              " nothing rather than a guess", file=sys.stderr)
        for path, wanted, said in confused:
            print(f"    {relative(path)}: {said} instead of {wanted}", file=sys.stderr)
        return 1

    # Cross products rather than raw counts: a rendering added to the corpus
    # changes the denominator, and two counts no longer compare.
    if kept * over < was * total:
        print(f"{RED}✗{RESET} the detection went back: {kept}/{total} recognised,"
              f" against {was}/{over} in the record", file=sys.stderr)
        print(f"\n  compare with {JOURNAL.relative_to(REPO_ROOT)} to see what moved.",
              file=sys.stderr)
        print(f"  fix the detection, or re-record deliberately:\n    {command}",
              file=sys.stderr)
        return 1

    if (kept, total) != (was, over):
        print(f"{GREEN}✓{RESET} the detection improves: {kept}/{total} recognised,"
              f" against {was}/{over} in the record")
        print(f"  record it:\n    {command}")
        return 0

    print(f"{GREEN}✓{RESET} {kept}/{total} formats recognised, as in the record")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--detector", action="append", required=True,
                        help="detection command; {} receives the path")
    parser.add_argument("--private", action="store_true",
                        help="adds the private corpus, without drawing a rate from it")
    parser.add_argument("--journal", action="store_true",
                        help="compares the measurement with the record")
    parser.add_argument("--record", action="store_true",
                        help="rewrites the record from the measurement; implies --journal")
    options = parser.parse_args()

    # **A single detector when the journal is at stake.** The record carries the
    # score of the project's detection, not that of a witness compared with it.
    if (options.journal or options.record) and len(options.detector) != 1:
        parser.error("--journal and --record take a single --detector")

    right = refused = confused = None
    for command in options.detector:
        print(f"{BOLD}{command}{RESET}")
        right, refused, confused = score(command)
        report(right, refused, confused)

    if options.private:
        report_private(options.detector)

    if options.journal or options.record:
        return compare_to_journal(right, refused, confused, options.record)
    return 0


if __name__ == "__main__":
    sys.exit(main())
