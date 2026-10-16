#!/usr/bin/env python3
"""Weighs the criteria for admission into the envelope, on the journal's records.

`docs/mesures/performances.md` keeps two things: a table of extremes, and the
records that feed it. A record feeds the envelope only if it is judged clean —
today on the machine's load. This script exists so that this judgement can be
**measured rather than argued**, from the records the repository already
carries.

    ./src/scripts/analyse-bench-journal.py

It enters no gate, and nothing depends on it: it is an instrument, not a check.
It was born of issue #168, which claimed that the load threshold let noise in —
and of which it showed the opposite.

What it computes, and the vocabulary that goes with it:

  spike    a measure beyond 1.5 times the median of its neighbours in time,
           ±WINDOW records. The window is centred: a real change in the code
           is included in it, a passing spike is not.
  pace     the median ratio, measure by measure, between a record and the
           median of the WINDOW previous ones. What "is this record slow?"
           means when one does not want to rely on the load.

A criterion is judged on two numbers at once: how many measures it admits, and
how many spikes it lets through with them. Admitting little is easy; admitting
much without noise is not.
"""

import re
import statistics
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
JOURNAL = REPO_ROOT / "docs" / "mesures" / "performances.md"

WINDOW = 3
SPIKE = 1.5
UNITS = {"ns": 1.0, "µs": 1e3, "ms": 1e6, "s": 1e9}

HEADER = re.compile(
    r"^### (?P<version>\S+) — (?P<date>\d{4}-\d{2}-\d{2})"
    r" — (?P<mode>[^—\n]+?) — load (?P<load>\S+)"
)


def duration(cell):
    """Rereads a duration of the journal, in nanoseconds."""
    found = re.match(r"^([0-9.]+)\s*(ns|µs|ms|s)$", cell.strip())
    return float(found.group(1)) * UNITS[found.group(2)] if found else None


def records(text):
    """The journal's records, from the oldest to the most recent."""
    body = text.split("<!-- records -->", 1)[-1]
    found, current = [], None
    for line in body.splitlines():
        header = HEADER.match(line)
        if header:
            try:
                load = float(header.group("load"))
            except ValueError:
                load = None
            current = {"version": header.group("version"), "load": load,
                       "measures": {}, "spreads": {}}
            found.append(current)
            continue
        if current is None:
            continue
        cells = [c.strip() for c in line.split("|")]
        if len(cells) == 5 and cells[1] not in ("Measure", ":-----", ""):
            value, deviation = duration(cells[2]), duration(cells[3])
            if value:
                current["measures"][cells[1]] = value
                if deviation is not None:
                    current["spreads"][cells[1]] = deviation / value
    found.reverse()
    return found


def set_paces(found):
    """Sets on each record its pace against the WINDOW previous ones."""
    for rank, record in enumerate(found):
        previous = found[max(0, rank - WINDOW):rank]
        ratios = []
        for name, value in record["measures"].items():
            earlier = [p["measures"][name] for p in previous if name in p["measures"]]
            if len(earlier) < 2:
                continue
            base = statistics.median(earlier)
            if base > 0:
                ratios.append(value / base)
        record["pace"] = statistics.median(ratios) if len(ratios) >= 5 else None


def comparable_measures(found):
    """Each measure, with its ratio to its neighbours in time."""
    series = {}
    for rank, record in enumerate(found):
        for name, value in record["measures"].items():
            series.setdefault(name, []).append((rank, value))

    comparable = []
    for name, points in series.items():
        for index, (rank, value) in enumerate(points):
            start, end = max(0, index - WINDOW), min(len(points), index + WINDOW + 1)
            neighbours = [v for j, (_, v) in enumerate(points[start:end], start=start) if j != index]
            if len(neighbours) < 3:
                continue
            base = statistics.median(neighbours)
            if base <= 0:
                continue
            comparable.append({
                "name": name,
                "version": found[rank]["version"],
                "ratio": value / base,
                "load": found[rank]["load"],
                "pace": found[rank]["pace"],
            })
    return comparable


def main():
    if not JOURNAL.exists():
        print(f"journal not found: {JOURNAL}", file=sys.stderr)
        return 1

    found = records(JOURNAL.read_text(encoding="utf-8"))
    if not found:
        print("no readable record in the journal", file=sys.stderr)
        return 1
    set_paces(found)
    comparable = comparable_measures(found)
    spikes = [m for m in comparable if m["ratio"] > SPIKE]

    print(f"{len(found)} records, {len(comparable)} measures comparable to their neighbours,")
    print(f"of which {len(spikes)} spikes beyond ×{SPIKE}.")
    print()

    def weigh(label, keeps):
        kept = [m for m in comparable if keeps(m)]
        dirty = [m for m in kept if m["ratio"] > SPIKE]
        share = f"{100 * len(dirty) / len(kept):.1f} %" if kept else "—"
        print(f"  {label:<38} {len(kept):4d} measures, {len(dirty):3d} spikes ({share})")
        return dirty

    # The median dispersion of each measure, over all the records kept. It is
    # what a record is compared with, and not a percentage written in advance:
    # the natural dispersion ranges from 3.8 % for scaling a rational to 43.6 %
    # for shifting four thousand subtitles.
    spreads = {}
    for record in found:
        for name, share in record["spreads"].items():
            spreads.setdefault(name, []).append(share)
    median = {name: statistics.median(shares) for name, shares in spreads.items()}
    for measure in comparable:
        record = next(r for r in found if r["version"] == measure["version"])
        measure["spread"] = record["spreads"].get(measure["name"])
        measure["usual_spread"] = median.get(measure["name"])

    print("what each criterion would admit into the envelope:")
    weigh("admit everything", lambda m: True)
    weigh("load < 1.5 — the rule in place",
          lambda m: m["load"] is not None and m["load"] < 1.5)
    let_in = weigh("pace ≤ 1.20", lambda m: m["pace"] is not None and m["pace"] <= 1.20)
    weigh("both at once",
          lambda m: m["load"] is not None and m["load"] < 1.5
          and m["pace"] is not None and m["pace"] <= 1.20)

    def steady(measure, factor):
        if measure["spread"] is None or not measure["usual_spread"]:
            return True
        return measure["spread"] <= factor * measure["usual_spread"]

    for factor in (1.0, 1.25, 1.5, 2.0):
        weigh(f"load, and dispersion <= {factor:g} x its median",
              lambda m, f=factor: m["load"] is not None and m["load"] < 1.5
              and steady(m, f))

    # ## The maxima set, and those that what follows contradicts — issue #202
    #
    # **The spike is not the right lens for this question.** It sees what
    # exceeds its neighbours by a half; a maximum is set by exceeding the
    # previous one by a hair, and it is final. The record that opened #202 was
    # not a spike — it was worth ×1.15 — and it fixed the envelope for good.
    #
    # What is counted here is therefore: how many maxima a criterion lets be
    # set, and how many of them the following records contradict — the median
    # of the next five falling back under four fifths of the maximum set.
    print()
    print("maxima set, and those the following records contradict:")

    series = {}
    for rank, record in enumerate(found):
        for name, value in record["measures"].items():
            series.setdefault(name, []).append((rank, value))

    def count(label, keeps):
        set_count = contradicted = 0
        for name, points in series.items():
            high = None
            for index, (rank, value) in enumerate(points):
                record = found[rank]
                share = record["spreads"].get(name)
                if not keeps(record, name, share):
                    continue
                if high is None or value > high:
                    high = value
                    following = [v for _, v in points[index + 1:index + 6]]
                    if len(following) >= 3:
                        set_count += 1
                        if statistics.median(following) < 0.8 * value:
                            contradicted += 1
        share = f"{100 * contradicted / set_count:.1f} %" if set_count else "—"
        print(f"  {label:<38} {set_count:4d} set, {contradicted:3d} contradicted ({share})")

    quiet = lambda r, n, p: r["load"] is not None and r["load"] < 1.5
    count("load < 1.5 — the rule before #202", quiet)
    for factor in (1.0, 1.25, 1.5, 2.0):
        count(f"load, and dispersion <= {factor:g} x its median",
              lambda r, n, p, f=factor: quiet(r, n, p)
              and (p is None or not median.get(n) or p <= f * median[n]))

    if let_in:
        print()
        print("the spikes the pace would let in, and the load would not:")
        for measure in sorted(let_in, key=lambda m: -m["ratio"])[:8]:
            if measure["load"] is not None and measure["load"] < 1.5:
                continue
            print(f"  ×{measure['ratio']:5.2f}  {measure['version']:<7}"
                  f"  pace ×{measure['pace']:.2f}  load {measure['load']:.2f}"
                  f"  {measure['name']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
