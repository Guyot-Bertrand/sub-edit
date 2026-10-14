#!/usr/bin/env python3
"""The vocabulary the manual checks look for, one table per language.

**A check that looks for a French word and finds none on an English page does not
fail: it says "nothing to check".** That is the failure `docs/exigences.md` names
(#657). The checks that read the manual — `check-gui-manual.py`, `check-cli-manual.py` —
therefore carry no word of a page in their logic: they ask for them here, per language.

    language of a page  declared by `<!-- language: fr -->` in its first five lines,
                        otherwise that of its tree: `docs/i18n/<lang>/manual/`, otherwise
                        the source language (`SOURCE_LANGUAGE`) of `docs/manual/`
    vocabulary          `TABLES[language]`, completed by a JSON file (`--vocabulary`)

**A page whose language has no table fails the check**, once per language. Adding a
language to the manual means adding its table here, and nothing else allows it: a
translated tree `docs/i18n/<lang>/manual/` is read by the checks as soon as it exists,
and refused until its table does.

The declaration is the exception, for a page whose tree does not say its language: a page
of `docs/manual/` that is not yet in the source language declares the language it is written
in. The window manual was the last such tree; #672 wrote it in English, and moved its French
pages to `docs/i18n/fr/manual/`, where the tree says their language.

**The language of a page is also the window's it is compared with** (#672): a translated page
quotes the labels of the translated window, so `check-gui-manual.py` lists the window in the
language this module gives the page — the source language with no option, any other under its
shipped catalogue. A tree in a language the window cannot be built in is refused there.

Keys of a table:

    shortcut_header     the header of the shortcut column
    label_headers       the possible headers of the column that names the action
    no_shortcut_words   the words that say "no shortcut" (besides "", "—", "-")
    option_header       the header of the first column of the option tables
    subcommand_header   the header of the subcommand table of `invocation.md`
"""

from __future__ import annotations

import json
import pathlib
import re

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# The language of the pages that are neither declared nor under `docs/i18n/`: that of `docs/manual/`.
SOURCE_LANGUAGE = "en"

KEYS = ("shortcut_header", "label_headers", "no_shortcut_words", "option_header", "subcommand_header")

TABLES: dict[str, dict] = {
    "en": {
        "shortcut_header": "Shortcut",
        "label_headers": ["Command", "Entry", "Action", "Menu entry"],
        "no_shortcut_words": ["none"],
        "option_header": "Option",
        "subcommand_header": "Subcommand",
    },
    "fr": {
        "shortcut_header": "Raccourci",
        "label_headers": ["Commande", "Entrée", "Action", "Entrée du menu"],
        "no_shortcut_words": ["aucun", "aucune"],
        "option_header": "Option",
        "subcommand_header": "Sous-commande",
    },
}

DECLARATION = re.compile(r"<!--\s*language:\s*([A-Za-z][A-Za-z_@-]*)\s*-->")
TREE = re.compile(r"(?:^|/)i18n/([^/]+)/manual(?:/|$)")


def load_tables(extra: pathlib.Path | None) -> dict[str, dict]:
    """The tables shipped, plus those of a JSON file `{language: table}`."""
    tables = {language: dict(table) for language, table in TABLES.items()}
    if extra is not None:
        for language, table in json.loads(extra.read_text(encoding="utf-8")).items():
            missing = [key for key in KEYS if key not in table]
            if missing:
                raise SystemExit(f"{extra}: the table \"{language}\" has no {', '.join(missing)}")
            tables[language] = table
    return tables


def declared_language(text: str) -> str | None:
    """The language a page declares in its first five lines, or None."""
    for line in text.splitlines()[:5]:
        if match := DECLARATION.search(line):
            return match.group(1)
    return None


def language_of(page: pathlib.Path, text: str) -> str:
    """The language of a page: its declaration, otherwise its tree, otherwise the source language."""
    if declared := declared_language(text):
        return declared
    if match := TREE.search(page.resolve().as_posix()):
        return match.group(1)
    return SOURCE_LANGUAGE


def translated_trees(manual_root: pathlib.Path, part: str) -> list[pathlib.Path]:
    """`docs/i18n/<lang>/manual/<part>` for every language that has one, in a stable order."""
    i18n = manual_root.parent / "i18n"
    return sorted(tree for tree in i18n.glob(f"*/manual/{part}") if tree.is_dir())


def missing_table(tables: dict[str, dict], pages_by_language: dict[str, list[str]]) -> list[str]:
    """One finding per page language that has no table, with the pages that speak it."""
    return [
        f"LANGUAGE WITHOUT VOCABULARY: the pages in \"{language}\" ({', '.join(sorted(pages))}) "
        f"have no vocabulary table (doc_vocabulary.py, or --vocabulary)"
        for language, pages in sorted(pages_by_language.items())
        if language not in tables
    ]
