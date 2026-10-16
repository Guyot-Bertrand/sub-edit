#!/usr/bin/env python3
"""The encoding fixtures: make them, and check that they are what the table says
they are.

Phase 8 reads files that are not UTF-8. Testing it requires files that **are** in
the encoding attributed to them — not UTF-8 files described as such. And once
committed, nobody can reread them in a diff to make sure: it is the problem
`subtitle-fixtures.py` solved for frame grids, and it is the same one, worse — a
file of six hundred cues is unreadable in a diff, a CP1252 file does not even say
what it is.

    --check       (default) rebuild each fixture and compare it with the disk
    --generate    rewrite the fixtures from the table

**The table is the source**, and it carries everything that tells one file from
another: its encoding, its BOM, its line ending and its text. Making follows from
it, and `--check` compares **byte by byte**.

**No external tool** — like `subtitle-fixtures.py`, and unlike `video-fixtures.sh`,
which needs ffmpeg. Python's codecs are deterministic, so the rebuild is
reproducible everywhere and the comparison can be exact rather than rely on a
third party.

The cue texts are test data, French, Czech, Polish, Hungarian, Russian and
English, and live in `src/test/data/encodages/fixture-cues.json`.

## What the table covers, and why

It is not the phase's set of encodings — the framing (#288) decides that. It is
the set that makes it possible to **tell apart the cases a reader must know how
to tell apart**:

  bare UTF-8, UTF-8 with BOM  today's case, and the only one already read
  UTF-16 LE and BE, with BOM  the BOM also carries the byte order there, which a
                              boolean `hadUtf8Bom` cannot say
  Latin-1 and CP1252          **the pair that counts**: identical outside the
                              0x80–0x9f range, so indistinguishable on a text
                              that does not use it — a detection must be able
                              to say so rather than decide
  CP1250                      Central Europe, where the same bytes carry other
                              letters
  KOI8-R                      a non-Latin single-byte encoding, which no Latin
                              heuristic can confuse

**Every fixture carries the same cues**, up to the accents its encoding can
write. That is what allows comparing one reading with another without comparing
two different texts.

## The three short ones, and what they add

The first nine are between a hundred and fifty and four hundred bytes long, and
a score taken on them alone **measures only the easy case** — the defect issue
#310 raised. Three fixtures of a single cue are added: `latin1-court`,
`cp1250-court`, `koi8-r-court`.

**It is not the length that decides, it is the high bytes**, and the
measurement says so — `score-encoding-detection.py --by-length`. One cue carries
three where three cues carry nine, and that is the side on which detection stops
knowing.

Each carries **the first cue of its set**, and that choice is a rule rather than
a selection: choosing the cue after seeing the answer would make the corpus say
the score one wanted to read. `cp1250-court` comes back wrong; it stays.

## The bilingual one, and it is the one that cost the most

`koi8-r-rare` is the opposite of the three short ones: thirteen hundred bytes,
nineteen of them outside ASCII. Twenty English cues, one Russian — a title card,
a translator's note, what a real file often carries.

It came back as `ISO-8859-1`, and the Russian with it, as accented Latin. It was
not a lack of information: `uchardet` answered right on the same file. It was the
mass of ASCII weighing in a letter statistic where it had nothing to say — ADR
0028.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
FIXTURE_DIR = REPO_ROOT / "src" / "test" / "data" / "encodages"

GREEN = "\033[32m"
RED = "\033[31m"
BOLD = "\033[1m"
RESET = "\033[0m"

NEWLINES = {"lf": "\n", "crlf": "\r\n", "cr": "\r"}

# A name of its own, so that the table's line does not spell "le" twice: the
# language check would read two French articles in it.
UTF_16_LE = "utf-16-le"

BOMS = {
    "utf-8": b"\xef\xbb\xbf",
    UTF_16_LE: b"\xff\xfe",
    "utf-16-be": b"\xfe\xff",
}

# The cue texts, by the character set they need. The Latin, Central European
# and Cyrillic sets say the same thing; what changes is what the encoding can
# write. The English set carries no high byte: it fills a bilingual file around
# the rare cues that do. Only one fixture uses it, and it is the one that counts
# most — #310.
CUES = json.loads((FIXTURE_DIR / "fixture-cues.json").read_text(encoding="utf-8"))
LATIN = CUES["latin"]
CENTRAL = CUES["central"]
CYRILLIC = CUES["cyrillic"]
ENGLISH = CUES["english"]

# name, encoding, BOM, line ending, cues, what the fixture shows
FIXTURES = [
    ("utf-8-lf.srt", "utf-8", False, "lf", LATIN,
     "today's case, and the reference for the others"),
    ("utf-8-bom-crlf.srt", "utf-8", True, "crlf", LATIN,
     "what a Windows editor leaves"),
    ("utf-16-le-bom.srt", UTF_16_LE, True, "crlf", LATIN,
     "the BOM carries the byte order, not only the encoding"),
    ("utf-16-be-bom.srt", "utf-16-be", True, "crlf", LATIN,
     "the other order, which nothing tells from the first without its BOM"),
    ("latin1.srt", "iso-8859-1", False, "lf", LATIN,
     "Western Europe, without a byte in the 0x80-0x9f range"),
    ("cp1252.srt", "cp1252", False, "lf", LATIN + [CUES["cp1252-extra"]],
     "the same family, but with the 0x80-0x9f range — hence the last cue"),
    ("cp1250.srt", "cp1250", False, "lf", CENTRAL,
     "Central Europe: the same bytes, other letters"),
    ("koi8-r.srt", "koi8-r", False, "lf", CYRILLIC,
     "a non-Latin single-byte encoding, which no Latin heuristic confuses"),
    ("cr-mac.srt", "utf-8", False, "cr", LATIN,
     "the bare carriage return, the line ending of classic Mac OS"),
    # The three short ones — #310. The first cue of their set, and nothing
    # else: the choice is a rule and not a selection, otherwise the corpus
    # would say the score one wanted to read.
    ("latin1-court.srt", "iso-8859-1", False, "lf", LATIN[:1],
     "the same, on one cue: three high bytes instead of nine"),
    ("cp1250-court.srt", "cp1250", False, "lf", CENTRAL[:1],
     "Central Europe on one cue, and the cliff starts there"),
    ("koi8-r-court.srt", "koi8-r", False, "lf", CYRILLIC[:1],
     "Cyrillic on one cue, which still holds"),
    # The bilingual file — #310, and the most instructive fixture of the
    # corpus. Twenty English cues and one Cyrillic: in the letter statistic,
    # the mass of ASCII is all there is to weigh, and the answer was ISO-8859-1
    # until the detection stopped weighing what does not discriminate.
    ("koi8-r-rare.srt", "koi8-r", False, "lf", ENGLISH + CYRILLIC[:1],
     "a Cyrillic title card in an English file, drowned by the mass of ASCII"),
]


def render(lines, newline):
    """A SubRip file, in the form the project writes."""
    parts = []
    for index, text in enumerate(lines, start=1):
        start = index * 2
        parts.append(
            f"{index}{newline}"
            f"00:00:{start:02d},000 --> 00:00:{start + 1:02d},000{newline}"
            f"{text}{newline}{newline}"
        )
    return "".join(parts)


def build(encoding, bom, newline_name, lines):
    """The exact bytes of a fixture."""
    content = render(lines, NEWLINES[newline_name])
    body = content.encode(encoding)
    return (BOMS[encoding] if bom else b"") + body


def generate():
    print(f"{BOLD}making the encoding fixtures{RESET}")
    FIXTURE_DIR.mkdir(parents=True, exist_ok=True)
    for name, encoding, bom, newline, lines, description in FIXTURES:
        data = build(encoding, bom, newline, lines)
        (FIXTURE_DIR / name).write_bytes(data)
        print(f"  {GREEN}✓{RESET} {name} — {encoding}"
              f"{', BOM' if bom else ''}, {newline}, {len(data)} bytes — {description}")
    return 0


def check():
    failures = 0
    for name, encoding, bom, newline, lines, _ in FIXTURES:
        path = FIXTURE_DIR / name
        if not path.is_file():
            print(f"  {RED}✗{RESET} {name} — missing;"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            failures += 1
            continue

        expected = build(encoding, bom, newline, lines)
        actual = path.read_bytes()
        if actual != expected:
            # **The first byte that differs, not "the file has changed".** An
            # encoding fixture has no readable diff: without this position, the
            # only way to understand is to take it apart by hand.
            where = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b),
                         min(len(actual), len(expected)))
            print(f"  {RED}✗{RESET} {name} — the file is no longer what the table says,"
                  f" first difference at byte {where}"
                  f" ({len(actual)} bytes against {len(expected)} expected);"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            failures += 1
            continue

        print(f"  {GREEN}✓{RESET} {name} — {encoding}"
              f"{', BOM' if bom else ''}, {newline}, {len(actual)} bytes")

    if failures:
        print(f"\n{RED}✗{RESET} {failures} encoding fixture(s) no longer"
              " what the table announces", file=sys.stderr)
        return 1
    return 0


def main():
    argument = sys.argv[1] if len(sys.argv) > 1 else "--check"
    if argument == "--check":
        return check()
    if argument == "--generate":
        return generate()
    print(f"usage: {Path(sys.argv[0]).name} [--check|--generate]", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
