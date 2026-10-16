#!/usr/bin/env python3
"""Measure, over a subtitle corpus, what hearing-impaired mentions amount to:
how many subtitles carry one, in which form, and how many become empty once
they are removed.

The proportions written in the benchmark fixture,
`src/test/bench/full_length_project.hpp`, come from here. The corpus read is
`src/data/`, private to each machine and ignored by git: the figures are copied
into the fixture, which reads no file at run time. This script exists so that
they can be redone — when the corpus grows, or when they are in doubt.

    ./src/scripts/measure-mentions.py [directory]

It belongs to no gate, and nothing depends on it: it is an instrument, not a
check. On a machine without a corpus, it says so and stops.
"""

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_CORPUS = REPO_ROOT / "src" / "data"
EXTENSIONS = {".srt", ".vtt", ".sub"}

# A mention fits on one line: the phase 4 pattern accepts a line break, but
# counting it here would mix two forms that are to be told apart.
MENTION = re.compile(r"\[[^\]\n]*\]|\([^)\n]*\)")
# The same, straddling the line break — counted apart.
STRADDLING = re.compile(r"\[[^\]]*\n[^\]]*\]|\([^)]*\n[^)]*\)")
# A whole line that is only a mention, a dialogue dash allowed.
SOLE_LINE = re.compile(r"^\s*(?:-\s*)?(?:\[[^\]\n]*\]|\([^)\n]*\))\s*$")
# Markup tags are not text: "<i>[sigh]</i>" is a mention alone, and staying
# one is what keeps the count of emptied subtitles right.
TAG = re.compile(r"</?[a-zA-Z][^>]*>")
TIMECODE = re.compile(r"\d{1,2}:\d{2}:\d{2}[.,]\d{1,3}\s*-->")


def decoded(raw: bytes) -> str:
    """Return the file's text, whatever its encoding — a real corpus mixes
    several, and refusing to read some would skew the proportions."""
    for encoding in ("utf-8-sig", "cp1252"):
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError:
            continue
    return raw.decode("latin-1", "replace")


def texts_of(path: Path) -> list[str]:
    """Return the text of each subtitle of the file.

    A deliberately coarse parse: a block, a timing line, and everything after
    it is text. It holds for SubRip as for WebVTT, and the point is to count
    mentions, not to rewrite a reader that exists."""
    content = decoded(path.read_bytes()).replace("\r\n", "\n").replace("\r", "\n")
    texts = []
    for block in re.split(r"\n[ \t]*\n", content):
        lines = [line for line in block.split("\n") if line.strip()]
        timecode = next((i for i, line in enumerate(lines) if TIMECODE.search(line)), None)
        if timecode is None:
            continue
        text = "\n".join(lines[timecode + 1:]).strip()
        if text:
            texts.append(TAG.sub("", text))
    return texts


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_CORPUS
    if not root.is_dir():
        print(f"corpus not found: {root}", file=sys.stderr)
        return 1

    files = sorted(
        path for path in root.rglob("*")
        if path.suffix.lower() in EXTENSIONS and "[write]" not in path.name)
    if not files:
        print(f"no subtitle file under {root}", file=sys.stderr)
        return 1

    total = biting = emptied = sole_line = straddling = brackets = parentheses = 0
    two_lines = 0
    lengths: list[int] = []
    per_file: list[tuple[str, int, int]] = []

    for path in files:
        texts = texts_of(path)
        if not texts:
            continue
        file_biting = 0
        for text in texts:
            total += 1
            lengths.append(len(text.replace("\n", " ")))
            two_lines += 1 if "\n" in text else 0
            straddling += 1 if STRADDLING.search(text) else 0
            if not MENTION.search(text):
                continue
            file_biting += 1
            biting += 1
            brackets += len(re.findall(r"\[[^\]\n]*\]", text))
            parentheses += len(re.findall(r"\([^)\n]*\)", text))
            if not MENTION.sub("", text).strip(" -\t\n"):
                emptied += 1
            elif "\n" in text and any(SOLE_LINE.match(l) for l in text.split("\n")):
                sole_line += 1
        per_file.append((str(path.relative_to(root)), len(texts), file_biting))

    lengths.sort()
    count = len(lengths)

    def share(part: int, whole: int) -> str:
        return f"{100 * part / whole:.1f} %" if whole else "—"

    print(f"{len(per_file)} files, {total} subtitles")
    print()
    print(f"  carrying a mention           {biting:6d}   {share(biting, total)} of the total")
    print(f"  emptied by its removal       {emptied:6d}   {share(emptied, biting)} of those hit")
    print(f"  with a whole line            {sole_line:6d}   {share(sole_line, biting)} of those hit")
    print(f"  straddling the break         {straddling:6d}   {share(straddling, total)} of the total")
    print(f"  brackets                     {brackets:6d}   "
          f"{share(brackets, brackets + parentheses)} of the mentions")
    print(f"  on two lines or more         {two_lines:6d}   {share(two_lines, total)} of the total")
    if count:
        print(f"  length: median {lengths[count // 2]}, "
              f"ninth decile {lengths[int(count * 0.9)]}, maximum {lengths[-1]}, "
              f"mean {sum(lengths) / count:.1f}")
    print()
    print("The proportion to keep is not the corpus mean: a file that is not")
    print("subtitled for the hearing impaired carries no mention, and there are")
    print("many. What counts is the range over the files that carry some.")
    print()
    print("     mention share     subtitles   file")
    for name, subtitles, file_biting in per_file:
        if file_biting:
            print(f"  {share(file_biting, subtitles):>16}   {subtitles:11d}   {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
