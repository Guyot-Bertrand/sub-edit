#!/usr/bin/env python3
"""Lints `man` pages by rendering them with `man`, and refuses the slightest warning.

    check-man-pages.py PATH...

A path is a page (`.1`, `.1.gz`) or a directory, every page under which is read.
**What is asked is what a reader gets**: `man -l --warnings=w` renders each page, with every
groff warning turned on, twice — under `C.UTF-8`, the terminal most readers have, and under
`C`, where groff renders in ASCII and a character it has no glyph for draws nothing. Each
page must also be ASCII from end to end: troff reads bytes, and a page that is ASCII renders
the same whether or not `preconv` is called for it (see `md-to-man.py`). A language whose script
ASCII cannot draw at all (Hebrew, Cyrillic, Chinese) is asked of the first terminal only.

**The exit code of `man` says nothing**: groff warns and returns 0. So it is the error output
that is read, and one byte of it is a failure.

Errors (exit code 1), each one named:

    NOT ASCII   a byte of the page is outside ASCII
    WARNING     `man` wrote something on its error output while rendering the page
    NO PAGE     a path given holds no page: a lint that reads nothing proves nothing

Exit code 2 when `man` is missing. Used by `check-installation.sh` on the installed pages and
proven by `verify-gates.sh` on a broken one — issue #683.
"""

from __future__ import annotations

import gzip
import os
import pathlib
import shutil
import subprocess
import sys

LOCALES = ("C.UTF-8", "C")

# A language written in a script that ASCII cannot draw, not even with an accent: under `C` its
# words draw nothing whatever the page does, so only the first terminal is asked of it. The
# page's language is the directory above `man1` (`share/man/he/man1`).
NON_LATIN_LANGUAGES = ("he", "ru", "sr", "zh_CN")


def pages_under(path: pathlib.Path) -> list[pathlib.Path]:
    if path.is_dir():
        return sorted(page for page in path.rglob("*") if page.name.endswith((".1", ".1.gz")))
    return [path]


def lint(man: str, page: pathlib.Path) -> list[str]:
    failures: list[str] = []
    raw = page.read_bytes()
    text = gzip.decompress(raw) if page.name.endswith(".gz") else raw
    for number, line in enumerate(text.splitlines(), start=1):
        if any(byte >= 0x80 for byte in line):
            failures.append(f"NOT ASCII: {page}:{number}: {line[:60]!r}")
            break
    # What the caller's environment would add to the call is taken out: a MANOPT or a pager
    # chosen by whoever runs the check must not decide what it sees.
    environment = {key: value for key, value in os.environ.items()
                   if key not in ("MANOPT", "MANPAGER", "PAGER", "MANROFFOPT", "LANGUAGE")}
    locales = LOCALES
    if page.parent.parent.name in NON_LATIN_LANGUAGES:
        locales = LOCALES[:1]
    for locale in locales:
        result = subprocess.run(
            [man, "-l", "--warnings=w", str(page)],
            capture_output=True,
            check=False,
            env={**environment, "LC_ALL": locale, "MANWIDTH": "80"},
        )
        complaints = result.stderr.decode("utf-8", errors="replace").strip()
        if complaints or result.returncode != 0:
            detail = complaints or f"exit code {result.returncode}"
            failures.append(f"WARNING: {page} under {locale}:\n"
                            + "\n".join(f"    {line}" for line in detail.splitlines()))
    return failures


def main() -> int:
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
        print(__doc__, file=sys.stderr)
        return 2
    man = shutil.which("man")
    if man is None:
        print("man is missing: the pages cannot be rendered\n"
              "  install it with \"./src/scripts/setup-toolchain.sh\"", file=sys.stderr)
        return 2

    failures: list[str] = []
    count = 0
    for argument in sys.argv[1:]:
        pages = pages_under(pathlib.Path(argument))
        if not pages or not all(page.is_file() for page in pages):
            failures.append(f"NO PAGE: {argument} holds no man page")
            continue
        for page in pages:
            count += 1
            failures.extend(lint(man, page))

    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        print(f"\n{len(failures)} problem(s) in {count} man page(s)", file=sys.stderr)
        return 1
    print(f"the {count} man page(s) render without a single warning")
    return 0


if __name__ == "__main__":
    sys.exit(main())
