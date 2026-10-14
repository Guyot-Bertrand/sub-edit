#!/usr/bin/env python3
"""Check that the references of the manual point at something.

**The manual is the one part of the repository whose correctness rests entirely on
review.** Its `console` blocks are produced by running the command, its images by
photographing the real window — but its internal references were checked by nothing.
A renamed section leaves behind anchors that lead nowhere, and a dead link displays as
cleanly as a right one.

What is confronted:

    what a manual POINTS AT   docs/manual/**/*.md and docs/i18n/*/manual/**/*.md, relative
                              links and images
    what EXISTS               the file aimed at, and the anchor aimed at in that file

Errors (exit code 1)

    MISSING TARGET   the link names a file that does not exist.
    MISSING ANCHOR   the file exists, but no heading in it produces that anchor.
    MISSING IMAGE    the page shows an image that does not exist (#672: every tree shows its
                     own captures, so a translated tree whose captures were not taken fails here,
                     whatever its language).

External links — `http:`, `https:`, `mailto:` — are not followed: a check that goes out on the
network fails for reasons that are none of the repository's business. Whether an image is the
tree's own and is produced by something is `check-screenshots.py`'s to say.
"""

from __future__ import annotations

import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
MANUAL = REPO_ROOT / "docs/manual"
# The translated manuals (ADR 0043): their references are checked like the source's, and their
# anchors are those of their translated headings.
TRANSLATED = sorted((REPO_ROOT / "docs/i18n").glob("*/manual"))

# [text](target) — images, which start with "!", excluded.
LINK = re.compile(r"(?<!!)\[[^\]]*\]\(([^)\s]+)\)")
# ![text](target) — an image.
IMAGE = re.compile(r"!\[[^\]]*\]\(([^)\s]+)\)")
# An ATX heading: its level, then its text.
HEADING = re.compile(r"^(#{1,6})\s+(.*?)\s*$", re.MULTILINE)
# What a GitHub anchor does not keep: everything but letters, digits, spaces, hyphens and
# underscores. Accented letters are letters, and stay.
DROPPED = re.compile(r"[^\w\s-]", re.UNICODE)
EXTERNAL = ("http:", "https:", "mailto:")

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"


def anchor(title: str) -> str:
    """The anchor GitHub gives this heading."""
    return DROPPED.sub("", title.strip().lower()).replace(" ", "-")


def anchors(page: pathlib.Path) -> set[str]:
    """Every anchor of a page, numbered duplicates included."""
    found: set[str] = set()
    seen: dict[str, int] = {}
    for _, title in HEADING.findall(page.read_text(encoding="utf-8")):
        base = anchor(title)
        rank = seen.get(base, 0)
        seen[base] = rank + 1
        found.add(base if rank == 0 else f"{base}-{rank}")
    return found


def main() -> int:
    errors: list[str] = []
    checked = 0
    images = 0

    for page in sorted(page for root in (MANUAL, *TRANSLATED) for page in root.rglob("*.md")):
        here = page.relative_to(REPO_ROOT)
        text = page.read_text(encoding="utf-8")
        for target in LINK.findall(text):
            if target.startswith(EXTERNAL):
                continue
            if target.startswith("#"):
                # A bare anchor aims at the page itself: it is checked like the others, with no
                # file part.
                path, fragment = page, target[1:]
            else:
                file_part, _, fragment = target.partition("#")
                path = (page.parent / file_part).resolve()

            checked += 1
            if not path.is_file():
                errors.append(f"MISSING TARGET  {here} → {target}")
                continue
            if fragment and path.suffix == ".md" and fragment not in anchors(path):
                errors.append(f"MISSING ANCHOR  {here} → {target}")

        for target in IMAGE.findall(text):
            if target.startswith(EXTERNAL):
                continue
            images += 1
            if not (page.parent / target).resolve().is_file():
                errors.append(f"MISSING IMAGE   {here} → {target}")

    for line in errors:
        print(f"{RED}✗{RESET} {line}")

    if errors:
        print(f"\n{RED}✗{RESET} {len(errors)} reference(s) of the manual lead nowhere")
        return 1

    print(f"{GREEN}✓{RESET} {checked} link(s) and {images} image(s) of the manual, all valid")
    return 0


if __name__ == "__main__":
    sys.exit(main())
