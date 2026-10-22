#!/usr/bin/env python3
"""Refuse a user-visible string literal that escapes translation — a ratchet down to zero.

Phase 15 translates both surfaces of the program (spec 15, D10, issue #661). A literal shown to
the user has to pass through the translation API — `translate`, `translateIn`, `translatePlural`
of `core/i18n/messages.hpp`, or Qt's `tr` — or no catalogue can ever translate it. This check
finds the literals that do not, counts them per file, and holds the count to a **ratchet**
that can only descend: a conversion issue lowers its files' counts, nothing raises one, and
the target is zero before the phase closes.

## What is read

Every C++ file (`.cpp`, `.hpp`, `.h`, `.cc`) that `git ls-files` names under `src/lib/` and
`src/exe/`. Tests, benchmarks and tools are not read: their strings are not shown to a user.
Files git does not track are never read.

## What "user-visible" means here

A scanner cannot know where a string ends up; it can know what a string looks like and what
receives it. The rule is pragmatic and written once:

1. **Every string literal is a candidate** — plain, prefixed or raw, in code, never in a
   comment or a preprocessor line (`#include "…"`). Adjacent literals (`"a" "b"`) are one
   string, counted once.
2. **A candidate is prose when it has a word**: a run of at least two letters. `""`, `", "`,
   `"%1"`, `"\n"` and `":"` have none and are never counted.
3. **A literal is translated when the call that directly receives it is the translation API**
   (`translate`, `translateIn`, `translatePlural`, `deferred`, `tr`, `QT_TR_NOOP`, `QT_TRANSLATE_NOOP`).
   Wrappers that only change the type — `QStringLiteral`, `QLatin1String`, `QString`,
   `std::string` and the like — are looked through, so `tr(QStringLiteral("…"))` would count as
   wrapped and `setText(QStringLiteral("…"))` is judged by `setText`.
4. **Everything else that is prose counts — unless the exemption table says why not.** The
   table is `.translation-exemptions`, at the root of the repository: the one place where
   "this literal is not for a reader" is written. Its sections are categories, each with the
   reason it is not translated (D3: identifiers, format names, option names, JSON keys,
   shortcut sequences, log lines, …), and each entry is one of:

       text      <regex>          the literal's whole text matches it
       call      <name>           the call that receives the literal is this one
       statement <regex>          the statement holding the literal, the literal written
                                  `@`, matches it (`.set(` of a JSON record, a comparison)
       file      <glob> [<regex>] a literal of a matching file, every one or those whose text
                                  matches: a vocabulary that is data in that file only

   The first entry that matches, in the order of the table, names the category.

So the count over-reports rather than under-reports: a literal the rules cannot place is
counted, and a reviewer either wraps it or adds the exemption that explains it. That keeps the
mistake visible — a missed message would be invisible.

## The ratchet

`.untranslated-ratchet` holds, one line each and sorted by path, the count of each file that
has untranslated literals, after a `total` line. The check fails when:

    RISEN         a file has more untranslated literals than its line records;
    NEW           a file that has no line has untranslated literals;
    STALE         a line records more than the file has — lower it (`--record`);
    UNSORTED      the lines are not sorted by path, or a path appears twice;
    TOTAL         the `total` line is not the sum of the lines;
    GROWN         the recorded total is above the total at the merge base with
                  `origin/main` — the ratchet only descends;
    EXEMPTIONS    the exemption table does not read.

**Growth is judged on the total, and the rest on each file.** A file's count may not rise
above its line, so a literal added anywhere is refused where it is added; but a refactoring
that moves literals from one file to another rewrites two lines and leaves the total where it
was, and that is not growth.

`--record` rewrites the ratchet from what the tree has. **It cannot raise it**: what it writes is
then judged against the merge base, as the French allowlist is (#660). When no base is
reachable — a shallow clone, an archive — that comparison is skipped and says so. `--list`
prints every untranslated literal, file by file, which is what a conversion issue works from.

## Self-test

`--check-fixtures` replays the versioned cases of `src/test/data/translatable/`: C++ files whose
literals are marked with what the scanner must say about them — every category of the table
must be proven by one of them — and small repositories built in a scratch directory where each
rule of the ratchet must fire once, or not at all.

Exit status: 0 when the tree holds the ratchet, 1 otherwise.
"""

from __future__ import annotations

import argparse
import fnmatch
import json
import pathlib
import re
import shutil
import sys
import tempfile
from dataclasses import dataclass

from revisions import UPSTREAM, git, merge_base

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
EXEMPTIONS = ".translation-exemptions"
RATCHET = ".untranslated-ratchet"
FIXTURES = REPO_ROOT / "src/test/data/translatable"
SCANNED_TREES = ("src/lib/", "src/exe/")
CPP = (".cpp", ".hpp", ".h", ".cc")

GREEN, RED, YELLOW, RESET = "\033[32m", "\033[31m", "\033[33m", "\033[0m"

# The calls whose literal argument is extracted into the catalogue.
TRANSLATION_API = frozenset({"translate", "translateIn", "translatePlural", "deferred", "tr",
                             "QT_TR_NOOP", "QT_TRANSLATE_NOOP"})
# Calls that only change a string's type: the call that receives them decides.
TRANSPARENT = frozenset({"{", "QStringLiteral", "QLatin1String", "QLatin1StringView", "QString",
                         "QStringView", "QByteArray", "QByteArrayLiteral", "QLatin1Char",
                         "QUtf8StringView", "fromUtf8", "fromLatin1", "fromStdString", "string",
                         "string_view", "u8string", "u8string_view", "QChar"})

# A comment, a raw string, a string or a character literal of C++, in the order they open.
CPP_TOKEN = re.compile(
    r"//[^\n]*|/\*.*?\*/"
    r'|(?<![\w])(?:u8|[uUL])?R"(?P<delimiter>[^(\s"\\]{0,16})\((?P<raw>.*?)\)(?P=delimiter)"'
    r'|(?<![\w])(?:u8|[uUL])?"(?P<plain>(?:\\.|[^"\\\n])*)"'
    r"|'(?:\\.|[^'\\\n])*'",
    re.DOTALL,
)
WORD = re.compile(r"[^\W\d_]{2,}")
IDENTIFIER_BEFORE = re.compile(r"([A-Za-z_]\w*)\s*(?:<[^<>()]*>)?\s*$")
PREPROCESSOR = re.compile(r"^[ \t]*#", re.MULTILINE)
KINDS = ("text", "call", "statement", "file")


@dataclass(frozen=True)
class Literal:
    line: int
    text: str
    call: str        # the call that receives it, wrappers looked through; "" outside any call
    statement: str   # the code of its statement on one line, the literal written `@`


@dataclass
class Exemption:
    category: str
    kind: str
    pattern: str
    compiled: re.Pattern | None = None   # the regex of `text` and `statement`, or of `file`


@dataclass
class Exemptions:
    reasons: dict[str, str]
    entries: list[Exemption]

    def exempt(self, path: str, literal: Literal) -> str | None:
        """The category that exempts a literal of a file, or None."""
        for entry in self.entries:
            if entry.kind == "file":
                glob = entry.pattern.split(None, 1)[0]
                if fnmatch.fnmatchcase(path, glob) and (
                        entry.compiled is None or entry.compiled.fullmatch(literal.text)):
                    return entry.category
                continue
            if entry.kind == "text" and entry.compiled.fullmatch(literal.text):
                return entry.category
            if entry.kind == "call" and literal.call == entry.pattern:
                return entry.category
            if entry.kind == "statement" and entry.compiled.search(literal.statement):
                return entry.category
        return None


def parse_exemptions(text: str) -> Exemptions:
    """The table: `[category] reason` opens a section; `kind pattern` lines fill it."""
    reasons: dict[str, str] = {}
    entries: list[Exemption] = []
    category = None
    for number, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        section = re.fullmatch(r"\[([a-z][a-z-]*)\]\s+(\S.*)", line)
        if section:
            category = section.group(1)
            if category in reasons:
                raise ValueError(f"line {number}: category [{category}] opened twice")
            reasons[category] = section.group(2)
            continue
        kind, _, pattern = line.partition(" ")
        pattern = pattern.strip()
        if category is None:
            raise ValueError(f"line {number}: an entry before any [category]")
        if kind not in KINDS or not pattern:
            raise ValueError(f"line {number}: expected one of {', '.join(KINDS)} and a pattern")
        compiled = None
        expression: str | None = pattern
        if kind == "file":   # a glob, then an optional regex
            parts = pattern.split(None, 1)
            expression = parts[1] if len(parts) == 2 else None
        if expression is not None:
            try:
                compiled = re.compile(expression)
            except re.error as error:
                raise ValueError(f"line {number}: {pattern!r} is not a regex: {error}") from None
        entries.append(Exemption(category, kind, pattern, compiled))
    for category in reasons:
        if not any(entry.category == category for entry in entries):
            raise ValueError(f"category [{category}] has no entry")
    return Exemptions(reasons, entries)


def blank(text: str, tokens: list[re.Match]) -> str:
    """The code of a text alone: comments, literals and preprocessor lines turned into spaces,
    newlines kept, and each string literal reduced to its two quotes, so that positions match."""
    code = list(text)
    for token in tokens:
        for index in range(token.start(), token.end()):
            if code[index] != "\n":
                code[index] = " "
        if token.group().startswith(("/", "'")):
            continue
        code[token.start()] = code[token.end() - 1] = '"'
    for match in PREPROCESSOR.finditer(text):
        end = text.find("\n", match.start())
        for index in range(match.start(), len(text) if end == -1 else end):
            code[index] = " "
    return "".join(code)


def literals_of(text: str) -> list[Literal]:
    """Every string literal of a C++ text, with the call that receives it and its statement."""
    tokens = list(CPP_TOKEN.finditer(text))
    code = blank(text, tokens)
    strings = [token for token in tokens if not token.group().startswith(("/", "'"))
               and code[token.start()] == '"']

    # The call that receives each literal: one pass over the code with a stack of brackets,
    # each '(' remembering the name before it.
    receivers: dict[int, str] = {}
    starts = {token.start() for token in strings}
    stack: list[str] = []
    for index, character in enumerate(code):
        if index in starts:
            receivers[index] = next((callee for callee in reversed(stack)
                                     if callee not in TRANSPARENT), "")
        if character == "(":
            name = IDENTIFIER_BEFORE.search(code[max(0, index - 120):index])
            stack.append(name.group(1) if name else "")
        elif character == "{":
            stack.append("{")     # an initializer or a block: the call around it receives
        elif character == "[":
            stack.append("")
        elif character in ")]}" and stack:
            stack.pop()

    found: list[Literal] = []
    run: list[re.Match] = []

    def flush() -> None:
        if not run:
            return
        first, last = run[0], run[-1]
        before = max(code.rfind(mark, 0, first.start()) for mark in ";{}")
        after = min((found_at for mark in ";{}"
                     if (found_at := code.find(mark, last.end())) != -1), default=len(code))
        statement = code[before + 1:first.start()] + "@" + code[last.end():after]
        pieces = [token.group("raw") if token.group("delimiter") is not None
                  else token.group("plain") for token in run]
        found.append(Literal(text.count("\n", 0, first.start()) + 1, "".join(pieces),
                             receivers[first.start()], " ".join(statement.split())))
        run.clear()

    for token in strings:
        if run and code[run[-1].end():token.start()].strip():
            flush()
        run.append(token)
    flush()
    return found


def verdict(path: str, literal: Literal, exemptions: Exemptions) -> str:
    """What the check says of one literal: `not-prose`, `translated`, `exempt:<category>` or
    `counted` — the last being the only one the ratchet counts."""
    if not WORD.search(literal.text):
        return "not-prose"
    if literal.call in TRANSLATION_API:
        return "translated"
    category = exemptions.exempt(path, literal)
    return f"exempt:{category}" if category else "counted"


def untranslated(path: str, text: str, exemptions: Exemptions) -> list[Literal]:
    """The literals of a file that are prose, not translated, and not exempt."""
    return [literal for literal in literals_of(text)
            if verdict(path, literal, exemptions) == "counted"]


def scanned(path: str) -> bool:
    return path.startswith(SCANNED_TREES) and pathlib.PurePosixPath(path).suffix in CPP


def untranslated_files(root: pathlib.Path,
                       exemptions: Exemptions) -> dict[str, list[Literal]]:
    """Every scanned file that has untranslated literals, with them."""
    listed = git(root, "ls-files", "-z", "--", *SCANNED_TREES)
    if listed.returncode != 0:
        raise SystemExit(f"{RED}✗{RESET} git ls-files failed in {root}: {listed.stderr.strip()}")
    found = {}
    for path in sorted(filter(None, listed.stdout.split("\0"))):
        file = root / path
        if not scanned(path) or not file.is_file() or file.is_symlink():
            continue
        if literals := untranslated(path, file.read_text(encoding="utf-8", errors="replace"),
                                    exemptions):
            found[path] = literals
    return found


def parse_ratchet(text: str) -> tuple[list[tuple[str, int]], int | None, list[str]]:
    """The (path, count) lines of a ratchet, its `total`, and the lines that do not read."""
    entries: list[tuple[str, int]] = []
    total = None
    bad = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if found := re.fullmatch(r"total\s+(\d+)", stripped):
            total = int(found.group(1))
        elif found := re.fullmatch(r"(\d+)\s+(\S+)", stripped):
            entries.append((found.group(2), int(found.group(1))))
        else:
            bad.append(stripped)
    return entries, total, bad


RATCHET_HEADER = """\
# The untranslated user-visible string literals of each file — spec 15, D10, issue #661.
#
# Written by `src/scripts/check-translatable-strings.py --record` (`make untranslated-record`),
# held by the `untranslated` step of the gate. A count can only go down: a conversion issue
# wraps literals in the translation API and lowers its files' lines; nothing raises one, and
# the total may never exceed the total at the merge base with origin/main.
#
# The target is zero before phase 15 closes.

"""


def ratchet_text(counts: dict[str, int]) -> str:
    lines = [RATCHET_HEADER, f"total {sum(counts.values())}\n"]
    lines += [f"{count:5}  {path}\n" for path, count in sorted(counts.items())]
    return "".join(lines)


def load_exemptions(root: pathlib.Path) -> Exemptions:
    table = root / EXEMPTIONS
    if not table.is_file():
        raise ValueError(f"{EXEMPTIONS} does not exist")
    return parse_exemptions(table.read_text(encoding="utf-8"))


def check(root: pathlib.Path,
          base: str | None) -> tuple[list[tuple[str, str, str]], list[str], dict[str, int]]:
    """The failures (kind, path, message), the notes and the actual counts of one run."""
    failures: list[tuple[str, str, str]] = []
    notes: list[str] = []
    try:
        exemptions = load_exemptions(root)
    except ValueError as error:
        notes.append("growth not checked: nothing is counted without the exemption table")
        return [("EXEMPTIONS", EXEMPTIONS, str(error))], notes, {}

    counts = {path: len(found) for path, found in untranslated_files(root, exemptions).items()}
    ratchet = root / RATCHET
    entries, total, bad = parse_ratchet(
        ratchet.read_text(encoding="utf-8") if ratchet.is_file() else "")
    for line in bad:
        failures.append(("UNSORTED", RATCHET, f"a line that does not read: {line!r}"))
    paths = [path for path, _ in entries]
    if paths != sorted(set(paths)):
        failures.append(("UNSORTED", RATCHET, "the lines must be sorted by path, each path once"))
    recorded = dict(entries)
    if total != sum(recorded.values()):
        said = "there is no total line" if total is None else f"the total line says {total}"
        failures.append(("TOTAL", RATCHET, f"{said}, the lines add up to"
                                           f" {sum(recorded.values())}"))

    for path, count in counts.items():
        if path not in recorded:
            failures.append(("NEW", path, f"{count} untranslated literal(s), and no line in"
                                          f" {RATCHET}: wrap them in the translation API"))
        elif count > recorded[path]:
            failures.append(("RISEN", path, f"{count} untranslated literal(s), {recorded[path]}"
                                            f" recorded: wrap the new one(s)"))
    for path, count in recorded.items():
        if count > counts.get(path, 0):
            failures.append(("STALE", path, f"{counts.get(path, 0)} untranslated literal(s),"
                                            f" {count} recorded: lower the line (--record)"))

    revision, described = merge_base(root, base)
    if revision is None:
        notes.append(f"growth not checked: {described}")
    else:
        shown = git(root, "show", f"{revision}:{RATCHET}")
        if shown.returncode != 0:
            notes.append(f"growth not checked: {described} has no {RATCHET}")
        else:
            before = sum(count for _, count in parse_ratchet(shown.stdout)[0])
            if sum(recorded.values()) > before:
                failures.append(("GROWN", RATCHET,
                                 f"records {sum(recorded.values())}, {before} at {described}:"
                                 f" the ratchet only descends"))
    return failures, notes, counts


def report(failures: list[tuple[str, str, str]]) -> None:
    for kind, path, message in failures:
        print(f"{RED}✗{RESET} {kind:10}  {path}: {message}", file=sys.stderr)


def run(root: pathlib.Path, base: str | None) -> int:
    failures, notes, counts = check(root, base)
    for note in notes:
        print(f"{YELLOW}!{RESET} {note}")
    if failures:
        report(failures)
        print(f"    a literal shown to the user goes through translate, translateIn,"
              f" translatePlural or tr; one that is not for a reader goes in {EXEMPTIONS}"
              f" with its reason — spec 15, D10, #661.", file=sys.stderr)
        return 1
    total = sum(counts.values())
    print(f"{GREEN}✓{RESET} {total} untranslated literal(s) in {len(counts)} file(s), as"
          f" {RATCHET} records — the target is zero before phase 15 closes")
    return 0


def record(root: pathlib.Path) -> int:
    try:
        exemptions = load_exemptions(root)
    except ValueError as error:
        print(f"{RED}✗{RESET} {EXEMPTIONS}: {error}", file=sys.stderr)
        return 1
    counts = {path: len(found) for path, found in untranslated_files(root, exemptions).items()}
    (root / RATCHET).write_text(ratchet_text(counts), encoding="utf-8")
    print(f"{GREEN}✓{RESET} {RATCHET}: {sum(counts.values())} untranslated literal(s) in"
          f" {len(counts)} file(s)")
    return 0


def list_literals(root: pathlib.Path) -> int:
    try:
        exemptions = load_exemptions(root)
    except ValueError as error:
        print(f"{RED}✗{RESET} {EXEMPTIONS}: {error}", file=sys.stderr)
        return 1
    for path, found in untranslated_files(root, exemptions).items():
        for literal in found:
            print(f"{path}:{literal.line}: {json.dumps(literal.text, ensure_ascii=False)}")
    return 0


MARKER = re.compile(r"//\s*expect:\s*(\S+)")


def check_literals(name: str, exemptions: Exemptions) -> tuple[int, set[str]]:
    """One annotated C++ fixture: each literal's verdict against the marker on its line.

    A line marked `// expect: <verdict>` holds literals that must all get that verdict; one
    marked `// expect: none` must hold none — a quotation in a comment, an `#include`.
    A line `// path: <path>` gives the path the literals after it are judged under, so that
    one fixture can prove the exemptions that hold in one file only.
    """
    text = (FIXTURES / name).read_text(encoding="utf-8")
    lines = text.splitlines()
    paths = {number: found.group(1) for number, line in enumerate(lines, start=1)
             if (found := re.match(r"\s*//\s*path:\s*(\S+)", line))}
    expected = {number: found.group(1) for number, line in enumerate(lines, start=1)
                if (found := MARKER.search(line))}
    failures = 0
    seen = set()
    verdicts = set()
    for literal in literals_of(text):
        if literal.line not in expected:
            continue
        seen.add(literal.line)
        path = next((paths[number] for number in sorted(paths, reverse=True)
                     if number < literal.line), "src/lib/subedit/fixture.cpp")
        said = verdict(path, literal, exemptions)
        verdicts.add(said)
        if said != expected[literal.line]:
            failures += 1
            print(f"{RED}✗{RESET} {name}:{literal.line}: {literal.text!r} is {said},"
                  f" expected {expected[literal.line]}")
    for number, wanted in sorted(expected.items()):
        if (wanted == "none") == (number in seen):
            failures += 1
            print(f"{RED}✗{RESET} {name}:{number}: expected {wanted}, and the line has"
                  f" {'a literal' if number in seen else 'no literal'}")
    if not failures:
        print(f"{GREEN}✓{RESET} {name}: {len(expected)} line(s) judged as marked")
    return failures, verdicts


def build_repository(where: pathlib.Path, case: dict) -> str | None:
    """A scratch repository for one case; returns the base revision when the case has one.

    The repository's own exemption table is copied in first, unless the case writes one:
    the cases prove the ratchet, the annotated files prove the table.
    """
    git(where, "init", "--quiet")
    git(where, "config", "user.email", "fixture@example.invalid")
    git(where, "config", "user.name", "fixture")
    shutil.copyfile(REPO_ROOT / EXEMPTIONS, where / EXEMPTIONS)
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
    """Replay the versioned cases: the verdict of each marked literal, and each ratchet rule."""
    exemptions = load_exemptions(REPO_ROOT)
    files = sorted(path.name for path in FIXTURES.glob("*.cpp.txt"))
    failures = 0
    verdicts: set[str] = set()
    for name in files:
        failed, said = check_literals(name, exemptions)
        failures += failed
        verdicts |= said
    # Every category of the table is proven by at least one marked literal: an exemption that
    # nothing exercises is one nobody would see go wrong.
    for category in exemptions.reasons:
        if f"exempt:{category}" not in verdicts:
            failures += 1
            print(f"{RED}✗{RESET} [{category}] of {EXEMPTIONS}: no annotated literal proves it")
    cases = json.loads((FIXTURES / "cases.json").read_text(encoding="utf-8"))
    for case in cases:
        with tempfile.TemporaryDirectory() as scratch:
            where = pathlib.Path(scratch)
            base = build_repository(where, case)
            found, notes, _ = check(where, base)
        kinds = sorted(kind for kind, _, _ in found)
        expected = sorted(case.get("expect", []))
        skipped = any(note.startswith("growth not checked") for note in notes)
        if kinds == expected and skipped == case.get("growth_skipped", base is None):
            print(f"{GREEN}✓{RESET} {case['name']}")
        else:
            failures += 1
            print(f"{RED}✗{RESET} {case['name']}: expected {expected}, got {kinds}; notes: {notes}")
            report(found)
    if failures:
        print(f"\n{RED}✗{RESET} {failures} translatable-string case(s) do not behave as written")
        return 1
    print(f"{GREEN}✓{RESET} {len(files)} annotated file(s) and {len(cases)} ratchet case(s)"
          f" behave as written")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=pathlib.Path, default=REPO_ROOT,
                        help="the git work tree to check (default: this repository)")
    parser.add_argument("--base", help=f"the revision whose {RATCHET} total may not be exceeded"
                        f" (default: the merge base of HEAD with {UPSTREAM})")
    action = parser.add_mutually_exclusive_group()
    action.add_argument("--record", action="store_true",
                        help=f"rewrite {RATCHET} from what the tree has")
    action.add_argument("--list", action="store_true",
                        help="print every untranslated literal, file by file")
    action.add_argument("--check-fixtures", action="store_true",
                        help="replay the versioned cases instead of checking the repository")
    args = parser.parse_args()

    if args.check_fixtures:
        return check_fixtures()
    root = args.root.resolve()
    if args.record:
        return record(root)
    if args.list:
        return list_literals(root)
    return run(root, args.base)


if __name__ == "__main__":
    sys.exit(main())
