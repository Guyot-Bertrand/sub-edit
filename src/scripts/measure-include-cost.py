#!/usr/bin/env python3
"""Mesure ce que l'inclusion d'un en-tête coûte aux unités qui l'incluent (#549).

**Une mesure, pas une porte** : elle ne gate rien, elle se rejoue quand on veut
savoir si un en-tête fréquemment modifié pèse. Elle répond à deux questions
distinctes, qu'il ne faut pas confondre :

    le coût d'ANALYSE      combien de temps de frontend chaque unité passe dans
                           l'en-tête et ce qu'il inclut (borne haute du gain à
                           l'alléger : une inclusion que l'unité paierait de
                           toute façon par un autre chemin ne s'économise pas) ;
    le coût d'INVALIDATION le nombre d'unités qu'une modification de l'en-tête
                           oblige à recompiler — et à réanalyser sous clang-tidy.
                           C'est ce qui se paie à chaque commit qui y touche.

Méthode : `clang++ -c -O0 -ftime-trace` sur chaque unité qui inclut l'en-tête,
avec les options de `build/dev/compile_commands.json` moins celles de GCC ; le
temps de l'en-tête est l'**inclusif** des événements « Source » qui le nomment.
Un seul passage par unité par défaut (`REPS=3` pour un médian) : sur une machine
chargée, comparer deux relevés pris à des charges différentes ne dit rien.

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
    """Les `.cpp` du dépôt qui incluent `header` — `src/exe` exclu : sans entrée de compilation propre."""
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
    # Les « Source » sont des paires b/e imbriquées : on les apparie par pile.
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
        print("usage : measure-include-cost.py <en-tête, relatif à src/lib>", file=sys.stderr)
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
    print(f"{header} : {len(units)} unités l'incluent, {len(rows)} mesurées")
    print(f"  coût d'invalidation : une modification recompile ces {len(rows)} unités, "
          f"{frontend:.0f} s de frontend cumulé (hors génération de code et clang-tidy)")
    print(f"  coût d'analyse      : {in_header:.0f} s dans l'en-tête et ses inclusions, "
          f"soit {100 * in_header / frontend:.1f} % — borne haute de ce qu'alléger l'en-tête économise")
    groups: dict[str, list[float]] = collections.defaultdict(lambda: [0, 0.0, 0.0])
    for path, _, fe, wd in rows:
        key = "test" if path.parts[1] == "test" else path.parts[3]
        groups[key][0] += 1
        groups[key][1] += fe
        groups[key][2] += wd
    for key, (count, fe, wd) in sorted(groups.items(), key=lambda kv: -kv[1][1]):
        print(f"    {key:6s} {int(count):3d} unités  frontend {fe:6.1f} s  en-tête {wd:5.1f} s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
