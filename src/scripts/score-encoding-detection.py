#!/usr/bin/env python3
"""Score une détection d'encodage, au lieu de la croire.

**Une détection est un classifieur, pas une fonction.** Elle a un taux de
succès, pas un résultat juste ou faux, et « la détection marche » sans chiffre
est exactement le genre d'affirmation que trois relectures de suite ont
démentie — l'icône que trois lecteurs sur quatre acceptaient, le `.rpm`
irréprochable à l'inspection et refusé à l'installation, les raccourcis mesurés
sur une plateforme qu'aucun utilisateur n'a.

**Le détecteur est un argument, et c'est tout l'intérêt.** Le cadrage de la
phase 8 (#288) a trois voies — ICU, uchardet, des tables écrites — et le choix
entre elles est un choix de taux. Le mesurer après avoir choisi, c'est choisir
sans mesurer. Chaque voie se passe donc au même corpus, sans que ce script ait à
connaître aucune d'elles :

    ./src/scripts/score-encoding-detection.py --detector 'uchardet {}'
    ./src/scripts/score-encoding-detection.py --detector 'file -b --mime-encoding {}'
    ./src/scripts/score-encoding-detection.py --detector './build/dev/bin/probe {}' --prive

`{}` est remplacé par le chemin du fichier ; ce que la commande écrit sur sa
sortie standard est lu comme le nom de l'encodage.

## Les deux corpus, et ce que chacun peut dire

**Le corpus étiqueté** — `src/test/data/encodages/`, fabriqué par
`encoding-fixtures.py`. L'étiquette vient avec le fichier, gratuitement : la
table dit dans quel encodage il a été écrit. C'est le seul des deux qui donne un
**taux**, et il donne aussi le détail des confusions — confondre deux latins ne
coûte pas ce que coûte confondre un latin et un cyrillique.

**Le corpus privé** — `src/data/`, qui porte des fichiers réels, et des fichiers
réels mal encodés sont précisément ce qu'un corpus fabriqué ne contient pas.
**Il n'a pas d'étiquettes**, donc il ne donne aucun taux : ce serait inventer une
vérité. Ce qu'il donne est ce qui se vérifie sans elle — la répartition des
réponses, combien décodent réellement le fichier de bout en bout, et sur quoi
deux détecteurs se contredisent.

**Il ne sait pas nommer un fichier**, et ce n'est pas une politesse : la règle du
dépôt interdit de citer le corpus privé, et un outil qui *peut* le faire finit
par le faire. Rien ici ne porte un chemin jusqu'à la sortie.

## Ce qui n'est pas mesuré ici

Le temps. Une détection qui prend dix millisecondes par fichier n'est pas un
problème dans un éditeur de sous-titres ; le banc est là pour le jour où la
question se posera.
"""

from __future__ import annotations

import argparse
import codecs
import collections
import importlib.util
import re
import shlex
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LABELLED = REPO_ROOT / "src" / "test" / "data" / "encodages"
PRIVATE = REPO_ROOT / "src" / "data"

GREEN = "\033[32m"
RED = "\033[31m"
BOLD = "\033[1m"
RESET = "\033[0m"

# Ce que les détecteurs écrivent, et que `codecs` ne sait pas relire tel quel.
ALIASES = {
    "utf-16": "utf-16",
    "utf-16le": "utf-16-le",
    "utf-16be": "utf-16-be",
    "us-ascii": "ascii",
    "unknown-8bit": "?",
    "binary": "?",
    "": "?",
    "none": "?",
}


def canonical(name):
    """Le nom que Python donne à cet encodage, ou `?` s'il n'en connaît aucun.

    Les détecteurs n'écrivent pas les mêmes mots pour le même encodage —
    `WINDOWS-1252`, `cp1252`, `windows-1252`. La comparaison passe donc par le
    nom canonique plutôt que par la chaîne rendue.
    """
    cleaned = name.strip().lower()
    if cleaned in ALIASES:
        return ALIASES[cleaned]
    try:
        return codecs.lookup(cleaned).name
    except LookupError:
        return cleaned or "?"


def labelled_fixtures():
    """Les fixtures et leur encodage, lus de la table qui les fabrique."""
    spec = importlib.util.spec_from_file_location(
        "encoding_fixtures", REPO_ROOT / "src" / "scripts" / "encoding-fixtures.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return [(LABELLED / name, encoding, bom) for name, encoding, bom, *_ in module.FIXTURES]


# La forme d'un nom d'encodage, et rien d'autre. Lettres, chiffres, tirets,
# points et soulignés, sur quelques dizaines de caractères au plus.
ENCODING_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,31}$")


def detect(command, path):
    """Ce que le détecteur dit de ce fichier, ou `?` s'il ne dit rien.

    **Sans shell, et la réponse est filtrée.** Les deux gardes existent pour la
    même raison, et elles ont été payées : la première version passait la
    commande au shell avec le chemin substitué en clair, et les noms du corpus
    privé portent des espaces et des crochets. `file` a donc reçu des arguments
    éclatés, s'est plaint de chacun — et **ses plaintes, qui citaient les noms,
    ont été lues comme des encodages et imprimées.**

    C'est exactement ce que #290 demandait de rendre impossible : « l'outil ne
    doit pas être capable de nommer un fichier, pas seulement s'en abstenir ».

    Le chemin est donc un élément d'`argv` et non un morceau de ligne de
    commande, et **ce qui ressort est refusé s'il n'a pas la forme d'un nom
    d'encodage** — un chemin n'en a jamais la forme. La sortie d'erreur, elle,
    n'est jamais lue.
    """
    try:
        parts = shlex.split(command)
    except ValueError:
        return "?"

    argv = [str(path) if part == "{}" else part for part in parts]

    try:
        done = subprocess.run(argv, capture_output=True, text=True, timeout=30)
    except (OSError, subprocess.SubprocessError):
        return "?"

    if done.returncode != 0:
        return "?"

    said = done.stdout.strip().splitlines()[0].strip() if done.stdout.strip() else ""
    return canonical(said) if ENCODING_NAME.match(said) else "?"


def agrees(said, expected, has_bom):
    """Le détecteur a-t-il raison ?

    **`utf-16` sans son côté est une bonne réponse quand le fichier a un BOM.**
    C'est le BOM qui porte l'ordre des octets, et un détecteur qui rend `UTF-16`
    ne s'est pas trompé : il a laissé au BOM ce qui lui revient.
    """
    if said == expected:
        return True
    return has_bom and said == "utf-16" and expected in ("utf-16-le", "utf-16-be")


def decodes(path, encoding):
    """Ces octets se décodent-ils entièrement dans cet encodage ?

    C'est la seule chose qu'on puisse vérifier sans étiquette — et elle est
    nécessaire sans être suffisante : un mono-octet décode presque tout, ce qui
    est précisément d'où vient le mojibake.
    """
    if encoding == "?":
        return False
    try:
        path.read_bytes().decode(encoding)
        return True
    except (UnicodeDecodeError, LookupError):
        return False


def score_labelled(command):
    fixtures = labelled_fixtures()
    right = 0
    confusions = []

    print(f"{BOLD}corpus étiqueté — {len(fixtures)} fixtures{RESET}")
    for path, expected, has_bom in fixtures:
        if not path.is_file():
            print(f"  {RED}✗{RESET} {path.name} — absente ;"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            continue
        said = detect(command, path)
        wanted = canonical(expected)
        if agrees(said, wanted, has_bom):
            right += 1
            print(f"  {GREEN}✓{RESET} {path.name:<22} {said}")
        else:
            confusions.append((path.name, wanted, said))
            print(f"  {RED}✗{RESET} {path.name:<22} {said}  au lieu de {wanted}")

    total = len(fixtures)
    part = f"{100 * right / total:.0f} %" if total else "—"
    print(f"\n  {right}/{total} — {part}")
    if confusions:
        print("\n  confusions :")
        for name, wanted, said in confusions:
            print(f"    {wanted} lu comme {said}   ({name})")
    return right, total


def report_private(commands):
    """Ce que le corpus privé peut dire, et il ne peut pas dire un taux."""
    files = sorted(p for p in PRIVATE.rglob("*")
                   if p.is_file() and p.suffix.lower() in (".srt", ".vtt"))
    if not files:
        print(f"\n{BOLD}corpus privé — absent de cette machine{RESET}")
        return

    print(f"\n{BOLD}corpus privé — {len(files)} fichiers, sans étiquettes{RESET}")
    print("  aucun taux : il n'y a pas de vérité à confronter, seulement ce qui")
    print("  se vérifie sans elle.")

    for command in commands:
        answers = collections.Counter()
        readable = 0
        for path in files:
            said = detect(command, path)
            answers[said] += 1
            if decodes(path, said):
                readable += 1
        print(f"\n  {BOLD}{command}{RESET}")
        for name, count in answers.most_common():
            print(f"    {count:3d}  {name}")
        print(f"    {readable}/{len(files)} réponses sous lesquelles le fichier"
              " se décode entièrement")

    if len(commands) > 1:
        disagreements = 0
        for path in files:
            said = {detect(command, path) for command in commands}
            if len(said) > 1:
                disagreements += 1
        print(f"\n  les détecteurs se contredisent sur {disagreements}"
              f" fichier(s) sur {len(files)}")


def main():
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument("--detector", action="append", required=True,
                        help="commande de détection, « {} » pour le chemin ; répétable")
    parser.add_argument("--prive", action="store_true",
                        help="ajoute le rapport sur le corpus privé, en agrégats")
    options = parser.parse_args()

    right = total = 0
    for command in options.detector:
        print(f"{BOLD}détecteur : {command}{RESET}\n")
        got, seen = score_labelled(command)
        right, total = right + got, total + seen
        print()

    if options.prive:
        report_private(options.detector)

    return 0 if total and right == total else 1


if __name__ == "__main__":
    sys.exit(main())
