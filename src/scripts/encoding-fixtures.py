#!/usr/bin/env python3
"""Les fixtures d'encodage : les fabriquer, et vérifier qu'elles sont bien ce
que la table dit qu'elles sont.

La phase 8 lit des fichiers qui ne sont pas de l'UTF-8. L'éprouver demande des
fichiers qui **sont** dans l'encodage qu'on leur prête — pas des fichiers UTF-8
qu'on décrit comme tels. Et une fois déposés, personne ne peut les relire dans
un diff pour s'en assurer : c'est le problème que `subtitle-fixtures.py` a résolu
pour les grilles d'images, et c'est le même, en pire — un fichier de six cents
répliques est illisible dans un diff, un fichier en CP1252 ne dit même pas ce
qu'il est.

    --check       (défaut) reconstruit chaque fixture et la compare au disque
    --generate    réécrit les fixtures depuis la table

**La table est la source**, et elle porte tout ce qui distingue un fichier d'un
autre : son encodage, son BOM, sa fin de ligne et son texte. La fabrication en
découle, et `--check` compare **octet par octet**.

**Aucun outil externe** — comme `subtitle-fixtures.py`, et contrairement à
`video-fixtures.sh` qui demande ffmpeg. Les codecs de Python sont déterministes,
donc la reconstruction est reproductible partout et la comparaison peut être
exacte plutôt que de s'en remettre à un tiers.

## Ce que la table couvre, et pourquoi

Ce n'est pas le jeu d'encodages de la phase — c'est le cadrage (#288) qui le
tranchera. C'est le jeu qui permet de **distinguer les cas qu'un lecteur doit
savoir distinguer** :

  UTF-8 nu, UTF-8 à BOM      le cas d'aujourd'hui, et le seul déjà lu
  UTF-16 LE et BE, à BOM     le BOM y porte aussi l'ordre des octets, ce qu'un
                             booléen `hadUtf8Bom` ne sait pas dire
  Latin-1 et CP1252          **la paire qui compte** : identiques hors de la
                             plage 0x80–0x9f, donc indistinguables sur un texte
                             qui n'en use pas — une détection doit pouvoir le
                             dire plutôt que trancher
  CP1250                     l'Europe centrale, où les mêmes octets portent
                             d'autres lettres
  KOI8-R                     un mono-octet non latin, qu'aucune heuristique
                             latine ne peut confondre

**Chaque fixture porte les mêmes répliques**, à l'accentuation près de ce que
son encodage sait écrire. C'est ce qui permet de comparer une lecture à une
autre sans comparer deux textes différents.
"""

from __future__ import annotations

import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
FIXTURE_DIR = REPO_ROOT / "src" / "test" / "data" / "encodages"

GREEN = "\033[32m"
RED = "\033[31m"
BOLD = "\033[1m"
RESET = "\033[0m"

NEWLINES = {"lf": "\n", "crlf": "\r\n", "cr": "\r"}

BOMS = {
    "utf-8": b"\xef\xbb\xbf",
    "utf-16-le": b"\xff\xfe",
    "utf-16-be": b"\xfe\xff",
}

# Le texte des répliques, par jeu de caractères qu'il faut. Les trois disent la
# même chose ; ce qui change est ce que l'encodage sait écrire.
LATIN = [
    "Le port était vide à cette heure-là.",
    "Où êtes-vous passé ?",
    "Ça n'a pas duré très longtemps.",
]
CENTRAL = [
    "Přišel jsem pozdě.",
    "Gdzie jesteś?",
    "Nem tudom, hová ment.",
]
CYRILLIC = [
    "Порт был пуст в этот час.",
    "Где вы были?",
    "Это длилось недолго.",
]

# nom, encodage, BOM, fin de ligne, répliques, ce que la fixture montre
FIXTURES = [
    ("utf-8-lf.srt", "utf-8", False, "lf", LATIN,
     "le cas d'aujourd'hui, et la référence des autres"),
    ("utf-8-bom-crlf.srt", "utf-8", True, "crlf", LATIN,
     "ce qu'un éditeur Windows dépose"),
    ("utf-16-le-bom.srt", "utf-16-le", True, "crlf", LATIN,
     "le BOM y porte l'ordre des octets, pas seulement l'encodage"),
    ("utf-16-be-bom.srt", "utf-16-be", True, "crlf", LATIN,
     "l'autre ordre, que rien ne distingue du premier sans son BOM"),
    ("latin1.srt", "iso-8859-1", False, "lf", LATIN,
     "l'Europe de l'Ouest, sans un octet dans la plage 0x80-0x9f"),
    ("cp1252.srt", "cp1252", False, "lf", LATIN + ["Il a dit « non »… puis il est parti."],
     "la même famille, mais avec la plage 0x80-0x9f — d'où la dernière réplique"),
    ("cp1250.srt", "cp1250", False, "lf", CENTRAL,
     "l'Europe centrale : les mêmes octets, d'autres lettres"),
    ("koi8-r.srt", "koi8-r", False, "lf", CYRILLIC,
     "un mono-octet non latin, qu'aucune heuristique latine ne confond"),
    ("cr-mac.srt", "utf-8", False, "cr", LATIN,
     "le retour chariot seul, la fin de ligne du Mac OS classique"),
]


def render(lines, newline):
    """Un fichier SubRip, dans la forme que le projet écrit."""
    parts = []
    for index, text in enumerate(lines, start=1):
        start = index * 2
        parts.append(
            f"{index}{newline}"
            f"00:00:{start:02d},000 --> 00:00:{start + 1:02d},000{newline}"
            f"{text}{newline}{newline}"
        )
    return "".join(parts)


def build(encoding, bom, newline_name, lines):
    """Les octets exacts d'une fixture."""
    content = render(lines, NEWLINES[newline_name])
    body = content.encode(encoding)
    return (BOMS[encoding] if bom else b"") + body


def generate():
    print(f"{BOLD}fabrication des fixtures d'encodage{RESET}")
    FIXTURE_DIR.mkdir(parents=True, exist_ok=True)
    for name, encoding, bom, newline, lines, description in FIXTURES:
        data = build(encoding, bom, newline, lines)
        (FIXTURE_DIR / name).write_bytes(data)
        print(f"  {GREEN}✓{RESET} {name} — {encoding}"
              f"{', BOM' if bom else ''}, {newline}, {len(data)} octets — {description}")
    return 0


def check():
    failures = 0
    for name, encoding, bom, newline, lines, _ in FIXTURES:
        path = FIXTURE_DIR / name
        if not path.is_file():
            print(f"  {RED}✗{RESET} {name} — absente ;"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            failures += 1
            continue

        expected = build(encoding, bom, newline, lines)
        actual = path.read_bytes()
        if actual != expected:
            # **Le premier octet qui diffère, et non « le fichier a changé ».**
            # Un fichier d'encodage n'a pas de diff lisible : sans cette
            # position, la seule façon de comprendre est de le désassembler à la
            # main.
            where = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b),
                         min(len(actual), len(expected)))
            print(f"  {RED}✗{RESET} {name} — le fichier n'est plus ce que la table dit,"
                  f" au premier écart en octet {where}"
                  f" ({len(actual)} octets contre {len(expected)} attendus) ;"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            failures += 1
            continue

        print(f"  {GREEN}✓{RESET} {name} — {encoding}"
              f"{', BOM' if bom else ''}, {newline}, {len(actual)} octets")

    if failures:
        print(f"\n{RED}✗{RESET} {failures} fixture(s) d'encodage ne sont plus"
              " ce que la table annonce", file=sys.stderr)
        return 1
    return 0


def main():
    argument = sys.argv[1] if len(sys.argv) > 1 else "--check"
    if argument == "--check":
        return check()
    if argument == "--generate":
        return generate()
    print(f"usage : {Path(sys.argv[0]).name} [--check|--generate]", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
