#!/usr/bin/env python3
"""Measure what including a header costs the units that include it (#549).

**A measurement, not a gate**: it gates nothing, and is replayed when one wants
to know whether a frequently changed header weighs. It answers two distinct
questions, which must not be confused:

    the PARSING cost       how much frontend time each unit spends in the
                           header and what it includes (an upper bound on the
                           gain of slimming it: an inclusion the unit would pay
                           anyway through another path is not saved);
    the INVALIDATION cost  the number of units a change to the header forces
                           to recompile — and to reanalyse under clang-tidy.
                           That is what every commit touching it pays.

Method: `clang++ -c -O0 -ftime-trace` on each unit that includes the header,
with the options of `build/dev/compile_commands.json` minus GCC's; the header's
time is the **inclusive** time of the "Source" events that name it. One pass
per unit by default (`REPS=3` for a median): on a loaded machine, comparing two
readings taken under different loads says nothing.

    src/scripts/measure-include-cost.py subedit/core/wording/translation.hpp
"""

from __future__ import annotations

import collections
import json
import os
import pathlib
import shlex
import statistics
import subprocess
import sys
import tempfile
import time

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent


def units_including(header: str) -> list[pathlib.Path]:
    """The repository's `.cpp` files that include `header` — `src/exe` excluded: it has no compile entry of its own."""
    needle = f"#include <{header}>"
    units = []
    for path in sorted((REPO_ROOT / "src").rglob("*.cpp")):
        if "src/exe" in str(path):
            continue
        if needle in path.read_text(encoding="utf-8", errors="replace"):
            units.append(path)
    return units


def compile_command(entries: dict, unit: pathlib.Path) -> tuple[list[str], str] | None:
    entry = entries.get(unit.resolve())
    if entry is None:
        return None
    arguments: list[str] = []
    skip = False
    for argument in shlex.split(entry["command"])[1:]:
        if skip:
            skip = False
            continue
        if argument in ("-o", "-MF", "-MT"):
            skip = True
            continue
        if argument in ("-MD", "-c") or argument.startswith(("-W", "-fsanitize")):
            continue
        if argument.endswith(".cpp"):
            continue
        arguments.append(argument)
    return arguments, entry["directory"]


def measure(unit: pathlib.Path, header: str, entries: dict, scratch: pathlib.Path, reps: int):
    built = compile_command(entries, unit)
    if built is None:
        return None
    arguments, directory = built
    obj = scratch / (unit.stem + ".o")
    command = ["clang++-20", "-c", "-O0", "-ftime-trace", "-ftime-trace-granularity=1000",
               "-Wno-unknown-warning-option", *arguments, str(unit), "-o", str(obj)]
    times = []
    for _ in range(reps):
        start = time.time()
        result = subprocess.run(command, cwd=directory, capture_output=True, text=True, check=False)
        times.append(time.time() - start)
        if result.returncode:
            print(result.stderr[:400], file=sys.stderr)
            return None
    events = json.load(open(obj.with_suffix(".json")))["traceEvents"]
    # The "Source" events are nested b/e pairs: they are matched with a stack.
    stack: list[tuple[int, str]] = []
    own = 0.0
    for event in sorted((e for e in events if e.get("name") == "Source"), key=lambda e: e["ts"]):
        if event["ph"] == "b":
            stack.append((event["ts"], event["args"]["detail"]))
        elif stack:
            began, detail = stack.pop()
            if detail.endswith(header.removeprefix("subedit/")) or detail.endswith(header):
                own += (event["ts"] - began) / 1e6
    frontend = next((e["dur"] for e in events if e.get("name") == "Total Frontend"), 0) / 1e6
    return statistics.median(times), frontend, own


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__.split("\n\n")[0], file=sys.stderr)
        print("usage: measure-include-cost.py <header, relative to src/lib>", file=sys.stderr)
        return 2
    header = sys.argv[1]
    entries = {pathlib.Path(e["file"]).resolve(): e
               for e in json.load(open(REPO_ROOT / "build/dev/compile_commands.json"))}
    units = units_including(header)
    reps = int(os.environ.get("REPS", "1"))
    rows = []
    with tempfile.TemporaryDirectory() as scratch:
        for unit in units:
            result = measure(unit, header, entries, pathlib.Path(scratch), reps)
            if result is not None:
                rows.append((unit.relative_to(REPO_ROOT), *result))

    frontend = sum(r[2] for r in rows)
    in_header = sum(r[3] for r in rows)
    print(f"{header}: {len(units)} units include it, {len(rows)} measured")
    print(f"  invalidation cost: a change recompiles these {len(rows)} units, "
          f"{frontend:.0f} s of cumulated frontend (code generation and clang-tidy excluded)")
    print(f"  parsing cost:      {in_header:.0f} s in the header and its inclusions, "
          f"that is {100 * in_header / frontend:.1f} % — an upper bound on what slimming the header saves")
    groups: dict[str, list[float]] = collections.defaultdict(lambda: [0, 0.0, 0.0])
    for path, _, fe, wd in rows:
        key = "test" if path.parts[1] == "test" else path.parts[3]
        groups[key][0] += 1
        groups[key][1] += fe
        groups[key][2] += wd
    for key, (count, fe, wd) in sorted(groups.items(), key=lambda kv: -kv[1][1]):
        print(f"    {key:6s} {int(count):3d} units  frontend {fe:6.1f} s  header {wd:5.1f} s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
