#!/usr/bin/env python3
"""Hold the review caveat of a machine-translated language — spec 15, D5, issue #720.

A language other than French ships machine-translated, and says so **until a reader of the
language signs it off**. The one list of the languages still waiting is `src/po/UNREVIEWED`, one
code per line (blank lines and `#` comments allowed); signing a language off is deleting its line
there, and the caveat everywhere else in the same diff. This check makes the list and the caveat
agree, both ways: a caveat missing for a language of the list, or left behind for one the list no
longer names, fails.

    check-review-caveat.py [--root DIR]

## The caveat, where it is

    catalogue        `src/po/<lang>.po`: in the header's comments, before the first `msgid`,
                     the line, exactly:
                     # Machine translation, not reviewed by a reader of the language (spec 15, D5).
    each page        every `docs/i18n/<lang>/manual/**/*.md`: right after the `#` title line
                     (blank lines between them allowed), the marker line, exactly
                     `<!-- machine-translated -->`, and on the very next line a block quote,
                     `> …`, that says it in the language. The quote holds prose only — no
                     inline code and no link — so that the structure check
                     (`check-translation-structure.py`), which compares code spans and links
                     with the English page, has nothing to compare; `md-to-man.py` moves it
                     after the SYNOPSIS of a man page.
    Language section every page of the English manual `docs/manual/` with a `## Language`
                     section: in that section, for each language installed (a line of
                     `src/po/LINGUAS`) and still in `UNREVIEWED`, the line
                     `<!-- machine-translated: <lang> -->`, beside the sentence that says so.

## What is found

    UNKNOWN          `UNREVIEWED` names a language that has no `src/po/<lang>.po`
    NO MANUAL        a language of `LINGUAS` has no `docs/i18n/<lang>/manual/`: it would ship
                     with the English manual and English man pages, silently
    CATALOGUE        the catalogue of a language of `UNREVIEWED` lacks the header line
    CATALOGUE LEFT   the catalogue of any other language still carries it
    PAGE             a page of a language of `UNREVIEWED` lacks the marker, or its marker is not
                     right after the title, or is not followed by a quote of prose
    PAGE LEFT        a page of any other language still carries the marker
    SECTION          a `## Language` section of the English manual lacks the line of a language
                     installed and unreviewed
    SECTION LEFT     it names a language that is not installed and unreviewed

Every finding fails (exit code 1); a clean tree says how many languages wait for a reader. It
reads text files only, and takes a fraction of a second: it runs in the `translations` step of
`check-local`, after `check-translations.py`. `--root` names another work tree: the proofs of
`verify-gates.sh` point it at a scratch copy.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

CATALOGUE_LINE = "# Machine translation, not reviewed by a reader of the language (spec 15, D5)."
PAGE_MARKER = "<!-- machine-translated -->"
SECTION_MARKER = re.compile(r"^<!--\s*machine-translated:\s*([A-Za-z_@]+)\s*-->\s*$")
SECTION_HEADING = "## Language"

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"


def codes(path: pathlib.Path) -> list[str]:
    """The language codes of a list file: one per line, blank lines and `#` comments skipped."""
    if not path.exists():
        return []
    found = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            found.append(line)
    return found


def header_comments(po: pathlib.Path) -> list[str]:
    """The comment lines of a catalogue before its first `msgid`: the header's."""
    lines = []
    for line in po.read_text(encoding="utf-8").splitlines():
        if line.startswith("msgid"):
            break
        lines.append(line.rstrip())
    return lines


def page_problem(text: str) -> str | None:
    """Why a page does not carry the caveat as the convention writes it, or None when it does."""
    lines = text.splitlines()
    if not lines or not lines[0].startswith("# "):
        return "no `#` title on the first line, so no place for the marker"
    at = 1
    while at < len(lines) and not lines[at].strip():
        at += 1
    if at >= len(lines) or lines[at].strip() != PAGE_MARKER:
        if PAGE_MARKER in (line.strip() for line in lines):
            return f"{PAGE_MARKER} is not the first thing after the title"
        return f"no {PAGE_MARKER} after the title"
    quote = []
    at += 1
    while at < len(lines) and lines[at].startswith(">"):
        quote.append(lines[at])
        at += 1
    if not quote or not "".join(quote).lstrip("> ").strip():
        return f"{PAGE_MARKER} is not followed, on the next line, by a quote that says it"
    if any("`" in line or "](" in line for line in quote):
        return "the quote of the caveat holds inline code or a link: prose only"
    return None


def language_sections(page: pathlib.Path) -> list[tuple[int, list[str]]]:
    """The `## Language` sections of a page, each as its line and its lines up to the next heading
    of the same depth or above."""
    lines = page.read_text(encoding="utf-8").splitlines()
    sections = []
    for number, line in enumerate(lines):
        if line.strip() != SECTION_HEADING:
            continue
        body = []
        for following in lines[number + 1:]:
            if re.match(r"^#{1,2}\s", following):
                break
            body.append(following)
        sections.append((number + 1, body))
    return sections


def check(root: pathlib.Path) -> tuple[list[tuple[str, str, str]], list[str]]:
    po_dir = root / "src/po"
    installed = codes(po_dir / "LINGUAS")
    unreviewed = codes(po_dir / "UNREVIEWED")
    catalogues = sorted(path.stem for path in po_dir.glob("*.po"))
    found: list[tuple[str, str, str]] = []

    for language in unreviewed:
        if language not in catalogues:
            found.append(("UNKNOWN", "src/po/UNREVIEWED", f"names {language!r}, and {language}.po does not exist"))

    for language in installed:
        if not (root / "docs/i18n" / language / "manual").is_dir():
            found.append(("NO MANUAL", f"docs/i18n/{language}/manual",
                          f"{language!r} is in LINGUAS and has no translated manual"))

    for language in catalogues:
        carries = CATALOGUE_LINE in header_comments(po_dir / f"{language}.po")
        where = f"src/po/{language}.po"
        if language in unreviewed and not carries:
            found.append(("CATALOGUE", where, f"its header lacks the line {CATALOGUE_LINE!r}"))
        elif language not in unreviewed and carries:
            found.append(("CATALOGUE LEFT", where,
                          "its header still says it is not reviewed, and src/po/UNREVIEWED does not name it"))

    for tree in sorted((root / "docs/i18n").glob("*/manual")):
        language = tree.parent.name
        for page in sorted(tree.rglob("*.md")):
            where = str(page.relative_to(root))
            text = page.read_text(encoding="utf-8")
            if language in unreviewed:
                problem = page_problem(text)
                if problem:
                    found.append(("PAGE", where, problem))
            elif PAGE_MARKER in (line.strip() for line in text.splitlines()):
                found.append(("PAGE LEFT", where,
                              f"still carries {PAGE_MARKER}, and src/po/UNREVIEWED does not name {language!r}"))

    expected = sorted(language for language in installed if language in unreviewed)
    for page in sorted((root / "docs/manual").rglob("*.md")):
        for line, body in language_sections(page):
            where = f"{page.relative_to(root)}:{line}"
            named = sorted({match.group(1) for text in body if (match := SECTION_MARKER.match(text.strip()))})
            for language in sorted(set(expected) - set(named)):
                found.append(("SECTION", where,
                              f"no <!-- machine-translated: {language} --> for {language!r}, installed and unreviewed"))
            for language in sorted(set(named) - set(expected)):
                found.append(("SECTION LEFT", where,
                              f"<!-- machine-translated: {language} --> names a language that is not installed and unreviewed"))
    return found, expected


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=pathlib.Path, default=REPO_ROOT,
                        help="the work tree to check (default: this repository)")
    args = parser.parse_args()
    root = args.root.resolve()

    found, waiting = check(root)
    for kind, where, message in found:
        print(f"{RED}✗{RESET} {kind:<14} {where}: {message}")
    if found:
        print(f"{RED}✗{RESET} {len(found)} problem(s) with the review caveat: src/po/README.md says where it goes")
        return 1
    listed = ", ".join(waiting) if waiting else "none"
    print(f"{GREEN}✓{RESET} the review caveat agrees with src/po/UNREVIEWED;"
          f" installed and awaiting a reader: {listed}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
