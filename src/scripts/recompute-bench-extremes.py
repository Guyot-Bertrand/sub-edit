#!/usr/bin/env python3
"""Recalcule la table des extrêmes depuis les relevés que le journal conserve.

**La table n'est jamais élaguée, donc un extrême posé par du bruit est
définitif** — et un maximum ne tombe que devant un maximum plus haut. Une règle
d'admission qui se resserre ne répare donc pas le passé toute seule : il faut
repasser sur ce qui a été posé quand la règle était plus lâche.

C'est arrivé trois fois. Deux corrections à la main — la version `0.2.15`, puis
`0.5.13` — et celle de l'issue #202, qui a ajouté le critère de dispersion et
trouvé **vingt-trois des soixante extrêmes** posés par un relevé que ce critère
refuse. Ce script existe pour que la quatrième fois ne soit pas une quatrième
correction à la main.

    --dry-run   (défaut) écrit ce qui changerait, ne touche à rien
    --apply     réécrit la table

**Il ne peut réparer que ce que le journal porte encore.** Les relevés de plus
d'un mois sont élagués ; un extrême dont la source a disparu ne peut être ni
jugé ni remplacé, et il est laissé tel quel — le dire vaut mieux que de le
recalculer sur une histoire tronquée.

Il n'entre dans aucune porte. Ce n'est pas une vérification, c'est une réparation.
"""

from __future__ import annotations

import argparse
import re
import statistics
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
JOURNAL = REPO_ROOT / "docs" / "mesures" / "performances.md"

# Les deux critères d'admission, écrits comme record-bench.sh les écrit. Deux
# endroits pour un seul nombre est un de trop ; ils sont ici parce que ce script
# doit pouvoir rejouer une règle passée pour comprendre ce qu'elle a laissé.
LOAD_BELOW = 1.5
SPREAD_FACTOR = 1.25
SPREAD_MINIMUM = 5

UNITS = {"ns": 1.0, "µs": 1e3, "ms": 1e6, "s": 1e9}
HEADER = re.compile(r"^### (?P<version>\S+) — (?P<date>\S+) — [^—]+ — charge (?P<load>\S+)")
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
    """Trois chiffres significatifs, dans l'unité qui va — comme record-bench.sh."""
    for unit, scale in (("s", 1e9), ("ms", 1e6), ("µs", 1e3), ("ns", 1.0)):
        if value >= scale:
            shown = f"{value / scale:.3g}"
            return f"{shown} {unit}"
    return f"{value:.3g} ns"


def readings(text):
    """Les relevés conservés : version, date, charge, moyennes et dispersions."""
    body = text.split("<!-- relevés -->", 1)[-1]
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
        if len(cells) == 5 and cells[1] not in ("Mesure", ":-----", ""):
            mean, sd = nanoseconds(cells[2]), nanoseconds(cells[3])
            if mean:
                current["means"][cells[1]] = mean
                if sd is not None:
                    current["spreads"][cells[1]] = sd / mean
    return found


def main():
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument("--apply", action="store_true", help="réécrit la table")
    parser.add_argument("--journal", default=str(JOURNAL))
    options = parser.parse_args()

    path = Path(options.journal)
    text = path.read_text(encoding="utf-8")
    kept = readings(text)
    if not kept:
        print("aucun relevé lisible dans le journal", file=sys.stderr)
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

    # **Seuls les extrêmes fautifs bougent, et un extrême est fautif quand son
    # propre relevé d'origine est refusé.** Recalculer l'enveloppe entière
    # depuis les relevés conservés paraît plus simple et perd davantage : tout
    # extrême dont la source a été élaguée serait remplacé par le meilleur des
    # trente derniers jours, ce qui remonte les minima et oublie ce qui a été
    # mesuré. Ce n'est pas ce que #202 demande.
    #
    # Trois cas, et deux ne touchent à rien :
    #
    #   source absente du journal   laissé — on ne peut ni le juger ni le
    #                               remplacer, et le dire vaut mieux que de
    #                               recalculer sur une histoire tronquée
    #   source présente, admise     laissé — il est ce qu'il prétend être
    #   source présente, refusée    remplacé par le meilleur relevé admis
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

    head, marker, rest = text.partition("<!-- extrêmes -->")
    block, records_marker, records = rest.partition("## Relevés")

    changes = []
    unjudgeable = []
    raw = {m.group("name"): m for m in RAW.finditer(block)}
    kept_values = {}

    def verdict(name, role, source):
        """Cet extrême doit-il bouger, et pourquoi ?"""
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

    print(f"{len(kept)} relevés conservés, {len(raw)} mesures dans la table")
    print(f"{len(changes)} extrême(s) posés par un relevé que les critères refusent :")
    for name, role, before, after in sorted(changes):
        print(f"  {role:<7} {humanise(before):>9} -> {humanise(after):>9}   {name}")
    if unjudgeable:
        print(f"\n{len(unjudgeable)} extrême(s) dont le relevé d'origine est élagué : laissés tels quels")

    if not options.apply:
        print("\n--dry-run : rien n'a été écrit. --apply pour réécrire.")
        return 0

    path.write_text(head + marker + rebuilt + records_marker + records, encoding="utf-8")
    print("\ntable réécrite.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
