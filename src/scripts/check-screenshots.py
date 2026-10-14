#!/usr/bin/env python3
"""Consistency guard of the manual's screenshots.

The arrangement has three moving parts, and nothing confronted them:

    what the capture program PRODUCES   src/test/tools/screenshots.cpp
    what a manual SHOWS                 docs/manual/**/*.md, docs/i18n/<lang>/manual/**/*.md
    what EXISTS on disk                 the `subedit-gui/captures/` directory of each tree

**One set per language** (#672): a shot carries the window's words, so every manual tree has
its own captures, with the same names — `docs/manual/subedit-gui/captures/` for English, the
source, and `docs/i18n/<lang>/manual/subedit-gui/captures/` for every other language, taken by
the same program under that language's catalogue (`make manual`). **Each tree is checked on its
own**, whatever its language: a page shows the images of its own tree, never another's.

Each defect below is **silent in a green chain**, because `compare-screenshots.py` only ever sees
the `*.new.png` a run produced: it cannot know of a manual pointing at nothing, nor of a reference
nothing produces any more.

Errors (exit code 1)

    BYPASS    a shot writes the reference's name directly instead of `<name>.new.png`, so the
              comparator never compares it: the run overwrites the reference in place, and a
              visual regression enters the manual with nobody seeing it.
    BROKEN    a manual shows an image that does not exist on disk — in a translated tree, a
              capture of that language that was never taken.
    FOREIGN   a manual shows an image outside its own tree's captures: a translated page that
              borrows the English set would show a window in the wrong language.
    FROZEN    a manual shows an image nothing produces: the reference can no longer be
              produced again, so it drifts away from the real window for ever. The costliest
              defect, because a stale image displays as cleanly as a right one.
    PAIR      a light shot has no dark one: the manual promises that every screen is shown under
              both palettes. Two cases, one error: `X-sombre` is not produced, or a manual shows
              `X.png` without showing `X-sombre.png`. Issue #400.

A screen that truly has one palette goes in `SINGLE_PALETTE`, with its reason: an exemption with
no reason is a forgotten pair that does not know it.

Notices (exit code 0)

    UNUSED    a shot no manual of the tree shows — build time spent for nothing.
    DEAD      a file of a tree's captures that nothing produces and nobody shows.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# The locations are read under a root, the repository's by default: `--root` moves it to a set
# written by hand, which is what lets `verify-gates.sh` put this check to the test without
# touching the real captures.
TOOL = "src/test/tools/screenshots.cpp"
MANUAL = "docs/manual"
TRANSLATIONS = "docs/i18n"
# Where the captures of a manual tree live, relative to that tree.
CAPTURES = "subedit-gui/captures"

# The suffix that makes a shot the dark one of the light one: `table` and `table-sombre`. A
# computed name would escape `produced()`, so the capture program writes both in full and this
# suffix is used here alone.
DARK_SUFFIX = "-sombre"

# The screens that truly have one palette: the name of the light shot, without `.png`, and WHY
# it has no dark one. **Empty today.** An entry is a decision, not a workaround: the reason is
# written, and a review reads it.
SINGLE_PALETTE: dict[str, str] = {}

# `capture…(` — the start of a call that photographs. What follows is read by counting
# parentheses rather than with a regular expression: an argument may hold some itself —
# `*window.table()` — and clang-format is free to break the call over four lines the day it
# grows. **The suffix is accepted since #321** (`captureSaveAsStrip`).
CAPTURE_CALL = re.compile(r"\bcapture[A-Za-z]*\(")
# Every literal that names a PNG in the capture program.
PNG_LITERAL = re.compile(r'"([^"]*\.png)"')
# ![text](path.png) in a manual.
MARKDOWN_IMAGE = re.compile(r"!\[[^\]]*\]\(([^)\s]+\.png)\)")

RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def produced(root: pathlib.Path = REPO_ROOT) -> set[str]:
    """The shots the program produces, read from its source: the last literal of each call."""
    source = (root / TOOL).read_text(encoding="utf-8")
    names: set[str] = set()
    for call in CAPTURE_CALL.finditer(source):
        literals = re.findall(r'"([^"]*)"', balanced(source, call.end()))
        if literals:
            names.add(literals[-1])
    return names


def balanced(source: str, start: int) -> str:
    """What runs from `start` to the parenthesis that closes the one before it."""
    depth = 1
    for index in range(start, len(source)):
        if source[index] == "(":
            depth += 1
        elif source[index] == ")":
            depth -= 1
            if depth == 0:
                return source[start:index]
    return ""


def bypasses(root: pathlib.Path = REPO_ROOT) -> list[str]:
    """The PNG literals of the program that do not go through `.new.png`."""
    return [
        literal
        for literal in PNG_LITERAL.findall((root / TOOL).read_text(encoding="utf-8"))
        if not literal.endswith(".new.png")
    ]


def trees(root: pathlib.Path = REPO_ROOT) -> list[pathlib.Path]:
    """The source manual and every translated one, `docs/i18n/<lang>/manual`, in a stable order."""
    found = [root / MANUAL] if (root / MANUAL).is_dir() else []
    return found + sorted(tree for tree in (root / TRANSLATIONS).glob("*/manual") if tree.is_dir())


def shown(tree: pathlib.Path, root: pathlib.Path) -> tuple[dict[str, list[pathlib.Path]], list[str]]:
    """The images a tree shows from its own captures, by name, and where; and those it borrows."""
    captures = (tree / CAPTURES).resolve()
    where: dict[str, list[pathlib.Path]] = {}
    foreign: list[str] = []
    for page in sorted(tree.rglob("*.md")):
        for reference in MARKDOWN_IMAGE.findall(page.read_text(encoding="utf-8")):
            target = (page.parent / reference).resolve()
            here = page.relative_to(root)
            if target.parent != captures:
                foreign.append(f"FOREIGN   {here} shows {reference}, outside its own tree's captures")
                continue
            where.setdefault(target.name, []).append(here)
    return where, foreign


def on_disk(tree: pathlib.Path) -> set[str]:
    captures = tree / CAPTURES
    return {found.name for found in captures.glob("*.png")} if captures.is_dir() else set()


def unpaired(produced_names: set[str], shown_names: dict[str, list[pathlib.Path]],
             exemptions: dict[str, str], label: str) -> list[str]:
    """The light shots whose dark one is missing, produced or shown."""
    light = sorted(
        name
        for name in produced_names | {image.removesuffix(".png") for image in shown_names}
        if not name.endswith(DARK_SUFFIX) and name not in exemptions
    )
    errors: list[str] = []
    for name in light:
        dark = f"{name}{DARK_SUFFIX}"
        if dark not in produced_names:
            errors.append(
                f"PAIR      {name}.png — {dark}.png is produced by nobody;\n"
                f"          every screen is shown under both palettes, or is exempted\n"
                f"          in SINGLE_PALETTE, with its reason"
            )
        elif f"{name}.png" in shown_names and f"{dark}.png" not in shown_names:
            errors.append(f"PAIR      {name}.png — shown, but {dark}.png is not, in {label}")
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument("--root", type=pathlib.Path, default=REPO_ROOT,
                        help="the root under which to read the program, the manuals and the captures")
    root = parser.parse_args(argv).root

    produced_names = produced(root)
    errors: list[str] = []
    notices: list[str] = []

    for literal in bypasses(root):
        errors.append(
            f"BYPASS    {literal} — the program would write the reference without going\n"
            f"          through the comparator; the suffix is \".new.png\", always"
        )

    shown_total = 0
    every_tree = trees(root)
    for tree in every_tree:
        label = str(tree.relative_to(root))
        shown_names, foreign = shown(tree, root)
        present = on_disk(tree)
        shown_total += len(shown_names)
        errors.extend(foreign)

        for name, pages in sorted(shown_names.items()):
            places = ", ".join(str(page) for page in pages)
            if name not in present:
                errors.append(f"BROKEN    {name} — shown by {places}, absent from {label}/{CAPTURES}")
            elif name.removesuffix(".png") not in produced_names:
                errors.append(
                    f"FROZEN    {name} — shown by {places}, produced by nobody;\n"
                    f"          nothing produces it again, it will drift from the window"
                )

        errors.extend(unpaired(produced_names, shown_names, SINGLE_PALETTE, label))

        for name in sorted(produced_names):
            if f"{name}.png" not in shown_names:
                notices.append(f"UNUSED    {name}.png — produced, shown by no page of {label}")
        for name in sorted(present):
            if name.removesuffix(".png") not in produced_names and name not in shown_names:
                notices.append(f"DEAD      {label}/{CAPTURES}/{name} — neither produced nor shown")

    for line in notices:
        print(f"{YELLOW}⚠{RESET} {line}")
    for line in errors:
        print(f"{RED}✗{RESET} {line}", file=sys.stderr)

    if errors:
        print(f"{RED}{len(errors)} screenshot inconsistency(ies){RESET}", file=sys.stderr)
        return 1

    print(
        f"{GREEN}✓{RESET} {len(produced_names)} shot(s) produced, {shown_total} shown "
        f"across {len(every_tree)} manual tree(s), nothing inconsistent"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
