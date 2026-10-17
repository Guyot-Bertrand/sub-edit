#!/usr/bin/env python3
"""Checks that the `subedit-cli` manual and its `--help` say the same names.

**What `manual-check` already held**: every subcommand page carries a
`subedit-cli <subcommand> --help` example, replayed, so that an added option
changes the help, makes the block stale, and fails the gate. **What it did not
hold**: the "Options" tables written by hand, a subcommand without a page, the
list of subcommands in `invocation.md`, and the `man` pages. Phase 13 adds
subcommands and dozens of options: that is where the drift gets paid for (issue
#546).

What is compared:

    what the binary SAYS       `subedit-cli --help` and `subedit-cli <sc> --help`
    what the manual WRITES     docs/manual/subedit-cli/*.md
    what the man pages WRITE   <man-root>/man1/subedit-cli*.1, generated from the manual by
                               md-to-man.py (#683), compressed or not

Every translated tree docs/i18n/<lang>/manual/subedit-cli is checked the same way, read with
the vocabulary of its language (doc_vocabulary.py), and so are its pages, under
<man-root>/<lang>/man1. `--man-root` defaults to the dev build tree, which lays the pages out
as an installation does; `check-installation.sh` points it at the installed ones.
`CLI-I18N-04`.

Errors (exit code 1), each one named:

    MISSING PAGE        a subcommand of the binary has no `<name>.md` page.
    UNDOCUMENTED OPTION a long option of the help appears in no table row of
                        the page of its subcommand.
    UNKNOWN OPTION      an option of the "Option" table of a page does not exist
                        in the help of the subcommand.
    UNLISTED / UNKNOWN SUBCOMMAND   the table of `invocation.md` and the binary
                        do not name the same subcommands.
    LANGUAGE WITHOUT VOCABULARY  a page is in a language that has no table (`doc_vocabulary.py`):
                        it would not be read, and the check would say nothing.
    MISSING GLOBAL OPTION    a global option of the binary is written neither in
                        `invocation.md` nor in the `subedit-cli(1)` page.
    MISSING MAN PAGE    `subedit-cli(1)`, or the page of a subcommand of the binary, is not
                        among the pages.
    UNKNOWN MAN PAGE    a `subedit-cli-<name>(1)` page names no subcommand of the binary.
    MAN PAGE WITHOUT OPTION  a long option of a subcommand's help is not in its page.

**What the check does not cover, and does not claim to cover**: the accepted
values, the defaults, the bounds and the "Errors" section stay written by hand —
the help does not carry them in a form that can be read without guessing. The
check therefore says *which names exist*, never *what they do*.
"""

from __future__ import annotations

import argparse
import gzip
import os
import pathlib
import re
import subprocess
import sys

import doc_vocabulary

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# A long option of the help: "--by TEXT", "-q,--quiet", "--to TEXT:{…}".
LONG_OPTION = re.compile(r"(?<![\w-])(--[a-z][a-z0-9-]*)")
# A long option written between backticks in the manual.
BACKTICKED_OPTION = re.compile(r"`(--[a-z][a-z0-9-]*)[^`]*`")
# The name, with or without the link to its page: "| [`shift`](shift.md) | …".
SUBCOMMAND_ROW = re.compile(r"^\|\s*\[?`([a-z][a-z0-9-]*)`\]?")


def run_help(binary: pathlib.Path, *args: str) -> str:
    """The output of `binary args --help`."""
    # In English, whatever the machine's locale: the manual checked here is the English one,
    # and `LANGUAGE=en` names the source language first (spec 15, D4).
    result = subprocess.run(
        [str(binary), *args, "--help"],
        capture_output=True,
        text=True,
        check=False,
        env={**os.environ, "LANGUAGE": "en"},
    )
    return result.stdout + result.stderr


def section(help_text: str, title: str) -> list[str]:
    """The lines of a section of the help (`Options:`, `Subcommands:`…)."""
    lines: list[str] = []
    inside = False
    for line in help_text.splitlines():
        if line.rstrip().endswith(":") and not line.startswith(" "):
            inside = line.strip() == title
            continue
        if inside:
            if not line.strip():
                if lines:
                    break
                continue
            lines.append(line)
    return lines


def long_options(help_text: str) -> set[str]:
    """The long options of the help, `--help` excluded: every subcommand has its own."""
    options: set[str] = set()
    for line in section(help_text, "Options:"):
        # The names column only: the descriptions quote other words.
        names = re.split(r"\s{2,}", line.strip(), maxsplit=1)[0]
        options.update(LONG_OPTION.findall(names))
    options.discard("--help")
    return options


def subcommands(help_text: str) -> list[str]:
    names: list[str] = []
    for line in section(help_text, "Subcommands:"):
        names.append(line.split()[0])
    return names


def table_rows(markdown: str) -> list[str]:
    return [line for line in markdown.splitlines() if line.lstrip().startswith("|")]


def documented_options(markdown: str) -> set[str]:
    """Every long option written between backticks in a table row."""
    found: set[str] = set()
    for row in table_rows(markdown):
        found.update(BACKTICKED_OPTION.findall(row))
        for cell in re.findall(r"`(--[a-z][a-z0-9-]*)", row):
            found.add(cell)
    return found


def option_column(markdown: str, option_header: str) -> set[str]:
    """The options of the first column of the tables whose header is the options one."""
    found: set[str] = set()
    inside = False
    for line in markdown.splitlines():
        stripped = line.strip()
        if not stripped.startswith("|"):
            inside = False
            continue
        cells = [cell.strip() for cell in stripped.strip("|").split("|")]
        if cells and cells[0] == option_header:
            inside = True
            continue
        if inside and cells:
            found.update(re.findall(r"`(--[a-z][a-z0-9-]*)", cells[0]))
    return found


def check(binary: pathlib.Path, manual: pathlib.Path, vocabularies: dict[str, dict]) -> list[str]:
    failures: list[str] = []
    unread: dict[str, list[str]] = {}
    top_help = run_help(binary)
    names = subcommands(top_help)
    if not names:
        return [f"NO SUBCOMMAND: `{binary} --help` lists none"]

    for name in names:
        page = manual / f"{name}.md"
        if not page.is_file():
            failures.append(f"MISSING PAGE: the subcommand `{name}` has no {page.name}")
            continue
        text = page.read_text(encoding="utf-8")
        language = doc_vocabulary.language_of(page, text)
        if language not in vocabularies:
            unread.setdefault(language, []).append(page.name)
            continue
        from_help = long_options(run_help(binary, name))
        written = documented_options(text)
        for option in sorted(from_help - written):
            failures.append(
                f"UNDOCUMENTED OPTION: `{name} {option}` is in the help, "
                f"not in a table of {page.name}"
            )
        for option in sorted(option_column(text, vocabularies[language]["option_header"]) - from_help):
            failures.append(
                f"UNKNOWN OPTION: {page.name} documents `{option}`, "
                f"which the help of `{name}` does not know"
            )

    invocation = manual / "invocation.md"
    invocation_text = invocation.read_text(encoding="utf-8") if invocation.is_file() else ""
    listed: set[str] = set()
    inside = False
    invocation_language = doc_vocabulary.language_of(invocation, invocation_text)
    if invocation_language not in vocabularies:
        unread.setdefault(invocation_language, []).append(invocation.name)
        subcommand_header = None
    else:
        subcommand_header = vocabularies[invocation_language]["subcommand_header"]
    for line in invocation_text.splitlines():
        if subcommand_header is not None and line.startswith(f"| {subcommand_header}"):
            inside = True
            continue
        if inside:
            match = SUBCOMMAND_ROW.match(line)
            if match:
                listed.add(match.group(1))
            elif not line.startswith("|"):
                # Only the first table: the later ones that open on the same word (the keys of
                # `counts`, one row per subcommand) would list a subcommand the table forgot (#671).
                break
    failures.extend(doc_vocabulary.missing_table(vocabularies, unread))
    if subcommand_header is not None:
        for name in sorted(set(names) - listed):
            failures.append(f"UNLISTED SUBCOMMAND: `{name}` is not in the table of {invocation.name}")
        for name in sorted(listed - set(names)):
            failures.append(f"UNKNOWN SUBCOMMAND: {invocation.name} lists `{name}`, which the binary does not know")

    for option in sorted(long_options(top_help)):
        if f"`{option}" not in invocation_text:
            failures.append(f"MISSING GLOBAL OPTION: `{option}` is not written in {invocation.name}")
    return failures


def man_text(page: pathlib.Path) -> str | None:
    """A page as `man` reads it, compressed or not, or None if there is neither."""
    if page.is_file():
        return page.read_text(encoding="utf-8")
    compressed = page.with_name(page.name + ".gz")
    if compressed.is_file():
        return gzip.decompress(compressed.read_bytes()).decode("utf-8")
    return None


def names_option(page: str, option: str) -> bool:
    """True if a page writes the option, as troff writes it: `\\-\\-by`, not followed by more."""
    escaped = re.escape(option.replace("-", "\\-"))
    return re.search(escaped + r"(?![a-z0-9]|\\-)", page) is not None


def check_man_pages(binary: pathlib.Path, pages: pathlib.Path) -> list[str]:
    """The pages of one language against the binary: one per subcommand, each with the options
    of its help, and `subedit-cli(1)` with the global ones — `CLI-I18N-04`."""
    failures: list[str] = []
    top_help = run_help(binary)
    names = subcommands(top_help)

    top = man_text(pages / "subedit-cli.1")
    if top is None:
        failures.append(f"MISSING MAN PAGE: subedit-cli(1) is not in {pages}")
    else:
        for option in sorted(long_options(top_help)):
            if not names_option(top, option):
                failures.append(f"MISSING GLOBAL OPTION: `{option}` is not written in {pages}/subedit-cli.1")

    for name in names:
        text = man_text(pages / f"subedit-cli-{name}.1")
        if text is None:
            failures.append(f"MISSING MAN PAGE: subedit-cli-{name}(1) is not in {pages}")
            continue
        for option in sorted(long_options(run_help(binary, name))):
            if not names_option(text, option):
                failures.append(f"MAN PAGE WITHOUT OPTION: `{name} {option}` is in the help, "
                                f"not in subedit-cli-{name}(1)")

    found = {re.sub(r"\.1(\.gz)?$", "", page.name) for page in pages.glob("subedit-cli-*.1*")}
    for page in sorted(found - {f"subedit-cli-{name}" for name in names}):
        failures.append(f"UNKNOWN MAN PAGE: {page}(1) in {pages} names no subcommand of the binary")
    return failures


def man_pages_of(man_root: pathlib.Path, manual: pathlib.Path) -> pathlib.Path:
    """Where the pages of a manual tree are: `man1` for the source language, `<lang>/man1` for
    a translation — the layout of `share/man`."""
    language = doc_vocabulary.language_of(manual / "invocation.md", "")
    if language == doc_vocabulary.SOURCE_LANGUAGE:
        return man_root / "man1"
    return man_root / language / "man1"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--binary", type=pathlib.Path,
                        default=REPO_ROOT / "build/dev/bin/subedit-cli")
    parser.add_argument("--manual", type=pathlib.Path,
                        help="one manual to check; by default docs/manual/subedit-cli and every "
                             "docs/i18n/<lang>/manual/subedit-cli")
    parser.add_argument("--man-root", type=pathlib.Path,
                        default=REPO_ROOT / "build/dev/share/man",
                        help="the man pages, laid out as share/man is (default: the dev build tree)")
    parser.add_argument("--vocabulary", type=pathlib.Path,
                        help="a JSON file {language: table} that adds to the shipped tables")
    parser.add_argument("--pages-only", action="store_true",
                        help="check the man pages alone, not the Markdown: what an installation "
                             "check asks, the manual being checked by manual-check")
    arguments = parser.parse_args()

    if not arguments.binary.is_file():
        print(f"binary not found: {arguments.binary}", file=sys.stderr)
        return 2

    tables = doc_vocabulary.load_tables(arguments.vocabulary)
    if arguments.manual is not None:
        trees = [arguments.manual]
    else:
        # The source manual and every translation of it, whatever the language: each one is
        # read with its own vocabulary, and a language without a table is refused.
        trees = [REPO_ROOT / "docs/manual/subedit-cli",
                 *doc_vocabulary.translated_trees(REPO_ROOT / "docs/manual", "subedit-cli")]
    failures = []
    for tree in trees:
        label = tree.relative_to(REPO_ROOT) if tree.is_relative_to(REPO_ROOT) else tree
        prefix = "" if tree == trees[0] else f"{label}: "
        if not arguments.pages_only:
            failures.extend(prefix + failure for failure in check(arguments.binary, tree, tables))
        failures.extend(prefix + failure
                        for failure in check_man_pages(arguments.binary, man_pages_of(arguments.man_root, tree)))
    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        print(f"\n{len(failures)} mismatch(es) between the subedit-cli manual and its --help",
              file=sys.stderr)
        return 1

    print("the subedit-cli manual and its --help name the same subcommands and options")
    return 0


if __name__ == "__main__":
    sys.exit(main())
