#!/usr/bin/env python3
"""Refuse French text anywhere in the repository, except where French is a locale.

**Everything is written in English; French exists as a locale only** — ADR 0043,
decision D7 of the phase 15 spec, issue #660. This check holds that rule over
every file `git ls-files` names: code and its comments, scripts, CMake and the
`Makefile`, documentation, specs, ADRs, YAML, packaging. Files git does not
track — `src/data/`, `reference/`, build trees — are never read; a new file is
read from its `git add` on. The script keeps the name it had when it read C++
comments only (#312, #325), so that what calls it does not change.

## The descending allowlist

About 220 000 words were French when the rule changed, and they are translated
in batches. Until the last batch, the files that still contain French are listed
in `.french-allowlist`, one repository-relative path per line, sorted; blank
lines and lines starting with `#` are ignored. The check fails when:

    UNLISTED    a file that is not on the list contains French;
    UNSORTED    the entries are not sorted, or not unique;
    STALE       a listed file no longer contains French — remove its line;
    MISSING     a listed file does not exist — remove its line;
    GROWN       the list has a line that its version at the merge base with
                `origin/main` does not have.

**An entry can be removed and never added.** The last rule is what makes the
list descend: it compares the working copy of the list with
`git show <base>:.french-allowlist`. When no base is reachable — a shallow
clone, an archive, a repository without `origin/main` — or when the base has no
list yet, that comparison is skipped and says so; the others still run.
`--base <revision>` names the base instead.

**To remove an entry**, translate the file, run this script, and delete the line
it reports as STALE. `--list` prints every file the classifier finds French in,
which is what the list was generated from.

## What is not read, and why

    src/po/**             the catalogues: French is a locale there
    docs/i18n/fr/**       the translated manual: French is a locale there
    CHANGELOG.md          generated from commit subjects, and history is not
                          rewritten (ADR 0043): the old subjects are French
    src/test/data/**      test data — subtitles, patterns, dictionaries, manual
                          fixtures — whose language is the point of the data;
                          its README.md and LISEZMOI.md files are read
    packaging/patterns/** pattern files taken from Gaupol, per-language data;
                          its README.md and LISEZMOI.md files are read
    binary files          anything with a NUL byte or that is not UTF-8

## How a line is classified

A line is classified by a **vote of its function words**: those with no
homograph in the other language, plus the elisions — `l'`, `d'`, `qu'` — that
exist only in French. A line is French when it has at least two French votes
and more French votes than English ones. Two votes, not one: a lone `du`, `ne`
or `est` is a shell command, an option or a time zone more often than a
sentence.

**A non-ASCII character does not decide**, unlike the test-title check: the
English prose of this repository is full of em dashes and guillemets.

Before the vote, a line loses what is not prose: inline code between backticks,
URLs, Markdown link targets, quotations between guillemets or curly quotes,
alternations of words (`le|la|les`, the vocabulary of a language check),
command-line options (`-ne`, `--le`), and, in Markdown, whole fenced code
blocks and the example markers of the manual (`<!-- exemple: <shell line> -->`,
the line `generate-manual.sh` runs and repeats at the head of the block below:
code, whatever language its sample subtitles are in). Lines shorter than twelve characters are not voted on. A line that
names a French locale explicitly — `[fr]`, `xml:lang="fr"` — is a translation
and is skipped.

**C++ is read through its tokens**: comments are voted on, without what they
quote between double quotes, and so are the string literals of a line that
calls a Catch2 reporting macro (`FAIL`, `SKIP`, `INFO`, `BENCHMARK`…). Every
other string literal is data — the subtitle a test reads, the text a correction
rewrites — and raw strings always are. Every other file is voted on whole, line
by line: a script's messages are prose as much as its comments.

**A file contains French when one line is French.** The threshold is one so
that a French sentence in a new file is refused; the two-vote rule above is what
keeps English files from being caught.

## Self-test

`--check-fixtures` replays the versioned cases of `src/test/data/language/`:
lines whose expected class is written next to them, and small repositories
built in a scratch directory where each rule above must fire once, or not at all.

Exit status: 0 when the tree is clean, 1 otherwise.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
ALLOWLIST = ".french-allowlist"
FIXTURES = REPO_ROOT / "src/test/data/language"
UPSTREAM = "origin/main"

GREEN, RED, YELLOW, RESET = "\033[32m", "\033[31m", "\033[33m", "\033[0m"

# French function words with no English homograph, and the elisions. "on", "son", "sa", "a",
# "or", "de", "en", "et" and "un" are left out: they are English words, or English text quotes
# them (`de`, `en`, `fr` are language codes; `un` is a prefix in identifiers).
FRENCH = re.compile(
    r"\b(le|la|les|une|des|qui|que|qu|pour|dans|est|pas|ce|cette|cet|plutôt|donc"
    r"|elle|aux|du|sans|avec|ne|par|quand|même|leur|nous|où|être|fait|ici|tout"
    r"|tous|cela|au|il|ils|elles|deux|trois|chaque|celui|celle|ceux|puis|alors"
    r"|entre|sous|vers|depuis|jamais|toujours|rien|autre|autres|comme|mais|sont"
    r"|sur|ou|ses|ces|dont|aussi|encore|très|déjà|là|été|avoir|fichier|fichiers"
    r"|ligne|lignes)\b"
    r"|\b(?:[lnjdmtcs]|qu)[’'](?=\w)",
    re.IGNORECASE,
)

# Their English counterparts, chosen the same way.
ENGLISH = re.compile(
    r"\b(the|is|are|of|that|and|to|it|what|which|with|for|not|its|this|these"
    r"|those|has|have|had|does|do|but|from|would|will|be|been|when|so|than"
    r"|then|only|rather|because|one|two|three|each|both|there|here|where|who"
    r"|why|how|into|over|under|after|before|never|always|nothing|else|same"
    r"|file|files|line|lines|if|or|an|by|at|on|in|as|can|must|should|no)\b",
    re.IGNORECASE,
)

SHORTEST_SENTENCE = 12
LEAST_FRENCH_VOTES = 2

INLINE_CODE = re.compile(r"`[^`]*`")
URL = re.compile(r"\b(?:https?|ftp|file)://\S+")
LINK_TARGET = re.compile(r"\]\([^)]*\)")
TYPOGRAPHIC_QUOTE = re.compile(r"«[^»]*»|“[^”]*”")
PLAIN_QUOTE = re.compile(r'"[^"]*"')
ALTERNATION = re.compile(r"\|?\w+(?:\|\w+)+")
OPTION = re.compile(r"(?<![\w-])--?[A-Za-z][\w-]*")
LOCALE_TAG = re.compile(r"\[fr(?:_[A-Z]{2})?\]|lang=[\"']fr")
FENCE = re.compile(r"^\s*(```|~~~)")
# The marker of a replayed example of the manual: a shell line, not prose (generate-manual.sh).
EXAMPLE_MARKER = re.compile(r"^\s*<!--\s*exemple:.*-->\s*$")
CPP = (".cpp", ".hpp", ".h", ".cc")
# A comment, a raw string, a string or a character literal of C++, in the order they open.
CPP_TOKEN = re.compile(
    r"//[^\n]*|/\*.*?\*/"
    r'|(?<![\w])(?:u8|[uUL])?R"(?P<delimiter>[^(\s"\\]{0,16})\(.*?\)(?P=delimiter)"'
    r'|"(?:\\.|[^"\\\n])*"'
    r"|'(?:\\.|[^'\\\n])*'",
    re.DOTALL,
)
# The Catch2 macros whose string is a message a person reads, not a datum the test handles.
REPORTING = re.compile(
    r"\b(?:TEST_CASE|SECTION|FAIL|FAIL_CHECK|SKIP|INFO|UNSCOPED_INFO|WARN|BENCHMARK"
    r"|BENCHMARK_ADVANCED|DYNAMIC_SECTION)\s*\(")

LOCALE_TREES = ("src/po/", "docs/i18n/fr/")
DATA_TREES = ("src/test/data/", "packaging/patterns/")
DATA_DOCUMENTATION = ("README.md", "LISEZMOI.md")
GENERATED = ("CHANGELOG.md",)


def excluded(path: str) -> bool:
    """Whether a tracked path is outside the rule; the module docstring says why for each."""
    if path.startswith(LOCALE_TREES) or path in GENERATED:
        return True
    if path.startswith(DATA_TREES):
        return pathlib.PurePosixPath(path).name not in DATA_DOCUMENTATION
    return False


def prose(line: str) -> str:
    """The line without what is not prose: inline code, URLs, link targets, quotations,
    alternations of words, options.

    A quotation between guillemets or curly quotes is what an English sentence says about a
    French word — a label of the French manual, a keyword of a fixture. An alternation — `le|la|les`
    — is a vocabulary, the very one a language check is made of.
    """
    for pattern in (INLINE_CODE, URL, LINK_TARGET, TYPOGRAPHIC_QUOTE, ALTERNATION, OPTION):
        line = pattern.sub(" ", line)
    return line


def is_french(line: str) -> bool:
    """Whether one line of text is French, by the vote of its function words."""
    if LOCALE_TAG.search(line):
        return False
    sentence = prose(line).strip()
    if len(sentence) < SHORTEST_SENTENCE:
        return False
    french = len(FRENCH.findall(sentence))
    return french >= LEAST_FRENCH_VOTES and french > len(ENGLISH.findall(sentence))


def cpp_voted_text(text: str) -> list[str]:
    """What is voted on in each line of C++: its comments, and the strings of a reporting macro.

    Every other string literal of C++ is data — the subtitle a test reads, the text a correction
    rewrites — and its language is the point of the test. Raw strings are always data.
    """
    count = text.count("\n") + 1
    comments = [""] * count
    strings = [""] * count
    code = [""] * count
    line = 0
    position = 0

    def spread(target: list[str], fragment: str) -> None:
        nonlocal line
        for index, piece in enumerate(fragment.split("\n")):
            target[line + index] += " " + piece

    for token in CPP_TOKEN.finditer(text):
        spread(code, text[position:token.start()])
        line += text.count("\n", position, token.start())
        found = token.group()
        if found.startswith("/"):
            spread(comments, found[2:-2] if found.startswith("/*") else found[2:])
        elif token.group("delimiter") is None and found.startswith('"'):
            strings[line] += " " + found[1:-1]
        line += found.count("\n")
        position = token.end()
    spread(code, text[position:])

    return [PLAIN_QUOTE.sub(" ", comment) + (literal if REPORTING.search(statement) else "")
            for comment, literal, statement in zip(comments, strings, code)]


def french_lines(path: str, text: str) -> list[int]:
    """The numbers of the French lines of a file.

    Markdown loses its fenced code blocks, C++ is read through `cpp_voted_text`, and every other
    file is voted on line by line, whole — a script's messages are prose as much as its comments.
    """
    if pathlib.PurePosixPath(path).suffix in CPP:
        return [number for number, line in enumerate(cpp_voted_text(text), start=1)
                if is_french(line)]
    found = []
    markdown = path.endswith(".md")
    fenced = False
    for number, line in enumerate(text.splitlines(), start=1):
        if markdown and FENCE.match(line):
            fenced = not fenced
            continue
        if fenced or (markdown and EXAMPLE_MARKER.match(line)):
            continue
        if is_french(line):
            found.append(number)
    return found


def read_text(path: pathlib.Path) -> str | None:
    """The text of a file, or None for a binary one."""
    data = path.read_bytes()
    if b"\0" in data:
        return None
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        return None


def git(root: pathlib.Path, *arguments: str) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "-C", str(root), *arguments], capture_output=True, text=True,
                          check=False)


def french_files(root: pathlib.Path) -> dict[str, list[int]]:
    """Every tracked file the rule applies to and that contains French, with its French lines."""
    listed = git(root, "ls-files", "-z")
    if listed.returncode != 0:
        raise SystemExit(f"{RED}✗{RESET} git ls-files failed in {root}: {listed.stderr.strip()}")
    found = {}
    for path in sorted(filter(None, listed.stdout.split("\0"))):
        if excluded(path) or path == ALLOWLIST:
            continue
        file = root / path
        if not file.is_file() or file.is_symlink():
            continue
        text = read_text(file)
        if text is not None and (lines := french_lines(path, text)):
            found[path] = lines
    return found


def parse_allowlist(text: str) -> list[str]:
    """The entries of an allowlist: one path per line; blank lines and # comments ignored."""
    return [line.strip() for line in text.splitlines()
            if line.strip() and not line.lstrip().startswith("#")]


def merge_base(root: pathlib.Path, base: str | None) -> tuple[str | None, str]:
    """The revision the list is compared with, or None and why there is none."""
    if base is not None:
        if git(root, "rev-parse", "--verify", "--quiet", f"{base}^{{commit}}").returncode != 0:
            return None, f"{base} is not a revision of this repository"
        return base, base
    if git(root, "rev-parse", "--verify", "--quiet", f"{UPSTREAM}^{{commit}}").returncode != 0:
        return None, f"{UPSTREAM} is not reachable"
    found = git(root, "merge-base", "HEAD", UPSTREAM)
    if found.returncode != 0:
        return None, f"HEAD and {UPSTREAM} have no merge base (shallow clone?)"
    revision = found.stdout.strip()
    return revision, f"the merge base with {UPSTREAM} ({revision[:10]})"


def check(root: pathlib.Path, base: str | None) -> tuple[list[tuple[str, str, str]], list[str]]:
    """The failures (kind, path, message) and the notes of one run over a repository."""
    failures: list[tuple[str, str, str]] = []
    notes: list[str] = []
    allowlist_file = root / ALLOWLIST
    text = allowlist_file.read_text(encoding="utf-8") if allowlist_file.is_file() else ""
    entries = parse_allowlist(text)

    if entries != sorted(set(entries)):
        failures.append(("UNSORTED", ALLOWLIST, "the entries must be sorted and unique"))

    found = french_files(root)
    allowed = set(entries)
    for path, lines in found.items():
        if path not in allowed:
            shown = ", ".join(map(str, lines[:5])) + (" …" if len(lines) > 5 else "")
            failures.append(("UNLISTED", path, f"French on line(s) {shown}"))
    for path in entries:
        if not (root / path).is_file():
            failures.append(("MISSING", path, f"no such file: remove the line from {ALLOWLIST}"))
        elif path not in found:
            failures.append(("STALE", path, f"no French left: remove the line from {ALLOWLIST}"))

    revision, described = merge_base(root, base)
    if revision is None:
        notes.append(f"growth not checked: {described}")
    else:
        shown = git(root, "show", f"{revision}:{ALLOWLIST}")
        if shown.returncode != 0:
            notes.append(f"growth not checked: {described} has no {ALLOWLIST}")
        else:
            before = set(parse_allowlist(shown.stdout))
            for path in entries:
                if path not in before:
                    failures.append(("GROWN", path,
                                     f"not on the list at {described}: an entry is never added"))
    return failures, notes


def report(failures: list[tuple[str, str, str]]) -> None:
    for kind, path, message in failures:
        print(f"{RED}✗{RESET} {kind:8}  {path}: {message}", file=sys.stderr)


def run(root: pathlib.Path, base: str | None) -> int:
    failures, notes = check(root, base)
    for note in notes:
        print(f"{YELLOW}!{RESET} {note}")
    if failures:
        report(failures)
        print(f"    the repository is written in English; French lives in {', '.join(LOCALE_TREES)}"
              f" and in the files of {ALLOWLIST} — ADR 0043, #660.", file=sys.stderr)
        return 1
    entries = parse_allowlist((root / ALLOWLIST).read_text(encoding="utf-8")) \
        if (root / ALLOWLIST).is_file() else []
    print(f"{GREEN}✓{RESET} no French outside the locales and the {len(entries)} file(s) still"
          f" to translate")
    return 0


def check_lines(cases: list[dict]) -> int:
    """The classifier against lines whose class is written next to them."""
    failures = 0
    for case in cases:
        if is_french(case["line"]) != (case["class"] == "fr"):
            failures += 1
            print(f"{RED}✗{RESET} line classified wrongly, expected {case['class']}: {case['line']}")
    return failures


def build_repository(where: pathlib.Path, case: dict) -> str | None:
    """A scratch repository for one case; returns the base revision when the case has one."""
    git(where, "init", "--quiet")
    git(where, "config", "user.email", "fixture@example.invalid")
    git(where, "config", "user.name", "fixture")
    base = None
    for step in ("base", "files"):
        for path, source in case.get(step, {}).items():
            target = where / path
            target.parent.mkdir(parents=True, exist_ok=True)
            if source is None:
                target.unlink(missing_ok=True)
            elif source.startswith("@"):
                shutil.copyfile(FIXTURES / source[1:], target)
            else:
                target.write_text(source, encoding="utf-8")
        git(where, "add", "--all")
        if step == "base" and "base" in case:
            git(where, "commit", "--quiet", "--allow-empty", "-m", "base")
            base = git(where, "rev-parse", "HEAD").stdout.strip()
    return base


def check_fixtures() -> int:
    """Replay the versioned cases: each rule fires once where it should, never where it should not."""
    lines = json.loads((FIXTURES / "lines.json").read_text(encoding="utf-8"))
    cases = json.loads((FIXTURES / "cases.json").read_text(encoding="utf-8"))
    failures = check_lines(lines)
    for case in cases:
        with tempfile.TemporaryDirectory() as scratch:
            where = pathlib.Path(scratch)
            base = build_repository(where, case)
            found, notes = check(where, base)
        kinds = sorted(kind for kind, _, _ in found)
        expected = sorted(case.get("expect", []))
        skipped = any(note.startswith("growth not checked") for note in notes)
        if kinds == expected and skipped == case.get("growth_skipped", base is None):
            print(f"{GREEN}✓{RESET} {case['name']}")
        else:
            failures += 1
            print(f"{RED}✗{RESET} {case['name']}: expected {expected}, got {kinds}; notes: {notes}")
            report(found)
    total = len(lines) + len(cases)
    if failures:
        print(f"\n{RED}✗{RESET} {failures} of {total} language case(s) do not behave as written")
        return 1
    print(f"{GREEN}✓{RESET} {total} language case(s) behave as written")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=pathlib.Path, default=REPO_ROOT,
                        help="the git work tree to check (default: this repository)")
    parser.add_argument("--base", help=f"the revision whose {ALLOWLIST} the list may not outgrow"
                        f" (default: the merge base of HEAD with {UPSTREAM})")
    parser.add_argument("--list", action="store_true",
                        help="print every file that contains French, with its count of French lines")
    parser.add_argument("--check-fixtures", action="store_true",
                        help="replay the versioned cases instead of checking the repository")
    args = parser.parse_args()

    if args.check_fixtures:
        return check_fixtures()
    if args.list:
        for path, lines in french_files(args.root.resolve()).items():
            print(f"{len(lines):6}  {path}")
        return 0
    return run(args.root.resolve(), args.base)


if __name__ == "__main__":
    sys.exit(main())
