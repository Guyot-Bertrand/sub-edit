#!/usr/bin/env python3
"""Check that the manual of `subedit-gui` writes the shortcuts the window declares.

**What `make check` already held**: that no two actions share a shortcut (`no two actions
answer the same shortcut`, `window_actions_test.cpp`), so new ones as soon as they enter
`WindowActions`. **What it did not hold**: that the manual says them. Its shortcuts are written
by hand, in tables, and nothing confronted them with the window — the defect
`check-cli-manual.py` settled for the command line (#546), and that phase 14 made worse: some
fifteen keyboard gestures cannot be kept in step by eye (issue #612).

What is confronted:

    what the window DECLARES   `subedit_list_shortcuts` — the real `WindowActions`, with no
                               screen, every action with its shortcuts and its label
    what a manual WRITES       docs/manual/subedit-gui/*.md, and every translation of it,
                               docs/i18n/<lang>/manual/subedit-gui/*.md

**Each page against the window in its own language** (#672): a translated page quotes the
labels of the translated window, so the window is listed in the page's language —
`subedit_list_shortcuts --language <lang>`, under that language's shipped catalogue — and
English, the source language, with no option. Nothing here names a language: a tree under
`docs/i18n/<lang>/` is read as soon as it exists, and a language the window cannot be built in
fails, said once.

**The form the manual must have**, so that the comparison is not a `grep`: a table with **a
column headed "Shortcut"** (in the page's language: `doc_vocabulary.py`) and another —
"Command", "Entry", "Action" or "Menu entry" — that names the action as the menu labels it, in
plain text, possibly preceded by its menu (Video ▸ Play / Pause). The shortcut is written as
code (`Ctrl+P`), and keys are not translated; "none" or "—" says there is none. The check reads
those tables and those alone: the prose quotes keys that are not actions of the window (`F2`
opens a cell's editor, for instance).

Errors (exit code 1), each one named:

    UNKNOWN ACTION         a table row names an action the window does not have, in the
                           page's language.
    UNKNOWN SHORTCUT       a table row writes a shortcut its action does not answer.
    FORGOTTEN SHORTCUT     a table row says "none", or writes nothing, of an action that has one.
    LANGUAGE WITHOUT VOCABULARY   a page is in a language with no table (`doc_vocabulary.py`):
                           it would not be read, and the check would say nothing.
    WINDOW LANGUAGE UNAVAILABLE   a page is in a language the window cannot be built in (no
                           catalogue of ours): it would be compared with the wrong window.
    PAGE LANGUAGE IN DOUBT a page is read in one language and its shortcut tables are headed in
                           another's words: it would be read as having no table (#671).
    NO SHORTCUT TABLE      no page of the manual has a shortcut table the check can read.
    UNDOCUMENTED SHORTCUT  an action answers a shortcut, and none of its shortcuts is written
                           anywhere in the manual tree.

**One of an action's shortcuts is enough to document it**: Qt gives several depending on the
platform (`Redo` answers three), and requiring the manual to say them all would make it describe
a platform rather than the program. The one it writes, on the other hand, must be one of them.

**What the check does not cover, and does not claim to**: what a shortcut does, where it lives,
the keys of the editors and dialogs, which are not actions of the window. It says *which
shortcuts exist*, never *what they do*.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

import doc_vocabulary

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# The words of headers and cells live in `doc_vocabulary.py`, one table per language (#657): the
# logic below writes no word of a page.

# What Qt writes and what the manual writes for the same key: both come down to one form.
KEY_NAMES = {
    "PgUp": "PageUp",
    "PgDown": "PageDown",
    "Ins": "Insert",
    "Del": "Delete",
    "Esc": "Escape",
    "Return": "Enter",
}

# What a cell may say for "no shortcut", whatever the language.
NO_SHORTCUT = {"", "—", "-"}

# A shortcut written as code: `Ctrl+Shift+S`, `F1`, `Del`.
CODE_SPAN = re.compile(r"`([^`]+)`")
# A key sequence: one or more keys joined by `+`. **The last may be `+` or `-`**: Gaupol's volume
# is `Ctrl++` and `Ctrl+-`, and a check that did not read them would have left them written
# without confronting them with anything.
SEQUENCE = re.compile(r"^(?:[A-Za-z0-9]+\+)*(?:[A-Za-z0-9]+|\+|-)$")
# The keys of a sequence, a final `+` being a key and not a separator.
KEYS = re.compile(r"[^+]+|\+$")


class Unavailable(Exception):
    """The window cannot be listed in a language."""


def canonical(sequence: str) -> str:
    """The common form of a sequence: the long key names, the modifiers as they are."""
    return "+".join(KEY_NAMES.get(part, part) for part in KEYS.findall(sequence.strip()))


def declared(binary: pathlib.Path, language: str) -> tuple[dict[str, set[str]], set[str]]:
    """What the window declares in `language`: the shortcuts of every named action, and all of them.

    The source language is the window with no option; any other is `--language <language>`."""
    command = [str(binary)]
    if language != doc_vocabulary.SOURCE_LANGUAGE:
        command += ["--language", language]
    result = subprocess.run(command, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        raise Unavailable(result.stderr.strip() or f"{binary} exited with {result.returncode}")

    by_label: dict[str, set[str]] = {}
    every: set[str] = set()
    for line in result.stdout.splitlines():
        label, _, sequences = line.partition("\t")
        keys = {canonical(part) for part in sequences.split(", ") if part.strip()}
        every |= keys
        if label:
            by_label.setdefault(label, set()).update(keys)
    return by_label, every


def code_spans(markdown: str) -> list[str]:
    """The code spans of the prose, **line by line**: a code block opens and closes backticks by
    three, and reading the text in one go mismatches everything after it."""
    found: list[str] = []
    fenced = False
    for line in markdown.splitlines():
        if line.lstrip().startswith("```"):
            fenced = not fenced
        elif not fenced:
            found.extend(CODE_SPAN.findall(line))
    return found


def cells(line: str) -> list[str]:
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def written_shortcuts(cell: str, vocabulary: dict) -> set[str] | None:
    """The shortcuts of a cell, or `None` when it says there are none."""
    keys = {canonical(span) for span in code_spans(cell) if SEQUENCE.match(span.strip())}
    if keys:
        return keys
    none = NO_SHORTCUT | set(vocabulary["no_shortcut_words"])
    return None if cell.strip().strip("`*").lower() in none else set()


def action_label(cell: str) -> str:
    """The label the menu gives: without backticks, and without the menu before it."""
    return cell.replace("`", "").split("▸")[-1].strip()


def tables(markdown: str, vocabulary: dict):
    """The rows of the tables with a shortcut column: (line number, label, cell)."""
    shortcut_header = vocabulary["shortcut_header"]
    lines = markdown.splitlines()
    index = 0
    while index < len(lines):
        header = lines[index]
        if not header.lstrip().startswith("|") or shortcut_header not in cells(header):
            index += 1
            continue

        names = cells(header)
        shortcut_column = names.index(shortcut_header)
        label_column = next((names.index(h) for h in vocabulary["label_headers"] if h in names), None)
        index += 2  # the header and its separator row
        while index < len(lines) and lines[index].lstrip().startswith("|"):
            row = cells(lines[index])
            if label_column is not None and len(row) > max(shortcut_column, label_column):
                yield index + 1, action_label(row[label_column]), row[shortcut_column]
            index += 1


def check(binary: pathlib.Path, manual: pathlib.Path, vocabularies: dict[str, dict]) -> list[str]:
    failures: list[str] = []
    pages = sorted(manual.glob("*.md"))
    all_text = ""
    unread: dict[str, list[str]] = {}
    rows_read = 0
    # The window in every language the tree's pages are written in, listed once per language.
    windows: dict[str, tuple[dict[str, set[str]], set[str]] | None] = {}

    def window(language: str, page: str) -> tuple[dict[str, set[str]], set[str]] | None:
        if language not in windows:
            try:
                windows[language] = declared(binary, language)
            except Unavailable as error:
                windows[language] = None
                failures.append(
                    f"WINDOW LANGUAGE UNAVAILABLE: {page} is in \"{language}\", and the window cannot "
                    f"be built in that language — {error}"
                )
        return windows[language]

    for page in pages:
        text = page.read_text(encoding="utf-8")
        all_text += text + "\n"

        language = doc_vocabulary.language_of(page, text)
        if language not in vocabularies:
            unread.setdefault(language, []).append(page.name)
            continue
        vocabulary = vocabularies[language]

        # A page read in the wrong language finds none of its tables, and says nothing: a shortcut
        # column headed in another language's words, and never in its own, is that mistake (#671).
        headers = [cells(line) for line in text.splitlines() if line.lstrip().startswith("|")]
        own = any(vocabulary["shortcut_header"] in header for header in headers)
        foreign = sorted({other for other, table in vocabularies.items()
                          if table["shortcut_header"] != vocabulary["shortcut_header"]
                          and any(table["shortcut_header"] in header for header in headers)})
        if foreign and not own:
            failures.append(
                f"PAGE LANGUAGE IN DOUBT: {page.name} is read as \"{language}\", but its shortcut tables are "
                f"headed in the words of \"{', '.join(foreign)}\" — declare its language "
                f"(<!-- language: … -->) or move it under docs/i18n/<lang>/manual/"
            )
            continue

        rows = list(tables(text, vocabulary))
        if not rows:
            continue
        listed = window(language, page.name)
        if listed is None:
            continue
        by_label = listed[0]
        for number, label, cell in rows:
            rows_read += 1
            where = f"{page.name}:{number}"
            if label not in by_label:
                failures.append(
                    f"UNKNOWN ACTION: {where} names \"{label}\", which the window in \"{language}\" does not have"
                )
                continue

            actual = by_label[label]
            written = written_shortcuts(cell, vocabulary)
            if written is None:
                if actual:
                    failures.append(
                        f"FORGOTTEN SHORTCUT: {where} says \"{label}\" has none, "
                        f"it answers {', '.join(sorted(actual))}"
                    )
                continue
            if not written and actual:
                failures.append(
                    f"FORGOTTEN SHORTCUT: {where} does not write that of \"{label}\" ({', '.join(sorted(actual))})"
                )
            for key in sorted(written - actual):
                failures.append(
                    f"UNKNOWN SHORTCUT: {where} writes `{key}` for \"{label}\", "
                    f"which answers {', '.join(sorted(actual)) or 'none'}"
                )

    failures.extend(doc_vocabulary.missing_table(vocabularies, unread))
    if pages and rows_read == 0 and not unread and all(windows.values()):
        failures.append(f"NO SHORTCUT TABLE: no page of {manual} has one this check can read")

    # Every action that answers a shortcut sees it written somewhere — under one of the key's forms.
    # The keys do not depend on the language, so any listing of the tree's languages will do.
    listings = [listed for listed in windows.values() if listed is not None]
    if not listings:
        return failures
    by_label, every = listings[0]
    spans = {canonical(span) for span in code_spans(all_text) if SEQUENCE.match(span.strip())}
    named = {key for keys in by_label.values() for key in keys}
    for label, keys in sorted(by_label.items()):
        if keys and not keys & spans:
            failures.append(
                f"UNDOCUMENTED SHORTCUT: \"{label}\" answers {', '.join(sorted(keys))}, "
                f"which no page of the manual writes"
            )
    # The shortcuts of the actions with no name (the keys of the tabs): the manual writes them too.
    for key in sorted(every - named):
        if key not in spans:
            failures.append(f"UNDOCUMENTED SHORTCUT: the window answers {key}, which no page of the manual writes")
    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--binary", type=pathlib.Path,
                        default=REPO_ROOT / "build/dev/bin/subedit_list_shortcuts")
    parser.add_argument("--manual", type=pathlib.Path,
                        help="one manual to check; by default docs/manual/subedit-gui and every "
                             "docs/i18n/<lang>/manual/subedit-gui")
    parser.add_argument("--vocabulary", type=pathlib.Path,
                        help="a JSON file {language: table} completing the shipped tables")
    arguments = parser.parse_args()

    if not arguments.binary.is_file():
        print(f"binary not found: {arguments.binary}", file=sys.stderr)
        return 2

    tables = doc_vocabulary.load_tables(arguments.vocabulary)
    if arguments.manual is not None:
        failures = check(arguments.binary, arguments.manual, tables)
    else:
        # The source manual and every translation of it, whatever the language (#671): each
        # tree has to write every shortcut, in the words of its own language, and quote the
        # labels of the window in that language (#672).
        failures = check(arguments.binary, REPO_ROOT / "docs/manual/subedit-gui", tables)
        for tree in doc_vocabulary.translated_trees(REPO_ROOT / "docs/manual", "subedit-gui"):
            label = tree.relative_to(REPO_ROOT)
            failures.extend(f"{label}: {failure}" for failure in check(arguments.binary, tree, tables))
    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        print(f"\n{len(failures)} difference(s) between the subedit-gui manual and its shortcuts",
              file=sys.stderr)
        return 1

    print("the subedit-gui manual writes the window's shortcuts, and only its own")
    return 0


if __name__ == "__main__":
    sys.exit(main())
