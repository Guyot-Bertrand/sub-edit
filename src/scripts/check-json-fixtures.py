#!/usr/bin/env python3
"""Vérifie les attendus JSON Lines : du JSON valide, l'enveloppe, aucun nombre à virgule.

Le C++ n'a pas de lecteur JSON — il n'écrit, il n'a rien à lire. Ce script est
**le second regard, indépendant de l'écrivain** : l'écrivain (`cli/json.cpp`) et
les attendus versionnés (`src/test/data/attendus/json/`) pourraient s'accorder
sur une erreur, un parseur de la bibliothèque standard Python n'a aucune raison
de s'y plier. ADR 0038, issue #556.

Ce qui est vérifié, **sur chaque ligne de chaque fichier `.jsonl`** :

- la ligne est du JSON, un objet, et le fichier est de l'UTF-8 sans marque, sans
  ligne vide, terminé par `\\n` ;
- l'enveloppe : `schema` (entier), `command` (texte), `file` (texte), `ok`
  (booléen) ;
- un échec (`ok` faux) porte `error.kind` et `error.message`, des textes ;
- un succès porte `warnings`, un tableau ; une sous-commande qui écrit y ajoute
  `destination` et `counts`, un objet d'**entiers** ;
- **aucun nombre à virgule**, nulle part dans l'objet — `1.5`, `1e3` ou `-0.0` :
  une position est un entier de millisecondes, une cadence une chaîne. C'est ce
  qui rend deux sorties comparables octet pour octet.

Usage : check-json-fixtures.py [--dir DOSSIER]

Sans `--dir`, le dossier des attendus du dépôt. Le dossier est un paramètre pour la
raison qui a donné `--input` à `prune-runs.sh` : ce que ce script décide est une
fonction de ce qu'il lit, donc cela se démontre sur un dossier écrit à la main —
voir `verify-gates.sh`.

Sortie : une ligne par défaut, nommant le fichier et la ligne. Code de retour 1
s'il y en a un.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_DIR = REPO_ROOT / "src" / "test" / "data" / "attendus" / "json"

# Les sous-commandes qui écrivent un fichier, et dont l'objet dit où.
WRITERS = {"convert", "shift", "transform", "framerate", "snap", "hearing-impaired"}


class FloatFound(Exception):
    """Un nombre à virgule, signalé par le parseur lui-même."""


def refuse_float(text: str) -> float:
    raise FloatFound(text)


def refuse_constant(text: str) -> float:
    # NaN et Infinity ne sont pas du JSON ; Python les accepte quand même.
    raise FloatFound(text)


def problems_of_line(line: str) -> list[str]:
    """Ce qui ne va pas dans une ligne, ou rien."""
    try:
        record = json.loads(line, parse_float=refuse_float, parse_constant=refuse_constant)
    except FloatFound as found:
        return [f"nombre à virgule : {found}"]
    except json.JSONDecodeError as error:
        return [f"JSON invalide : {error}"]

    if not isinstance(record, dict):
        return ["la ligne n'est pas un objet"]

    problems: list[str] = []
    typed = (("schema", int), ("command", str), ("file", str), ("ok", bool))
    for key, kind in typed:
        value = record.get(key)
        # `bool` est un `int` pour Python : un booléen n'est pas un schéma.
        if not isinstance(value, kind) or (kind is int and isinstance(value, bool)):
            problems.append(f"« {key} » absent ou de mauvais type")
    if problems:
        return problems

    if record["ok"] is False:
        error = record.get("error")
        if not isinstance(error, dict) or not all(
            isinstance(error.get(key), str) for key in ("kind", "message")
        ):
            problems.append("un échec porte error.kind et error.message, des textes")
        return problems

    if not isinstance(record.get("warnings"), list):
        problems.append("un succès porte « warnings », un tableau")
    if record["command"] in WRITERS:
        if not isinstance(record.get("destination"), str):
            problems.append("une sous-commande qui écrit dit sa « destination »")
        counts = record.get("counts")
        if not isinstance(counts, dict) or not all(
            isinstance(value, int) and not isinstance(value, bool) for value in counts.values()
        ):
            problems.append("« counts » est un objet d'entiers")
    return problems


def check(directory: pathlib.Path) -> list[str]:
    files = sorted(directory.glob("*.jsonl"))
    if not files:
        return [f"{directory} : aucun attendu .jsonl à vérifier"]

    report: list[str] = []
    for path in files:
        raw = path.read_bytes()
        if raw.startswith(b"\xef\xbb\xbf"):
            report.append(f"{path.name} : marque d'ordre des octets")
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError as error:
            report.append(f"{path.name} : pas de l'UTF-8 ({error})")
            continue
        if not text.endswith("\n"):
            report.append(f"{path.name} : ne se termine pas par une fin de ligne")
        for number, line in enumerate(text.split("\n")[:-1], start=1):
            if not line:
                report.append(f"{path.name}:{number} : ligne vide")
                continue
            report.extend(f"{path.name}:{number} : {p}" for p in problems_of_line(line))
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
    print(f"✓ {count} attendu(s) JSON : valides, enveloppe complète, aucun nombre à virgule")
    return 0


if __name__ == "__main__":
    sys.exit(main())
