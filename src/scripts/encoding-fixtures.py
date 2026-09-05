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

## Les trois courtes, et ce qu'elles ajoutent

Les neuf premières font entre cent cinquante et quatre cents octets, et un
score relevé sur elles seules **ne mesure que le cas facile** — c'est le défaut
qu'a soulevé l'issue #310. Trois fixtures d'une seule réplique s'y ajoutent :
`latin1-court`, `cp1250-court`, `koi8-r-court`.

**Ce n'est pas la longueur qui décide, ce sont les octets hauts**, et la mesure
le dit — `score-encoding-detection.py --par-longueur`. Une réplique en porte
trois là où trois répliques en portent neuf, et c'est de ce côté-là que la
détection cesse de savoir.

Chacune porte **la première réplique de son jeu**, et ce choix est une règle
plutôt qu'un tri : choisir la réplique après avoir vu la réponse ferait dire au
corpus le score qu'on voulait lire. `cp1250-court` en revient fausse ; elle
reste.

## La bilingue, et c'est elle qui a coûté le plus

`koi8-r-rare` est à l'opposé des trois courtes : mille trois cents octets, dont
dix-neuf hors ASCII. Vingt répliques d'anglais, une de russe — un carton, une
note de traducteur, ce qu'un fichier réel porte souvent.

Elle revenait en `ISO-8859-1`, et le russe avec elle, en latin accentué. Ce
n'était pas un manque d'information : `uchardet` répondait juste sur le même
fichier. C'était la masse d'ASCII qui pesait dans une statistique de lettres où
elle n'avait rien à dire — ADR 0028.
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

# De l'anglais, qui ne porte aucun octet haut : ce qui remplit un fichier
# bilingue autour des rares répliques qui en portent. Une seule fixture s'en
# sert, et c'est celle qui compte le plus — #310.
ENGLISH = [
    "I don't know what you mean.",
    "The harbour was empty at that hour.",
    "Where did he go?",
    "It didn't last very long.",
    "She said the same thing again, quietly.",
    "Do you think they will come back?",
    "They told me it was settled.",
    "The hotel had been closed since February.",
    "Stop it, please.",
    "After that, nobody spoke.",
    "A strange idea, all the same.",
    "It is already dark on the coast.",
    "Nobody answered the telephone.",
    "We waited until the rain stopped.",
    "That was not what he promised.",
    "The train leaves in ten minutes.",
    "She never mentioned it again.",
    "Somebody had opened the window.",
    "You should have told me sooner.",
    "It made no difference at all.",
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
    # Les trois courtes — #310. La première réplique de leur jeu, et rien
    # d'autre : le choix est une règle et non un tri, sans quoi le corpus
    # dirait le score qu'on aurait voulu lire.
    ("latin1-court.srt", "iso-8859-1", False, "lf", LATIN[:1],
     "la même, sur une réplique : trois octets hauts au lieu de neuf"),
    ("cp1250-court.srt", "cp1250", False, "lf", CENTRAL[:1],
     "l'Europe centrale sur une réplique, et la falaise commence là"),
    ("koi8-r-court.srt", "koi8-r", False, "lf", CYRILLIC[:1],
     "le cyrillique sur une réplique, qui tient encore"),
    # Le fichier bilingue — #310, et c'est la fixture la plus instructive du
    # corpus. Vingt répliques d'anglais et une de cyrillique : de la statistique
    # de lettres, la masse d'ASCII est tout ce qu'il y a à peser, et la réponse
    # était ISO-8859-1 avant que la détection cesse de peser ce qui ne
    # discrimine pas.
    ("koi8-r-rare.srt", "koi8-r", False, "lf", ENGLISH + CYRILLIC[:1],
     "un carton cyrillique dans un fichier anglais, que la masse d'ASCII noyait"),
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
