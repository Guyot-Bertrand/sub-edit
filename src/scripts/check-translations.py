#!/usr/bin/env python3
"""Refuse an installed language whose catalogue falls behind the template — issue #670, spec 15, D10.

When a source string changes, its translations go stale: `make po` brings the new
message in empty, or the old translation back flagged `fuzzy`, and nothing
stopped a language from shipping in that state. This check judges every
catalogue of `src/po/` by **what `make po` would make of it** against the
current template, and holds the installed languages to the rule of D5: a
language ships complete.

    check-translations.py [--po-dir DIR] [--root DIR]
    check-translations.py --check-fixtures

## What is compared

The template `src/po/subedit.pot` is taken as it is — the `pot-check` step
proves it is what the code produces — and merged into a **copy** of every
catalogue by `update-po.sh --into`, the very function `make po` runs, with the
same flags: fuzzy matching for a language of `LINGUAS`, none for a seed.
Nothing of `src/po/` is written. The copy is what the counts are taken from;
the committed file is what it is compared with.

**Content, never bytes.** `msgmerge` rewraps long lines and rewrites the
references and the header's dates, and a catalogue edited by hand need not be
laid out as it would lay it out: none of that is a translation falling behind.
An entry is compared as its key (context, msgid, msgid_plural, obsolete or
not) and its content (the translations and the flags); comments, references,
previous msgids, line wrapping and the header are not compared.

## What is found, entry by entry

    NO-FILE        LINGUAS names a language that has no catalogue
    UNTRANSLATED   the entry has no translation (or a plural form is empty)
    NEW            the template has the entry and the committed catalogue does not:
                   it was added since the catalogue was last merged
    FUZZY          the committed entry is flagged fuzzy
    CHANGED        the merge brings the entry back flagged fuzzy: its source
                   changed since the catalogue was last merged
    GONE           the template no longer has an entry the catalogue translates:
                   the merge makes it obsolete
    DIFFERS        the merge changes the entry in another way (its flags)
    PLACEHOLDERS   a translation has not the `%N` arguments of its source — the same
                   set for a singular; for a plural each form a subset of the
                   source's, and the forms together all of the plural's
    ACCELERATOR    the source has an `&` mnemonic and the translation not, the
                   reverse, or the translation has two
    PLURAL-FORMS   a plural has not as many forms as the header's Plural-Forms
                   says, or the header says nothing

The last three are checked on translated entries only, fuzzy ones excepted.

## What fails

**An installed language — a line of `src/po/LINGUAS` — must be complete**: any
finding fails the check. A **seed** — every other catalogue — is only
reported: its counts, its share of the template, and how many entries of each
kind it has; nobody can review a seed (D5), and its gaps are the expected
state of it. Obsolete entries are counted and never fail: `make po` keeps
them, and they are what a translator of a returning sentence starts from.

The report is a table — one line per language, translated, untranslated, fuzzy
and obsolete counted on the merged copy — followed, for each installed
language that fails, by its first offending entries, so that the fix starts
from a name rather than a count. `make po` then a translator's pass is the fix.

`--po-dir` names another catalogue directory (default `src/po` under
`--root`, itself this repository by default): the proofs of `verify-gates.sh`
point it at a scratch copy. msgmerge is the one tool needed, as for `make po`;
without it the check fails and says how to install it, as `pot-check` does
without xgettext.

## Self-test

`--check-fixtures` replays `src/test/data/i18n/stale/`: one small template and
three catalogues, run under several `LINGUAS`, and the exact findings, counts
and verdict each run must give (`cases.json`, described by the README there).
"""

from __future__ import annotations

import argparse
import contextlib
import dataclasses
import importlib.util
import io
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
FIXTURES = REPO_ROOT / "src/test/data/i18n/stale"
UPDATE_PO = REPO_ROOT / "src/scripts/update-po.sh"
SHOWN = 10

RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BOLD = "\033[1m"
RESET = "\033[0m"


def _seed_module():
    """The catalogue reader of `seed-catalogues.py`: one parser of `.po` files for the project."""
    path = REPO_ROOT / "src/scripts/seed-catalogues.py"
    spec = importlib.util.spec_from_file_location("seed_catalogues", path)
    module = importlib.util.module_from_spec(spec)
    sys.modules["seed_catalogues"] = module
    spec.loader.exec_module(module)
    return module


SEED = _seed_module()
PLACEHOLDER = re.compile(r"%[1-9]")


@dataclasses.dataclass(frozen=True)
class Finding:
    language: str
    kind: str
    msgid: str | None
    fatal: bool


@dataclasses.dataclass
class Language:
    name: str
    installed: bool
    translated: int = 0
    untranslated: int = 0
    fuzzy: int = 0
    obsolete: int = 0

    @property
    def total(self) -> int:
        return self.translated + self.untranslated + self.fuzzy


def linguas(po_dir: pathlib.Path) -> list[str]:
    path = po_dir / "LINGUAS"
    if not path.exists():
        return []
    return [line.strip() for line in path.read_text(encoding="utf-8").splitlines()
            if line.strip() and not line.strip().startswith("#")]


def key(entry) -> tuple:
    return (entry.context, entry.msgid, entry.plural, entry.obsolete)


def content(entry) -> tuple:
    return (tuple(sorted(entry.msgstr.items())), frozenset(entry.flags))


def complete(entry) -> bool:
    return bool(entry.msgstr) and all(entry.msgstr.values())


def form_problems(entry, forms: int | None) -> list[str]:
    """The placeholder, accelerator and plural-form findings of one translated entry."""
    found = []
    source = set(PLACEHOLDER.findall(entry.msgid or ""))
    texts = [entry.msgstr[n] for n in sorted(entry.msgstr)]
    if entry.plural is None:
        if set(PLACEHOLDER.findall(texts[0])) != source:
            found.append("PLACEHOLDERS")
    else:
        allowed = source | set(PLACEHOLDER.findall(entry.plural))
        used = [set(PLACEHOLDER.findall(text)) for text in texts]
        if any(not u <= allowed for u in used) or \
                not set(PLACEHOLDER.findall(entry.plural)) <= set().union(*used):
            found.append("PLACEHOLDERS")
    marked = SEED.accelerator_key(entry.msgid or "")[1] > 0
    for text in texts:
        count = SEED.accelerator_key(text)[1]
        if count > 1 or (count == 1) != marked:
            found.append("ACCELERATOR")
            break
    if entry.plural is not None and (forms is None or sorted(entry.msgstr) != list(range(forms))):
        found.append("PLURAL-FORMS")
    return found


def merge(po_dir: pathlib.Path, into: pathlib.Path) -> None:
    """Every catalogue of `po_dir` merged as `make po` would, into `into`; `po_dir` is not written."""
    environment = dict(os.environ, LC_ALL="C", LANGUAGE="en")
    result = subprocess.run([str(UPDATE_PO), "--into", str(into), "--po-dir", str(po_dir)],
                            capture_output=True, text=True, env=environment)
    if result.returncode != 0:
        raise RuntimeError(f"update-po.sh --into failed:\n{result.stderr.strip()}")


def check(po_dir: pathlib.Path) -> tuple[list[Language], list[Finding]]:
    installed = linguas(po_dir)
    catalogues = sorted(path.stem for path in po_dir.glob("*.po"))
    languages: list[Language] = []
    findings: list[Finding] = []
    for name in installed:
        if name not in catalogues:
            findings.append(Finding(name, "NO-FILE", None, True))

    with tempfile.TemporaryDirectory() as scratch:
        merged_dir = pathlib.Path(scratch)
        merge(po_dir, merged_dir)
        for name in catalogues:
            fatal = name in installed
            language = Language(name, fatal)
            languages.append(language)
            committed = SEED.parse_po(po_dir / f"{name}.po")
            merged = SEED.parse_po(merged_dir / f"{name}.po")
            before = {key(e): e for e in committed if e.msgid != ""}
            header = next((e for e in merged if e.msgid == "" and not e.obsolete), None)
            plural_forms = SEED.header_fields(header).get("Plural-Forms") if header else None
            forms = SEED.nplurals(plural_forms) if plural_forms else None

            def found(kind: str, entry) -> None:
                findings.append(Finding(name, kind, entry.msgid, fatal))

            for entry in merged:
                if entry.msgid == "":
                    continue
                old = before.get(key(entry))
                if entry.obsolete:
                    language.obsolete += 1
                    live = before.get((entry.context, entry.msgid, entry.plural, False))
                    if live is not None and old is None:
                        found("GONE", entry)
                    continue
                if "fuzzy" in entry.flags:
                    language.fuzzy += 1
                    found("FUZZY" if old is not None and "fuzzy" in old.flags else "CHANGED", entry)
                    continue
                if not complete(entry):
                    language.untranslated += 1
                    found("UNTRANSLATED" if old is not None else "NEW", entry)
                    continue
                language.translated += 1
                if old is not None and content(old) != content(entry):
                    found("DIFFERS", entry)
                for kind in form_problems(entry, forms):
                    found(kind, entry)
    return languages, findings


def excerpt(text: str | None, width: int = 60) -> str:
    if text is None:
        return "—"
    flat = text.replace("\n", "\\n")
    return repr(flat if len(flat) <= width else flat[:width - 1] + "…")


def report(languages: list[Language], findings: list[Finding], po_dir: pathlib.Path) -> None:
    print(f"{'Language':<10}{'Kind':<11}{'Translated':>11}{'Untransl.':>11}{'Fuzzy':>7}"
          f"{'Obsolete':>10}{'Share':>8}")
    for language in sorted(languages, key=lambda one: (not one.installed, one.name)):
        share = f"{round(100 * language.translated / language.total)} %" if language.total else "–"
        kind = "installed" if language.installed else "seed"
        print(f"{language.name:<10}{kind:<11}{language.translated:>11}{language.untranslated:>11}"
              f"{language.fuzzy:>7}{language.obsolete:>10}{share:>8}")

    for language in sorted(languages, key=lambda one: one.name):
        if language.installed:
            continue
        kinds: dict[str, int] = {}
        for finding in findings:
            if finding.language == language.name and finding.kind != "UNTRANSLATED":
                kinds[finding.kind] = kinds.get(finding.kind, 0) + 1
        if kinds:
            listed = ", ".join(f"{count} {kind}" for kind, count in sorted(kinds.items()))
            print(f"{YELLOW}-{RESET} {language.name} (seed, reported only): {listed}")

    fatal = [finding for finding in findings if finding.fatal]
    for name in sorted({finding.language for finding in fatal}):
        own = [finding for finding in fatal if finding.language == name]
        print(f"{RED}✗{RESET} {name}: {len(own)} problem(s) in an installed language;"
              f" the first {min(len(own), SHOWN)}:")
        for finding in own[:SHOWN]:
            if finding.kind == "NO-FILE":
                print(f"    NO-FILE       {po_dir.name}/LINGUAS names {name!r} and {name}.po does not exist")
            else:
                print(f"    {finding.kind:<13} {name}.po: {excerpt(finding.msgid)}")


def run(po_dir: pathlib.Path) -> int:
    if shutil.which("msgmerge") is None:
        print(f"{RED}✗ msgmerge cannot be found{RESET}\n  install it with: ./src/scripts/setup-toolchain.sh",
              file=sys.stderr)
        return 1
    if not (po_dir / "subedit.pot").exists():
        print(f"{RED}✗{RESET} {po_dir}/subedit.pot does not exist", file=sys.stderr)
        return 1
    languages, findings = check(po_dir)
    report(languages, findings, po_dir)
    if any(finding.fatal for finding in findings):
        print(f"{RED}✗{RESET} an installed language falls behind the template:"
              " `make po`, then translate what is named above")
        return 1
    installed = sum(1 for language in languages if language.installed)
    print(f"{GREEN}✓{RESET} {installed} installed language(s) complete against the template;"
          f" {len(languages) - installed} seed(s) reported")
    return 0


def check_fixtures() -> int:
    """Replay src/test/data/i18n/stale/: each case's LINGUAS, findings, counts and verdict."""
    cases = json.loads((FIXTURES / "cases.json").read_text(encoding="utf-8"))
    failures = 0
    for case in cases:
        with tempfile.TemporaryDirectory() as scratch:
            po_dir = pathlib.Path(scratch) / "po"
            shutil.copytree(FIXTURES / "po", po_dir)
            (po_dir / "LINGUAS").write_text("".join(f"{one}\n" for one in case["linguas"]), encoding="utf-8")
            before = {path.name: path.read_bytes() for path in po_dir.iterdir()}
            languages, findings = check(po_dir)
            printed = io.StringIO()
            with contextlib.redirect_stdout(printed):
                status = run(po_dir)
            after = {path.name: path.read_bytes() for path in po_dir.iterdir()}

        def order(one: list) -> list[str]:
            return [str(part) for part in one]

        got = sorted(([f.language, f.kind, f.msgid, f.fatal] for f in findings), key=order)
        wanted = sorted(case["findings"], key=order)
        counts = {one.name: [one.translated, one.untranslated, one.fuzzy, one.obsolete] for one in languages}
        problems = []
        if got != wanted:
            problems.append(f"findings {got} instead of {wanted}")
        if counts != case["counts"]:
            problems.append(f"counts {counts} instead of {case['counts']}")
        if status != case["status"]:
            problems.append(f"status {status} instead of {case['status']}")
        # What fails is named in the report, by its kind and its source text.
        for finding in findings:
            if finding.fatal and finding.msgid is not None and \
                    f"{finding.kind:<13} {finding.language}.po: {excerpt(finding.msgid)}" not in printed.getvalue():
                problems.append(f"the report does not name {finding.kind} {finding.msgid!r}")
            if finding.fatal and finding.msgid is None and f"names {finding.language!r}" not in printed.getvalue():
                problems.append(f"the report does not name the missing {finding.language}.po")
        if before != after:
            problems.append("the catalogue directory was written")
        if problems:
            failures += 1
            print(f"{RED}✗{RESET} {case['name']}: " + "; ".join(problems))
        else:
            print(f"{GREEN}✓{RESET} {case['name']}")
    if failures:
        print(f"\n{RED}✗{RESET} {failures} stale-translation case(s) do not behave as written")
        return 1
    print(f"{GREEN}✓{RESET} {len(cases)} stale-translation case(s) behave as written")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=pathlib.Path, default=REPO_ROOT,
                        help="the work tree whose src/po is checked (default: this repository)")
    parser.add_argument("--po-dir", type=pathlib.Path,
                        help="the catalogue directory to check (default: src/po under --root)")
    parser.add_argument("--check-fixtures", action="store_true",
                        help="replay the versioned cases of src/test/data/i18n/stale/ instead")
    args = parser.parse_args()
    if args.check_fixtures:
        return check_fixtures()
    po_dir = (args.po_dir or args.root / "src/po").resolve()
    return run(po_dir)


if __name__ == "__main__":
    sys.exit(main())
