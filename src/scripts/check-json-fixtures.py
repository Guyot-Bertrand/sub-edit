#!/usr/bin/env python3
"""Checks the JSON Lines expected outputs: valid JSON, the envelope, no floating-point number.

The C++ has no JSON reader — it writes, it has nothing to read. This script is
**the second look, independent of the writer**: the writer (`cli/json.cpp`) and
the tracked expected outputs (`src/test/data/attendus/json/`) could agree on a
mistake; a parser from Python's standard library has no reason to go along with
it. ADR 0038, issue #556.

What is checked, **on every line of every `.jsonl` file**:

- the line is JSON, an object, and the file is UTF-8 without a byte order mark,
  without an empty line, ending with `\\n`;
- the envelope: `schema` (integer), `command` (text), `file` (text), `ok`
  (boolean);
- a failure (`ok` false) carries `error.kind` and `error.message`, both texts;
- a success carries `warnings`, an array; a subcommand that writes adds
  `dry_run`, `destination` — a path, or `null` when the run is a dry run — and
  `counts`, an object of **integers**; `changes`, when present, is the list of
  the texts changed;
- **no floating-point number**, anywhere in the object — `1.5`, `1e3` or
  `-0.0`: a position is an integer number of milliseconds, a frame rate a
  string. That is what makes two outputs comparable byte for byte.

Usage: check-json-fixtures.py [--dir DIRECTORY]

Without `--dir`, the repository's expected-output directory. The directory is a
parameter for the reason that gave `--input` to `prune-runs.sh`: what this script
decides is a function of what it reads, so it can be demonstrated on a directory
written by hand — see `verify-gates.sh`.

Output: one line per defect, naming the file and the line. Exit code 1 if there
is any.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_DIR = REPO_ROOT / "src" / "test" / "data" / "attendus" / "json"

# The subcommands that write a file, and whose object says where.
WRITERS = {"convert", "shift", "transform", "framerate", "snap", "hearing-impaired", "adjust",
           "replace", "case", "italics", "dialogue-dashes", "sort", "append", "split-file", "pair"}


class FloatFound(Exception):
    """A floating-point number, reported by the parser itself."""


def refuse_float(text: str) -> float:
    raise FloatFound(text)


def refuse_constant(text: str) -> float:
    # NaN and Infinity are not JSON; Python accepts them all the same.
    raise FloatFound(text)


def is_integer(value: object) -> bool:
    """An integer, and not a boolean — which Python nonetheless takes for an `int`."""
    return isinstance(value, int) and not isinstance(value, bool)


def problems_of_changes(changes: object) -> list[str]:
    """What is wrong in `changes`: `{subtitle, document, before, after}`, `after` possibly null."""
    if not isinstance(changes, list):
        return ["\"changes\" is an array"]
    for change in changes:
        if not isinstance(change, dict) or set(change) != {"subtitle", "document", "before", "after"}:
            return ["a change carries subtitle, document, before and after, and nothing else"]
        number = change["subtitle"]
        if not isinstance(number, int) or isinstance(number, bool) or number < 1:
            return ["\"subtitle\" is an integer number, from one"]
        if change["document"] not in ("main", "translation"):
            return ["\"document\" is main or translation"]
        if not isinstance(change["before"], str) or not (
            change["after"] is None or isinstance(change["after"], str)
        ):
            return ["\"before\" is a text, \"after\" a text or null"]
    return []


def problems_of_line(line: str) -> list[str]:
    """What is wrong in a line, or nothing."""
    try:
        record = json.loads(line, parse_float=refuse_float, parse_constant=refuse_constant)
    except FloatFound as found:
        return [f"floating-point number: {found}"]
    except json.JSONDecodeError as error:
        return [f"invalid JSON: {error}"]

    if not isinstance(record, dict):
        return ["the line is not an object"]

    problems: list[str] = []
    typed = (("schema", int), ("command", str), ("file", str), ("ok", bool))
    for key, kind in typed:
        value = record.get(key)
        # `bool` is an `int` to Python: a boolean is not a schema.
        if not isinstance(value, kind) or (kind is int and isinstance(value, bool)):
            problems.append(f"\"{key}\" missing or of the wrong type")
    if problems:
        return problems

    if record["ok"] is False:
        error = record.get("error")
        if not isinstance(error, dict) or not all(
            isinstance(error.get(key), str) for key in ("kind", "message")
        ):
            problems.append("a failure carries error.kind and error.message, both texts")
        return problems

    if not isinstance(record.get("warnings"), list):
        problems.append("a success carries \"warnings\", an array")
    if record["command"] in WRITERS:
        # A dry run writes nowhere: it says so with `null`, and a real run with
        # a path — never one for the other (ADR 0040).
        dry_run = record.get("dry_run")
        if not isinstance(dry_run, bool):
            problems.append("a subcommand that writes states \"dry_run\", a boolean")
        elif dry_run and record.get("destination") is not None:
            problems.append("a dry run has no \"destination\": null")
        elif not dry_run and not isinstance(record.get("destination"), str):
            problems.append("a subcommand that writes states its \"destination\"")
        if "changes" in record:
            problems.extend(problems_of_changes(record["changes"]))
        counts = record.get("counts")
        if not isinstance(counts, dict) or not all(
            is_integer(value)
            # A group of counts (`sacrificed`): an object of integers, one level deep.
            or (isinstance(value, dict) and value and all(is_integer(one) for one in value.values()))
            for value in counts.values()
        ):
            problems.append("\"counts\" is an object of integers, or of groups of integers")
    return problems


def check(directory: pathlib.Path) -> list[str]:
    files = sorted(directory.glob("*.jsonl"))
    if not files:
        return [f"{directory}: no .jsonl expected output to check"]

    report: list[str] = []
    for path in files:
        raw = path.read_bytes()
        if raw.startswith(b"\xef\xbb\xbf"):
            report.append(f"{path.name}: byte order mark")
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError as error:
            report.append(f"{path.name}: not UTF-8 ({error})")
            continue
        if not text.endswith("\n"):
            report.append(f"{path.name}: does not end with a line ending")
        for number, line in enumerate(text.split("\n")[:-1], start=1):
            if not line:
                report.append(f"{path.name}:{number}: empty line")
                continue
            report.extend(f"{path.name}:{number}: {p}" for p in problems_of_line(line))
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--dir", type=pathlib.Path, default=DEFAULT_DIR)
    arguments = parser.parse_args()

    report = check(arguments.dir)
    for line in report:
        print(f"✗ {line}", file=sys.stderr)
    if report:
        return 1
    count = len(list(arguments.dir.glob("*.jsonl")))
    print(f"✓ {count} JSON expected output(s): valid, complete envelope, no floating-point number")
    return 0


if __name__ == "__main__":
    sys.exit(main())
