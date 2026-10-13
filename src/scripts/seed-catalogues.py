#!/usr/bin/env python3
"""Seed the catalogues of src/po/ from Gaupol's — issue #667, spec 15, D5.

Gaupol's twenty locales are GPL-3.0-or-later, as subedit is, and about a third of
our messages already have a translation there. This script carries them over:
for every Gaupol catalogue it writes `src/po/<lang>.po`, whose entries are **the
msgids of our template, in the template's order**, translated when Gaupol has
the same message and empty otherwise. It is a maintenance tool, run by hand
when the template or the upstream catalogues have moved; nothing of the gate,
the tests or the build runs it on the upstream files, which a fresh clone does
not have. `--check-fixtures` replays the versioned fixture of
`src/test/data/i18n/seed/` instead, and that is what the `fixtures` step runs.

    seed-catalogues.py [SOURCE] --revision HASH [--report FILE]
    seed-catalogues.py --check-fixtures

SOURCE is the `po/` directory of a Gaupol checkout, by default the read-only
clone `reference/gaupol/po`; it is only read. HASH is the upstream revision of
that checkout (`git -C reference/gaupol rev-parse HEAD`): it is recorded in
every header, and given rather than read so that the script never runs git in
the clone. `--report` writes the coverage table of `docs/mesures/`.

## When an entry is carried over

**Only on an exact match, and never a doubtful one.** A Gaupol entry translates
a template entry when, after the conversions below, its msgid (and its
msgid_plural) is our msgid; the translation is converted the same way.
Anything that cannot be converted without guessing leaves the entry empty.

- **Accelerators.** Gaupol marks a mnemonic with `_` (`__` is a literal
  underscore), we mark it with `&` (`&&` is a literal ampersand). A Gaupol text
  is first compared as it is — a literal `_` in a message that is not a label —
  then with `_X` read as a mnemonic: `_X` becomes `&X`, `__` becomes `_`, `&`
  becomes `&&`. In the second reading the mnemonic's position is not compared,
  only its presence: where the translator put it is their language's choice.
  A text with two mnemonics, or a `_` with nothing after it, is not converted.
- **Placeholders.** Gaupol's are Python's (`{}`, `{:d}`, `{name}`, `%s`,
  `%(name)s`, flagged `python-brace-format` or `python-format`); ours are
  positional (`%1`..`%9`, spec 15, D2). Anonymous fields are numbered in order,
  named and indexed ones by their first appearance in the msgid, and the
  translation must use the same fields — the same count of anonymous ones, the
  same set of named ones. A conversion (`{!r}`), a format spec other than `d`
  or `s`, two styles mixed, or a literal `%1` in the text leaves the entry
  empty.
- **Plurals** are carried when both sides are plural, when Gaupol's entry has
  exactly as many forms as its `Plural-Forms` header says, and when every form
  converts. A plural form may drop a named field (the singular often does), not
  an anonymous one, whose numbering would shift.
- **Fuzzy and obsolete entries are never carried**: a fuzzy translation is one
  its own translator did not stand behind, and installing it as clean is the
  one outcome this script must not allow. The entry stays empty.
- **A context** (`msgctxt`) matches only the same context; Gaupol has none, so
  an entry of ours with a context is never seeded.
- **Two Gaupol entries** that yield different translations for one of ours
  leave it empty.

## What is written, and what is refused

The header names our project and the language, takes Gaupol's `Plural-Forms`,
and records in its comment the origin (upstream URL, file, revision), the
Gaupol translators credited in that file, and the licence. There is no date:
the same inputs give the same bytes, and a second run changes nothing. The
entries are then laid out by `msgmerge --no-fuzzy-matching` against the
template, which is exactly what `make po` does to a seed — so `make po` on a
fresh seed changes nothing either.

**A seed is marked**, by the header line that starts with `MARKER` below. The
script overwrites a file only if it carries that line, and never writes a
language of `LINGUAS`: an installed language is reviewed, and a seed must not
replace it. That is why `fr` is skipped — `src/po/fr.po` is the shipped
French, reviewed by the maintainer (D5), and it is seeded by hand in #668
from the same table: run the script with `--output-dir` set to a scratch
directory to get the French seed without touching the tree. A translator who
takes a seed over deletes the marker line, and from then on the script leaves
the file alone and says so.

Seeds are committed and **not installed**: only the languages of
`src/po/LINGUAS` are compiled and installed (`cmake/Translations.cmake`). A
language joins that list when it is complete — no untranslated and no fuzzy
entry — and reviewed by someone who reads it (D5).
"""

from __future__ import annotations

import argparse
import dataclasses
import json
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
DEFAULT_SOURCE = REPO_ROOT / "reference/gaupol/po"
DEFAULT_TEMPLATE = REPO_ROOT / "src/po/subedit.pot"
DEFAULT_OUTPUT = REPO_ROOT / "src/po"
FIXTURE = REPO_ROOT / "src/test/data/i18n/seed"

UPSTREAM_URL = "https://github.com/otsaloma/gaupol"
MARKER = "# seed-catalogues.py: a seed, neither reviewed nor installed"
BUGS_ADDRESS = "https://github.com/Guyot-Bertrand/sub-edit/issues"

RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
RESET = "\033[0m"


class Ambiguous(Exception):
    """A text that cannot be converted without guessing: the entry stays empty."""


# ## Reading a catalogue


@dataclasses.dataclass
class Entry:
    context: str | None = None
    msgid: str | None = None
    plural: str | None = None
    msgstr: dict[int, str] = dataclasses.field(default_factory=dict)
    flags: set[str] = dataclasses.field(default_factory=set)
    comments: list[str] = dataclasses.field(default_factory=list)
    obsolete: bool = False


ESCAPES = {"n": "\n", "t": "\t", "r": "\r", '"': '"', "\\": "\\", "a": "\a", "b": "\b", "f": "\f", "v": "\v"}


def unquote(text: str, where: str) -> str:
    text = text.strip()
    if len(text) < 2 or text[0] != '"' or text[-1] != '"':
        raise ValueError(f"{where}: not a quoted string: {text}")
    body, out, i = text[1:-1], [], 0
    while i < len(body):
        c = body[i]
        if c == "\\":
            if i + 1 >= len(body) or body[i + 1] not in ESCAPES:
                raise ValueError(f"{where}: unknown escape in {text}")
            out.append(ESCAPES[body[i + 1]])
            i += 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def quote(text: str) -> str:
    out = text.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t")
    return f'"{out}"'


KEYWORD = re.compile(r"^(msgctxt|msgid_plural|msgid|msgstr(?:\[(\d+)\])?)\s+(\".*\")\s*$")


def parse_po(path: pathlib.Path) -> list[Entry]:
    """The entries of a `.po` or `.pot`, in order; the header is the first, with msgid ""."""
    entries: list[Entry] = []
    current = Entry()
    field: tuple[str, int] | None = None

    def flush() -> None:
        nonlocal current, field
        if current.msgid is not None:
            entries.append(current)
        current, field = Entry(), None

    for number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        where = f"{path.name}:{number}"
        line = raw.strip()
        if not line:
            flush()
            continue
        if line.startswith("#~"):
            current.obsolete = True
            line = line[2:].strip()
            # `#~|` is the previous msgid of an obsolete fuzzy entry, a comment like `#|`.
            if not line or line.startswith("#") or line.startswith("|"):
                continue
        elif line.startswith("#"):
            if current.msgid is not None:
                flush()
            if line.startswith("#,"):
                current.flags.update(flag.strip() for flag in line[2:].split(",") if flag.strip())
            current.comments.append(raw)
            continue
        if line.startswith('"'):
            if field is None:
                raise ValueError(f"{where}: a continuation with nothing to continue")
            value = unquote(line, where)
            kind, index = field
            if kind == "msgctxt":
                current.context = (current.context or "") + value
            elif kind == "msgid":
                current.msgid = (current.msgid or "") + value
            elif kind == "msgid_plural":
                current.plural = (current.plural or "") + value
            else:
                current.msgstr[index] += value
            continue
        match = KEYWORD.match(line)
        if not match:
            raise ValueError(f"{where}: cannot read: {raw}")
        keyword, index, value = match.group(1), match.group(2), unquote(match.group(3), where)
        if keyword == "msgctxt":
            if current.msgid is not None:
                flush()
            current.context, field = value, ("msgctxt", 0)
        elif keyword == "msgid":
            if current.msgid is not None:
                flush()
            current.msgid, field = value, ("msgid", 0)
        elif keyword == "msgid_plural":
            current.plural, field = value, ("msgid_plural", 0)
        else:
            position = int(index) if index is not None else 0
            current.msgstr[position] = value
            field = ("msgstr", position)
    flush()
    return entries


def header_fields(entry: Entry) -> dict[str, str]:
    fields = {}
    for line in entry.msgstr.get(0, "").splitlines():
        key, _, value = line.partition(":")
        if key:
            fields[key.strip()] = value.strip()
    return fields


def nplurals(plural_forms: str) -> int:
    match = re.search(r"nplurals\s*=\s*(\d+)", plural_forms)
    if not match:
        raise ValueError(f"no nplurals in Plural-Forms: {plural_forms}")
    return int(match.group(1))


# ## Converting Gaupol's text to ours

BRACE_FIELD = re.compile(r"^(?P<name>[^!:]*)(?:!(?P<conversion>[^:]*))?(?::(?P<spec>.*))?$")
PERCENT_FIELD = re.compile(r"%(?:\((?P<name>[A-Za-z_][A-Za-z0-9_]*)\))?(?P<type>[sd%])")
OUR_PLACEHOLDER = re.compile(r"%[1-9]")


def tokens(text: str, flags: set[str]) -> list[tuple[str, object]]:
    """Gaupol's text cut into ("text", str) and ("field", key) pieces; keys are ("auto",), ("index", i), ("name", n)."""
    pieces: list[tuple[str, object]] = []

    def literal(value: str) -> None:
        if pieces and pieces[-1][0] == "text":
            pieces[-1] = ("text", str(pieces[-1][1]) + value)
        else:
            pieces.append(("text", value))

    if "python-brace-format" in flags:
        i = 0
        while i < len(text):
            c = text[i]
            if c in "{}" and text[i + 1:i + 2] == c:
                literal(c)
                i += 2
            elif c == "}":
                raise Ambiguous("a lone closing brace")
            elif c == "{":
                end = text.find("}", i)
                if end < 0:
                    raise Ambiguous("an unclosed brace")
                match = BRACE_FIELD.match(text[i + 1:end])
                if not match or match.group("conversion") is not None:
                    raise Ambiguous("a conversion in a field")
                if (match.group("spec") or "") not in ("", "d", "s"):
                    raise Ambiguous("a format spec that changes the rendering")
                name = match.group("name")
                if name == "":
                    pieces.append(("field", ("auto",)))
                elif name.isdigit():
                    pieces.append(("field", ("index", int(name))))
                elif re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
                    pieces.append(("field", ("name", name)))
                else:
                    raise Ambiguous("a field that is neither a name nor an index")
                i = end + 1
            else:
                literal(c)
                i += 1
    elif "python-format" in flags:
        position = 0
        for match in PERCENT_FIELD.finditer(text):
            literal(text[position:match.start()])
            if match.group("type") == "%":
                literal("%")
            elif match.group("name"):
                pieces.append(("field", ("name", match.group("name"))))
            else:
                pieces.append(("field", ("auto",)))
            position = match.end()
        rest = text[position:]
        if "%" in rest.replace("%%", ""):
            raise Ambiguous("a percent sign that is not a placeholder")
        literal(rest)
    else:
        literal(text)

    for kind, value in pieces:
        if kind == "text" and OUR_PLACEHOLDER.search(str(value)):
            raise Ambiguous("a literal %1 that our substitution would read")
    return pieces


def numbering(pieces: list[tuple[str, object]]) -> dict[object, int]:
    """The number each field of a msgid becomes: anonymous ones in order, the others by first appearance."""
    keys = [value for kind, value in pieces if kind == "field"]
    if any(key == ("auto",) for key in keys) and any(key != ("auto",) for key in keys):
        raise Ambiguous("anonymous and named fields mixed")
    numbers: dict[object, int] = {}
    if keys and keys[0] == ("auto",):
        for n in range(len(keys)):
            numbers[("auto", n)] = n + 1
    else:
        for key in keys:
            numbers.setdefault(key, len(numbers) + 1)
    if len(numbers) > 9:
        raise Ambiguous("more than nine placeholders")
    return numbers


def fields_of(pieces: list[tuple[str, object]]) -> list[object]:
    return [value for kind, value in pieces if kind == "field"]


def convert_text(segment: str, mnemonic: bool) -> tuple[str, int]:
    """One literal segment in our syntax, and the number of mnemonics it holds."""
    if not mnemonic:
        return segment, 0
    out, count, i = [], 0, 0
    while i < len(segment):
        c = segment[i]
        if c == "_":
            if segment[i + 1:i + 2] == "_":
                out.append("_")
                i += 2
                continue
            following = segment[i + 1:i + 2]
            if not following or following.isspace():
                raise Ambiguous("an underscore with nothing to mark")
            out.append("&")
            count += 1
            i += 1
            continue
        out.append("&&" if c == "&" else c)
        i += 1
    return "".join(out), count


def render(pieces: list[tuple[str, object]], numbers: dict[object, int], mnemonic: bool,
           exact_fields: bool) -> tuple[str, int]:
    """Gaupol's pieces written in our syntax, with the mnemonic count; the fields must be the msgid's."""
    out, count, auto = [], 0, 0
    keys = fields_of(pieces)
    auto_wanted = sum(1 for key in numbers if key[0] == "auto")
    if any(key == ("auto",) for key in keys) or auto_wanted:
        if any(key != ("auto",) for key in keys) or len(keys) != auto_wanted:
            raise Ambiguous("not the same anonymous fields as the msgid")
    else:
        wanted, used = set(numbers), set(keys)
        if not used <= wanted or (exact_fields and used != wanted):
            raise Ambiguous("not the same named fields as the msgid")
    for kind, value in pieces:
        if kind == "text":
            text, marks = convert_text(str(value), mnemonic)
            out.append(text)
            count += marks
        elif value == ("auto",):
            out.append(f"%{numbers[('auto', auto)]}")
            auto += 1
        else:
            out.append(f"%{numbers[value]}")
    if count > 1:
        raise Ambiguous("two mnemonics in one text")
    return "".join(out), count


def accelerator_key(text: str) -> tuple[str, int]:
    """Our text without its accelerator, and how many it had: `&X` → X, `&&` → &."""
    out, count, i = [], 0, 0
    while i < len(text):
        c = text[i]
        if c == "&" and text[i + 1:i + 2] == "&":
            out.append("&")
            i += 2
        elif c == "&" and text[i + 1:i + 2] and not text[i + 1].isspace():
            count += 1
            i += 1
        else:
            out.append(c)
            i += 1
    return "".join(out), count


@dataclasses.dataclass(frozen=True)
class Candidate:
    msgid: str
    plural: str | None
    translations: tuple[str, ...]


def candidates(entry: Entry, forms: int) -> list[tuple[bool, Candidate]]:
    """The readings of one Gaupol entry: (mnemonic?, converted entry), those that convert."""
    found = []
    msgid_pieces = tokens(entry.msgid or "", entry.flags)
    plural_pieces = tokens(entry.plural, entry.flags) if entry.plural is not None else None
    reference = plural_pieces if plural_pieces is not None else msgid_pieces
    numbers = numbering(reference)
    if entry.plural is not None:
        strings = [entry.msgstr.get(n, "") for n in range(forms)]
        if sorted(entry.msgstr) != list(range(forms)) or not all(strings):
            raise Ambiguous("the plural forms do not match the header's count")
    else:
        strings = [entry.msgstr.get(0, "")]
        if not strings[0]:
            raise Ambiguous("untranslated")
    translated = [tokens(text, entry.flags) for text in strings]
    for mnemonic in (False, True):
        try:
            if plural_pieces is not None:
                msgid, _ = render(msgid_pieces, numbers, mnemonic, exact_fields=False)
                plural, _ = render(plural_pieces, numbers, mnemonic, exact_fields=True)
                outs = tuple(render(p, numbers, mnemonic, exact_fields=False)[0] for p in translated)
            else:
                msgid, _ = render(msgid_pieces, numbers, mnemonic, exact_fields=True)
                plural = None
                outs = (render(translated[0], numbers, mnemonic, exact_fields=True)[0],)
        except Ambiguous:
            continue
        found.append((mnemonic, Candidate(msgid, plural, outs)))
    return found


@dataclasses.dataclass
class Upstream:
    locale: str
    language: str
    plural_forms: str
    revision_date: str
    translators: list[str]
    exact: dict[tuple[str, str | None], set[tuple[str, ...]]]
    loose: dict[tuple[str, int, str | None], set[tuple[str, ...]]]


def read_upstream(path: pathlib.Path) -> Upstream:
    entries = parse_po(path)
    if not entries or entries[0].msgid != "":
        raise ValueError(f"{path.name}: no header")
    header = entries[0]
    fields = header_fields(header)
    plural_forms = fields.get("Plural-Forms", "nplurals=2; plural=(n != 1);")
    forms = nplurals(plural_forms)
    team = fields.get("Language-Team", "")
    language = re.sub(r"\s*\(https?://[^)]*\)\s*$", "", team).strip() or path.stem

    translators, in_list = [], False
    for line in header.comments:
        body = line[1:].strip() if line.startswith("#") and not line.startswith("#,") else None
        if body is None:
            continue
        if body == "Translators:":
            in_list = True
        elif in_list and body:
            translators.append(body)
        elif in_list:
            in_list = False

    exact: dict = {}
    loose: dict = {}
    for entry in entries[1:]:
        if entry.obsolete or "fuzzy" in entry.flags or entry.context is not None:
            continue
        try:
            readings = candidates(entry, forms)
        except Ambiguous:
            continue
        for mnemonic, candidate in readings:
            if not mnemonic:
                exact.setdefault((candidate.msgid, candidate.plural), set()).add(candidate.translations)
            else:
                key, count = accelerator_key(candidate.msgid)
                plural_key = accelerator_key(candidate.plural)[0] if candidate.plural is not None else None
                loose.setdefault((key, count, plural_key), set()).add(candidate.translations)
    revision_date = fields.get("PO-Revision-Date", "YEAR-MO-DA HO:MI+ZONE")
    return Upstream(path.stem, language, plural_forms, revision_date, translators, exact, loose)


def translation_for(entry: Entry, upstream: Upstream) -> tuple[str, ...] | None:
    if entry.context is not None:
        return None
    found = upstream.exact.get((entry.msgid, entry.plural))
    if not found:
        key, count = accelerator_key(entry.msgid or "")
        plural_key = accelerator_key(entry.plural)[0] if entry.plural is not None else None
        found = upstream.loose.get((key, count, plural_key))
    if not found or len(found) != 1:
        return None
    return next(iter(found))


# ## Writing a seed


def seed_text(template: list[Entry], upstream: Upstream, source_name: str, revision: str) -> tuple[str, int]:
    """The raw seed — header and translated entries only — and how many entries it translates."""
    credit = [f"# {name}" for name in upstream.translators] or ["# (no translator is named in that file)"]
    lines = [
        f"# {upstream.language} catalogue of subedit, seeded from Gaupol.",
        "#",
        f"# Origin: {UPSTREAM_URL}, file po/{source_name}, revision {revision}.",
        "# The translations are those of Gaupol's translators, credited in that file:",
        *credit,
        "# This file is distributed under the GNU General Public License, version 3 or",
        "# later (GPL-3.0-or-later), as Gaupol and subedit are.",
        "#",
        MARKER + " (spec 15, D5).",
        "# Delete the line above to take the file over: the script then leaves it alone.",
        'msgid ""',
        'msgstr ""',
        '"Project-Id-Version: subedit\\n"',
        f'"Report-Msgid-Bugs-To: {BUGS_ADDRESS}\\n"',
        '"POT-Creation-Date: YEAR-MO-DA HO:MI+ZONE\\n"',
        f'"PO-Revision-Date: {upstream.revision_date}\\n"',
        '"Last-Translator: \\n"',
        f'"Language-Team: {upstream.language}\\n"',
        f'"Language: {upstream.locale}\\n"',
        '"MIME-Version: 1.0\\n"',
        '"Content-Type: text/plain; charset=UTF-8\\n"',
        '"Content-Transfer-Encoding: 8bit\\n"',
        f'"Plural-Forms: {upstream.plural_forms}\\n"',
        "",
    ]
    translated = 0
    for entry in template[1:]:
        found = translation_for(entry, upstream)
        if found is None:
            continue
        translated += 1
        if entry.context is not None:
            lines.append(f"msgctxt {quote(entry.context)}")
        lines.append(f"msgid {quote(entry.msgid or '')}")
        if entry.plural is not None:
            lines.append(f"msgid_plural {quote(entry.plural)}")
            lines.extend(f"msgstr[{n}] {quote(text)}" for n, text in enumerate(found))
        else:
            lines.append(f"msgstr {quote(found[0])}")
        lines.append("")
    return "\n".join(lines), translated


def merge(raw: str, template: pathlib.Path) -> str:
    """Lay the raw seed out as `make po` does: every template entry, in the template's order."""
    with tempfile.TemporaryDirectory() as scratch:
        source = pathlib.Path(scratch) / "seed.po"
        source.write_text(raw, encoding="utf-8")
        result = subprocess.run(
            ["msgmerge", "--quiet", "--no-fuzzy-matching", "--output-file=-", str(source), str(template)],
            check=True, capture_output=True, text=True, encoding="utf-8")
    return result.stdout


def installed_languages(output: pathlib.Path) -> set[str]:
    linguas = output / "LINGUAS"
    if not linguas.exists():
        return set()
    return {line.strip() for line in linguas.read_text(encoding="utf-8").splitlines()
            if line.strip() and not line.strip().startswith("#")}


def is_seed(path: pathlib.Path) -> bool:
    return any(line.startswith(MARKER) for line in path.read_text(encoding="utf-8").splitlines())


@dataclasses.dataclass
class Outcome:
    locale: str
    language: str
    translated: int
    written: bool
    reason: str
    state: str


def seed(source: pathlib.Path, template_path: pathlib.Path, output: pathlib.Path, revision: str,
         quiet: bool = False) -> tuple[list[Outcome], int]:
    template = parse_po(template_path)
    if not template or template[0].msgid != "":
        raise ValueError(f"{template_path}: no header")
    installed = installed_languages(output)
    outcomes = []
    for path in sorted(source.glob("*.po")):
        upstream = read_upstream(path)
        raw, translated = seed_text(template, upstream, path.name, revision)
        target = output / path.name
        if upstream.locale in installed:
            reason = "skipped: an installed language (LINGUAS) is never overwritten by a seed"
            written, state = False, "not written: installed (`LINGUAS`)"
        elif target.exists() and not is_seed(target):
            reason = "skipped: the file is not a seed (its marker line is gone), a translator owns it"
            written, state = False, "not written: taken over by a translator"
        else:
            text = merge(raw, template_path)
            if not target.exists() or target.read_text(encoding="utf-8") != text:
                target.write_text(text, encoding="utf-8")
            reason, written, state = "written", True, "written"
        outcomes.append(Outcome(upstream.locale, upstream.language, translated, written, reason, state))
        if not quiet:
            colour = GREEN if written else YELLOW
            mark = "✓" if written else "-"
            print(f"{colour}{mark}{RESET} {upstream.locale}: {translated} entries translated, {reason}")
    return outcomes, len(template) - 1


def report(outcomes: list[Outcome], size: int, revision: str) -> str:
    def share(count: int) -> str:
        return f"{round(100 * count / size)} %" if size else "–"

    written = [o for o in outcomes if o.written]
    total = sum(o.translated for o in written)
    lines = [
        "# Catalogue seeds",
        "",
        "Generated by `src/scripts/seed-catalogues.py --report` (issue #667, spec 15, D5); do not edit by hand.",
        "",
        f"How many of the template's **{size} entries** each catalogue seeded from Gaupol already translates,",
        f"from {UPSTREAM_URL} at revision `{revision}`.",
        "Only exact matches are carried, and fuzzy entries never are. The seeds are committed and not",
        "installed: a language ships when it is complete and reviewed, and joins `src/po/LINGUAS` then.",
        "A language that is not written is still counted, for the translator who will start from it.",
        "",
        "| Locale | Language | Translated | Share | Seed |",
        "| :----- | :------- | ---------: | ----: | :--- |",
    ]
    for o in outcomes:
        lines.append(f"| `{o.locale}` | {o.language} | {o.translated} | {share(o.translated)} | {o.state} |")
    lines.append("")
    if written:
        def named(count: int) -> str:
            return ", ".join(f"`{o.locale}`" for o in written if o.translated == count)

        most, least = max(o.translated for o in written), min(o.translated for o in written)
        lines.append(f"**{len(written)} seeds written, {total} entries translated in all**: the most by "
                     f"{named(most)} ({most}, {share(most)}), the fewest by {named(least)} ({least}, {share(least)}).")
    else:
        lines.append("No seed written.")
    lines.append("")
    return "\n".join(lines)


# ## The versioned fixture


def check_fixtures() -> int:
    """Replay src/test/data/i18n/seed/: the expected seeds, the skips, determinism, idempotence."""
    cases = json.loads((FIXTURE / "cases.json").read_text(encoding="utf-8"))
    revision = cases["revision"]
    failures = []

    def expect(condition: bool, what: str) -> None:
        print(f"{GREEN}✓{RESET} {what}" if condition else f"{RED}✗{RESET} {what}")
        if not condition:
            failures.append(what)

    with tempfile.TemporaryDirectory() as scratch:
        first, second = pathlib.Path(scratch) / "first", pathlib.Path(scratch) / "second"
        for directory in (first, second):
            shutil.copytree(FIXTURE / "po", directory)
        template = FIXTURE / "template.pot"
        untouched = {name: (first / name).read_bytes() for name in cases["untouched"]}

        outcomes, size = seed(FIXTURE / "gaupol", template, first, revision, quiet=True)
        (first / "report.md").write_text(report(outcomes, size, revision), encoding="utf-8")
        for name in sorted(p.name for p in (FIXTURE / "expected").iterdir()):
            expect((first / name).read_bytes() == (FIXTURE / "expected" / name).read_bytes(),
                   f"{name} is the expected file, byte for byte")
        for name, content in untouched.items():
            expect((first / name).read_bytes() == content, f"{name} is left alone: {cases['untouched'][name]}")

        for locale, entries in cases["entries"].items():
            seeded = {(e.context, e.msgid): e for e in parse_po(first / f"{locale}.po")}
            for msgid, wanted in entries.items():
                found = seeded.get((None, msgid))
                if wanted is None:
                    expect(found is None, f"{locale}: {msgid!r} is absent, as the template does not have it")
                    continue
                got = None if found is None else [found.msgstr[n] for n in sorted(found.msgstr)]
                wanted = wanted if isinstance(wanted, list) else [wanted]
                expect(got == wanted and found is not None and "fuzzy" not in found.flags,
                       f"{locale}: {msgid!r} → {wanted!r}")

        for locale in cases["seeded"]:
            text = (first / f"{locale}.po").read_text(encoding="utf-8")
            for needle in (UPSTREAM_URL, f"revision {revision}", "GPL-3.0-or-later", MARKER):
                expect(needle in text, f"{locale}: the header records «{needle}»")
            result = subprocess.run(["msgfmt", "--check", "--output-file=/dev/null", str(first / f"{locale}.po")],
                                    capture_output=True, text=True)
            expect(result.returncode == 0, f"{locale}: msgfmt --check accepts the seed {result.stderr.strip()}")

        before = {p.name: p.read_bytes() for p in sorted(first.iterdir())}
        seed(FIXTURE / "gaupol", template, first, revision, quiet=True)
        (first / "report.md").write_text(report(outcomes, size, revision), encoding="utf-8")
        after = {p.name: p.read_bytes() for p in sorted(first.iterdir())}
        expect(before == after, "a second run changes nothing (idempotent)")

        outcomes, _ = seed(FIXTURE / "gaupol", template, second, revision, quiet=True)
        (second / "report.md").write_text(report(outcomes, size, revision), encoding="utf-8")
        expect(after == {p.name: p.read_bytes() for p in sorted(second.iterdir())},
               "a run from the same inputs elsewhere writes the same bytes (deterministic)")

        for locale in cases["seeded"]:
            path = first / f"{locale}.po"
            content = path.read_bytes()
            subprocess.run(["msgmerge", "--update", "--backup=none", "--quiet", "--no-fuzzy-matching",
                            str(path), str(template)], check=True)
            expect(path.read_bytes() == content, f"{locale}: the merge of make po leaves a fresh seed unchanged")

    if failures:
        print(f"\n{RED}✗{RESET} {len(failures)} expectation(s) of the seed fixture fail")
        return 1
    print(f"{GREEN}✓{RESET} the seed fixture behaves as written")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", nargs="?", type=pathlib.Path, default=DEFAULT_SOURCE,
                        help="the po/ directory of a Gaupol checkout, only read (default: reference/gaupol/po)")
    parser.add_argument("--revision", help="the upstream revision of SOURCE, a full 40-character hash")
    parser.add_argument("--template", type=pathlib.Path, default=DEFAULT_TEMPLATE,
                        help="our template (default: src/po/subedit.pot)")
    parser.add_argument("--output-dir", type=pathlib.Path, default=DEFAULT_OUTPUT,
                        help="where the seeds are written, and whose LINGUAS is read (default: src/po)")
    parser.add_argument("--report", type=pathlib.Path, help="also write the coverage table to this file")
    parser.add_argument("--check-fixtures", action="store_true",
                        help="replay the versioned fixture of src/test/data/i18n/seed/ instead")
    args = parser.parse_args()

    if args.check_fixtures:
        return check_fixtures()
    if not args.revision or not re.fullmatch(r"[0-9a-f]{40}", args.revision):
        parser.error("--revision takes the full hash of the upstream revision (git rev-parse HEAD in the checkout)")
    if not args.source.is_dir() or not any(args.source.glob("*.po")):
        parser.error(f"{args.source}: no .po file there")
    if shutil.which("msgmerge") is None:
        print(f"{RED}✗{RESET} msgmerge cannot be found\n  install it with: ./src/scripts/setup-toolchain.sh",
              file=sys.stderr)
        return 1

    outcomes, size = seed(args.source, args.template, args.output_dir, args.revision)
    if args.report:
        args.report.write_text(report(outcomes, size, args.revision), encoding="utf-8")
        print(f"{GREEN}✓{RESET} {args.report}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
