#!/usr/bin/env python3
"""Check that a translated manual keeps the structure of its source.

**The manual is the one text that exists in two languages** (ADR 0043, 0044): the
English tree `docs/manual/` is the source, and every other language lives in
`docs/i18n/<lang>/manual/`, page for page. Two texts drift apart whenever nobody
compares them, and a translation that quietly lost a table row, a code block
or a link is wrong in a way no reader of the translation can see.

What is compared is the **structure**, never the prose. A page and its
translation must agree on:

    MISSING PAGE   a source page has no translation
    EXTRA PAGE     a translated page has no source
    HEADINGS       the depth of every heading, in order
    TABLE          the number of tables, their rows and columns, and the inline
                   code of every cell (options, shortcuts, key names)
    CODE BLOCK     the fenced blocks, byte for byte, with their language tag
                   (the examples are replayed by `manual-check`, so they must
                   be the same text in both trees)
    LINK           the files the links point to (the anchors may differ: a
                   translated heading has a translated anchor, and
                   `check-manual-links.py` checks that each resolves)
    IMAGE          the images shown
    CODE SPAN      the inline code outside tables: identifiers are never
                   translated
    LIST ITEMS     the number of list items

**There is no exemption.** A translated page may not lag behind its source: the
check fails until the two agree again. This settles the question phase 15 left
open (#656): a declared, dated lag is a second thing to keep up to date, and
the structure is cheap to keep in step when it is kept at every change.

A link that leaves the tree is compared after resolving it, and a link into the
translated tree counts as its counterpart in the source tree, so `../x.md`
written from a page one directory deeper is the same link.

With no translated tree the check says so and passes: it does not pretend to
have compared anything. `--check-fixtures` replays the versioned cases of
`src/test/data/traductions/`.
"""

from __future__ import annotations

import argparse
import collections
import json
import pathlib
import re
import shutil
import sys
import tempfile
from dataclasses import dataclass, field

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
SOURCE = REPO_ROOT / "docs/manual"
TRANSLATIONS = REPO_ROOT / "docs/i18n"
CASES = REPO_ROOT / "src/test/data/traductions"

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"

FENCE = re.compile(r"^\s*(`{3,}|~{3,})\s*(\S*)")
HEADING = re.compile(r"^(#{1,6})\s+\S")
LIST_ITEM = re.compile(r"^\s*(?:[-*+]|\d+[.)])\s+\S")
CODE_SPAN = re.compile(r"(`+)(.+?)\1")
IMAGE = re.compile(r"!\[[^\]]*\]\(([^)\s]+)\)")
LINK = re.compile(r"(?<!!)\[[^\]]*\]\(([^)\s]+)\)")
EXTERNAL = ("http:", "https:", "mailto:")


@dataclass
class Table:
    line: int
    rows: int
    columns: int
    cells: list[list[tuple[str, ...]]]


@dataclass
class Page:
    headings: list[tuple[int, int]] = field(default_factory=list)  # (line, level)
    fences: list[tuple[int, str, str]] = field(default_factory=list)  # (line, info, body)
    tables: list[Table] = field(default_factory=list)
    links: list[tuple[int, str]] = field(default_factory=list)  # (line, resolved target)
    images: list[tuple[int, str]] = field(default_factory=list)
    spans: list[tuple[int, str]] = field(default_factory=list)  # outside tables
    items: list[int] = field(default_factory=list)  # lines


def split_cells(row: str) -> list[str]:
    """The cells of a table row; a `|` inside inline code or escaped does not split."""
    text = row.strip().removeprefix("|").removesuffix("|")
    cells: list[str] = []
    current: list[str] = []
    in_code = False
    previous = ""
    for char in text:
        if char == "`":
            in_code = not in_code
        if char == "|" and not in_code and previous != "\\":
            cells.append("".join(current))
            current = []
        else:
            current.append(char)
        previous = char
    cells.append("".join(current))
    return cells


def spans_of(text: str) -> tuple[str, ...]:
    return tuple(sorted(match.group(2).strip() for match in CODE_SPAN.finditer(text)))


def blank_code(text: str) -> str:
    """The text with the inside of every code span blanked, so that a link written in code is no link."""
    return CODE_SPAN.sub(lambda match: " " * len(match.group(0)), text)


def canonical(target: str, page: pathlib.Path, root: pathlib.Path, source_root: pathlib.Path) -> str:
    """A link target as a path in the source tree when it points into this one."""
    if target.startswith(EXTERNAL):
        return target
    file_part, _, _ = target.partition("#")
    if not file_part:
        return "#"  # an anchor on the page itself
    resolved = (page.parent / file_part).resolve()
    try:
        return str(source_root / resolved.relative_to(root))
    except ValueError:
        return str(resolved)


def parse(page: pathlib.Path, root: pathlib.Path, source_root: pathlib.Path) -> Page:
    parsed = Page()
    lines = page.read_text(encoding="utf-8").splitlines()
    fence: tuple[str, str, int] | None = None  # marker, info, starting line
    body: list[str] = []
    table: list[tuple[int, str]] = []

    def close_table() -> None:
        if not table:
            return
        rows = [split_cells(text) for _, text in table]
        parsed.tables.append(
            Table(
                line=table[0][0],
                rows=len(rows),
                columns=len(rows[0]),
                cells=[[spans_of(cell) for cell in row] for row in rows],
            )
        )
        table.clear()

    for number, line in enumerate(lines, start=1):
        if fence is not None:
            marker, info, start = fence
            if line.strip().startswith(marker[0] * len(marker)) and not line.strip().strip(marker[0]):
                parsed.fences.append((start, info, "\n".join(body)))
                fence = None
                body = []
            else:
                body.append(line)
            continue

        opening = FENCE.match(line)
        if opening:
            close_table()
            fence = (opening.group(1), opening.group(2), number)
            continue

        if line.lstrip().startswith("|"):
            table.append((number, line))
            for target in LINK.findall(blank_code(line)):
                parsed.links.append((number, canonical(target, page, root, source_root)))
            continue
        close_table()

        heading = HEADING.match(line)
        if heading:
            parsed.headings.append((number, len(heading.group(1))))
        if LIST_ITEM.match(line):
            parsed.items.append(number)
        for target in IMAGE.findall(line):
            parsed.images.append((number, canonical(target, page, root, source_root)))
        bare = blank_code(line)
        for target in LINK.findall(bare):
            parsed.links.append((number, canonical(target, page, root, source_root)))
        for match in CODE_SPAN.finditer(line):
            parsed.spans.append((number, match.group(2).strip()))

    close_table()
    if fence is not None:
        parsed.fences.append((fence[2], fence[1], "\n".join(body)))
    return parsed


def multiset_gap(
    source: list[tuple[int, str]], translation: list[tuple[int, str]]
) -> tuple[list[tuple[int, str]], list[tuple[int, str]]]:
    """(in the source only, in the translation only), each entry with its line."""

    def unmatched(
        mine: list[tuple[int, str]], theirs: list[tuple[int, str]]
    ) -> list[tuple[int, str]]:
        budget = collections.Counter(key for _, key in theirs)
        left = []
        for line, key in mine:
            if budget[key] > 0:
                budget[key] -= 1
            else:
                left.append((line, key))
        return left

    return unmatched(source, translation), unmatched(translation, source)


def compare(
    name: str, source: Page, translation: Page, base: pathlib.Path
) -> list[tuple[str, str, int, str]]:
    """The differences of one page pair as (kind, page, line, message)."""
    found: list[tuple[str, str, int, str]] = []

    def show(key: str) -> str:
        """A path relative to the directory holding the manual, a code span as it is."""
        try:
            return str(pathlib.Path(key).relative_to(base))
        except ValueError:
            return key

    def at(kind: str, line: int, message: str) -> None:
        found.append((kind, name, line, message))

    s_levels = [level for _, level in source.headings]
    t_levels = [level for _, level in translation.headings]
    if s_levels != t_levels:
        first = next(
            (i for i, (a, b) in enumerate(zip(s_levels, t_levels)) if a != b),
            min(len(s_levels), len(t_levels)),
        )
        line = translation.headings[first][0] if first < len(translation.headings) else (
            translation.headings[-1][0] if translation.headings else 1
        )
        at("HEADINGS", line, f"heading {first + 1} differs from the source: depths "
           f"{t_levels[first:first + 1] or 'none'} against {s_levels[first:first + 1] or 'none'}")

    if len(source.tables) != len(translation.tables):
        line = translation.tables[-1].line if translation.tables else 1
        at("TABLE", line, f"{len(translation.tables)} table(s), the source has {len(source.tables)}")
    for index, (s_table, t_table) in enumerate(zip(source.tables, translation.tables), start=1):
        if (s_table.rows, s_table.columns) != (t_table.rows, t_table.columns):
            at("TABLE", t_table.line, f"table {index} is {t_table.rows}×{t_table.columns}, "
               f"the source's is {s_table.rows}×{s_table.columns}")
        elif s_table.cells != t_table.cells:
            at("TABLE", t_table.line, f"table {index} differs from the source in the inline code of a cell")

    if len(source.fences) != len(translation.fences):
        line = translation.fences[-1][0] if translation.fences else 1
        at("CODE BLOCK", line, f"{len(translation.fences)} code block(s), the source has {len(source.fences)}")
    for index, (s_fence, t_fence) in enumerate(zip(source.fences, translation.fences), start=1):
        if s_fence[1:] != t_fence[1:]:
            at("CODE BLOCK", t_fence[0], f"code block {index} is not the source's, byte for byte")

    for kind, label, s_list, t_list in (
        ("LINK", "link", source.links, translation.links),
        ("IMAGE", "image", source.images, translation.images),
        ("CODE SPAN", "inline code", source.spans, translation.spans),
    ):
        lost, gained = multiset_gap(s_list, t_list)
        # A thing changed is one difference, not a loss and a gain.
        for (_, was), (line, now) in zip(lost, gained):
            at(kind, line, f"{label} «{show(now)}» replaces «{show(was)}» of the source")
        for line, key in gained[len(lost):]:
            at(kind, line, f"{label} «{show(key)}» is not in the source")
        for line, key in lost[len(gained):]:
            at(kind, translation.headings[0][0] if translation.headings else 1,
               f"{label} «{show(key)}» of the source line {line} is missing")

    if len(source.items) != len(translation.items):
        line = translation.items[-1] if translation.items else 1
        at("LIST ITEMS", line, f"{len(translation.items)} list item(s), the source has {len(source.items)}")
    return found


def pages(root: pathlib.Path) -> dict[str, pathlib.Path]:
    return {str(page.relative_to(root)): page for page in sorted(root.rglob("*.md"))}


def check_pair(source: pathlib.Path, translation: pathlib.Path) -> list[tuple[str, str, int, str]]:
    source_root = source.resolve()
    translation_root = translation.resolve()
    from_source = pages(source_root)
    from_translation = pages(translation_root)
    found: list[tuple[str, str, int, str]] = []
    for name in sorted(from_source.keys() - from_translation.keys()):
        found.append(("MISSING PAGE", name, 0, "the source page has no translation"))
    for name in sorted(from_translation.keys() - from_source.keys()):
        found.append(("EXTRA PAGE", name, 0, "the translated page has no source"))
    for name in sorted(from_source.keys() & from_translation.keys()):
        found.extend(
            compare(
                name,
                parse(from_source[name], source_root, source_root),
                parse(from_translation[name], translation_root, source_root),
                source_root.parent,
            )
        )
    return found


def report(label: str, found: list[tuple[str, str, int, str]]) -> None:
    for kind, name, line, message in found:
        where = f"{name}:{line}" if line else name
        print(f"{RED}✗{RESET} {kind}  {label}/{where}: {message}")


def check_fixtures() -> int:
    """Replay the versioned cases: the faithful translation passes, each edit is refused once."""
    cases = json.loads((CASES / "cases.json").read_text(encoding="utf-8"))
    failures = 0
    for case in cases:
        with tempfile.TemporaryDirectory() as scratch:
            docs = pathlib.Path(scratch) / "docs"
            shutil.copytree(CASES / "docs", docs)
            translation = docs / "i18n/es/manual"
            edit = case.get("edit", {})
            if "delete" in edit:
                (translation / edit["delete"]).unlink()
            if "create" in edit:
                (translation / edit["create"]).write_text(edit["text"], encoding="utf-8")
            if "file" in edit:
                path = translation / edit["file"]
                text = path.read_text(encoding="utf-8")
                if edit["old"] not in text:
                    print(f"{RED}✗{RESET} {case['name']}: «{edit['old']}» is not in {edit['file']}")
                    failures += 1
                    continue
                path.write_text(text.replace(edit["old"], edit["new"], 1), encoding="utf-8")
            found = check_pair(docs / "manual", translation)

        expected = case.get("expect")
        if expected is None:
            ok = not found
        else:
            ok = len(found) == 1 and (found[0][0], found[0][1], found[0][2]) == (
                expected["kind"], expected["page"], expected["line"])
        if ok:
            print(f"{GREEN}✓{RESET} {case['name']}")
        else:
            failures += 1
            print(f"{RED}✗{RESET} {case['name']}: expected {expected}, got:")
            report("fixture", found)
    if failures:
        print(f"\n{RED}✗{RESET} {failures} case(s) of {len(cases)} do not behave as written")
        return 1
    print(f"{GREEN}✓{RESET} {len(cases)} translation case(s) behave as written")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--source", type=pathlib.Path, default=SOURCE, help="the source manual")
    parser.add_argument("--translation", type=pathlib.Path,
                        help="one translated manual; by default every docs/i18n/<lang>/manual")
    parser.add_argument("--check-fixtures", action="store_true",
                        help="replay the versioned cases instead of checking the manual")
    args = parser.parse_args()

    if args.check_fixtures:
        return check_fixtures()

    trees = [args.translation] if args.translation else sorted(TRANSLATIONS.glob("*/manual"))
    if not trees:
        print(f"{GREEN}✓{RESET} no translated manual: nothing to compare with the source")
        return 0

    total = 0
    for tree in trees:
        found = check_pair(args.source, tree)
        report(str(tree), found)
        total += len(found)
    if total:
        print(f"\n{RED}✗{RESET} {total} difference(s) between the source manual and its translation")
        return 1
    print(f"{GREEN}✓{RESET} {len(trees)} translated manual(s), same structure as the source")
    return 0


if __name__ == "__main__":
    sys.exit(main())
