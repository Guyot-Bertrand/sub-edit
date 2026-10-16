#!/usr/bin/env python3
"""Recomputes the table of extremes from the records the journal keeps.

**The table is never pruned, so an extreme set by noise is final** — and a
maximum only falls before a higher maximum. An admission rule that tightens
therefore does not repair the past by itself: what was set when the rule was
looser has to be gone over again.

It happened three times. Two corrections by hand — version `0.2.15`, then
`0.5.13` — and that of issue #202, which added the dispersion criterion and
found **twenty-three of the sixty extremes** set by a record that this
criterion refuses. This script exists so that the fourth time is not a fourth
correction by hand.

    --dry-run   (default) writes what would change, touches nothing
    --apply     rewrites the table

**It can only repair what the journal still carries.** Records older than a
month are pruned; an extreme whose source has disappeared can be neither judged
nor replaced, and it is left as is — saying so is better than recomputing it on
a truncated history.

It enters no gate. It is not a check, it is a repair.
"""

from __future__ import annotations

import argparse
import re
import statistics
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
JOURNAL = REPO_ROOT / "docs" / "mesures" / "performances.md"

# The two admission criteria, written as record-bench.sh writes them. Two places
# for a single number is one too many; they are here because this script must be
# able to replay a past rule to understand what it let through.
LOAD_BELOW = 1.5
SPREAD_FACTOR = 1.25
SPREAD_MINIMUM = 5

UNITS = {"ns": 1.0, "µs": 1e3, "ms": 1e6, "s": 1e9}
HEADER = re.compile(r"^### (?P<version>\S+) — (?P<date>\S+) — [^—]+ — load (?P<load>\S+)")
ROW = re.compile(r"^\| (?P<name>.+?) \| (?P<mean>[^|]+?) \| (?P<sd>[^|]+?) \|$", re.MULTILINE)
TABLE_ROW = re.compile(
    r"^\| (?P<name>.+?) \| (?P<low>[^|]+?) \| (?P<minsrc>[^|]+?)"
    r" \| (?P<high>[^|]+?) \| (?P<maxsrc>[^|]+?) \|$",
    re.MULTILINE,
)
RAW = re.compile(r"<!-- (?P<name>.+?) min=(?P<low>[0-9.eE+-]+) max=(?P<high>[0-9.eE+-]+) -->")


def nanoseconds(cell):
    found = re.match(r"^([0-9.]+)\s*(ns|µs|ms|s)$", cell.strip())
    return float(found.group(1)) * UNITS[found.group(2)] if found else None


def humanise(value):
    """Three significant digits, in the unit that fits — as record-bench.sh does."""
    for unit, scale in (("s", 1e9), ("ms", 1e6), ("µs", 1e3), ("ns", 1.0)):
        if value >= scale:
            shown = f"{value / scale:.3g}"
            return f"{shown} {unit}"
    return f"{value:.3g} ns"


def readings(text):
    """The records kept: version, date, load, means and dispersions."""
    body = text.split("<!-- records -->", 1)[-1]
    found, current = [], None
    for line in body.splitlines():
        head = HEADER.match(line)
        if head:
            try:
                load = float(head.group("load"))
            except ValueError:
                load = None
            current = {
                "stamp": f"{head.group('version')} — {head.group('date')}",
                "load": load,
                "means": {},
                "spreads": {},
            }
            found.append(current)
            continue
        if current is None:
            continue
        cells = [c.strip() for c in line.split("|")]
        if len(cells) == 5 and cells[1] not in ("Measure", ":-----", ""):
            mean, sd = nanoseconds(cells[2]), nanoseconds(cells[3])
            if mean:
                current["means"][cells[1]] = mean
                if sd is not None:
                    current["spreads"][cells[1]] = sd / mean
    return found


def main():
    parser = argparse.ArgumentParser(add_help=True)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true",
                      help="(default) writes what would change, touches nothing")
    mode.add_argument("--apply", action="store_true", help="rewrites the table")
    parser.add_argument("--journal", default=str(JOURNAL))
    options = parser.parse_args()

    path = Path(options.journal)
    text = path.read_text(encoding="utf-8")
    kept = readings(text)
    if not kept:
        print("no readable record in the journal", file=sys.stderr)
        return 1

    usual = {}
    for reading in kept:
        for name, spread in reading["spreads"].items():
            usual.setdefault(name, []).append(spread)
    median = {name: statistics.median(seen) for name, seen in usual.items()}

    def admits(reading, name):
        if reading["load"] is None or reading["load"] >= LOAD_BELOW:
            return False
        seen = usual.get(name, [])
        spread = reading["spreads"].get(name)
        if len(seen) < SPREAD_MINIMUM or spread is None:
            return True
        return spread <= SPREAD_FACTOR * median[name]

    # **Only faulty extremes move, and an extreme is faulty when its own
    # originating record is refused.** Recomputing the whole envelope from the
    # records kept looks simpler and loses more: every extreme whose source was
    # pruned would be replaced by the best of the last thirty days, which raises
    # the minima and forgets what was measured. That is not what #202 asks.
    #
    # Three cases, and two touch nothing:
    #
    #   source absent from the journal  left — it can be neither judged nor
    #                                   replaced, and saying so is better than
    #                                   recomputing on a truncated history
    #   source present, admitted        left — it is what it claims to be
    #   source present, refused         replaced by the best admitted record
    by_stamp = {reading["stamp"]: reading for reading in kept}

    best = {}
    for reading in kept:
        for name, mean in reading["means"].items():
            if not admits(reading, name):
                continue
            low, minsrc, high, maxsrc = best.get(
                name, (mean, reading["stamp"], mean, reading["stamp"]))
            if mean < low:
                low, minsrc = mean, reading["stamp"]
            if mean > high:
                high, maxsrc = mean, reading["stamp"]
            best[name] = (low, minsrc, high, maxsrc)

    head, marker, rest = text.partition("<!-- extremes -->")
    block, records_marker, records = rest.partition("## Records")

    changes = []
    unjudgeable = []
    raw = {m.group("name"): m for m in RAW.finditer(block)}
    kept_values = {}

    def verdict(name, role, source):
        """Must this extreme move, and why?"""
        reading = by_stamp.get(source)
        if reading is None:
            unjudgeable.append((name, role, source))
            return False
        return not admits(reading, name)

    def rewrite_row(match):
        name = match.group("name")
        low = float(raw[name].group("low")) if name in raw else None
        high = float(raw[name].group("high")) if name in raw else None
        minsrc, maxsrc = match.group("minsrc").strip(), match.group("maxsrc").strip()

        if low is not None and verdict(name, "minimum", minsrc) and name in best:
            changes.append((name, "minimum", low, best[name][0]))
            low, minsrc = best[name][0], best[name][1]
        if high is not None and verdict(name, "maximum", maxsrc) and name in best:
            changes.append((name, "maximum", high, best[name][2]))
            high, maxsrc = best[name][2], best[name][3]

        if low is None or high is None:
            return match.group(0)
        kept_values[name] = (low, high)
        return f"| {name} | {humanise(low)} | {minsrc} | {humanise(high)} | {maxsrc} |"

    rebuilt = TABLE_ROW.sub(rewrite_row, block)
    rebuilt = RAW.sub(
        lambda m: (f"<!-- {m.group('name')} min={kept_values[m.group('name')][0]!r}"
                   f" max={kept_values[m.group('name')][1]!r} -->")
        if m.group("name") in kept_values else m.group(0),
        rebuilt,
    )

    print(f"{len(kept)} records kept, {len(raw)} measures in the table")
    print(f"{len(changes)} extreme(s) set by a record the criteria refuse:")
    for name, role, before, after in sorted(changes):
        print(f"  {role:<7} {humanise(before):>9} -> {humanise(after):>9}   {name}")
    if unjudgeable:
        print(f"\n{len(unjudgeable)} extreme(s) whose originating record was pruned: left as is")

    if not options.apply:
        print("\n--dry-run: nothing was written. --apply to rewrite.")
        return 0

    path.write_text(head + marker + rebuilt + records_marker + records, encoding="utf-8")
    print("\ntable rewritten.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
