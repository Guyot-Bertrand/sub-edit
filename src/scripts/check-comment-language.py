#!/usr/bin/env python3
"""Refuse un commentaire français dans le C++.

**La frontière du projet est celle du fichier, pas celle du sujet.** Le C++ —
identifiants et commentaires — est en anglais ; tout le reste est en français :
documentation, specs, ADR, manuels, messages de commit, scripts, système de
construction. Voir `CLAUDE.md`.

Ce contrôle-ci ne tient qu'une moitié de cette frontière, celle qui a dérivé.
L'autre n'en a pas besoin : les scripts sont français à 1 971 lignes contre 6,
et le système de construction à 386 contre 4. Rien n'y bouge.

## Il fut un cliquet, et il ne l'est plus

Mille cent soixante et une lignes françaises vivaient dans le C++ le jour où ce
contrôle a été écrit — issue #312. Un refus sec aurait rendu l'arbre rouge à
l'instant où la règle était posée, et la règle aurait été débranchée avant
d'avoir servi ; un cliquet, lui, tenait dès le premier jour, le compte ne
pouvant que descendre.

**La descente est faite** — issue #325, en deux pull requests : `screenshots.cpp`
d'abord, puis les quarante-neuf autres fichiers. Le cliquet a donc rendu ce
qu'on lui demandait et cessé d'être utile : ce qui reste est un refus, et il
n'y a plus de nombre à relire.

## Ce qui est reconnu, et ce qui ne l'est pas

Une ligne de commentaire est classée par **vote de ses mots outils** : ceux qui
n'ont pas d'homographe dans l'autre langue, plus les élisions — `l'`, `d'`,
`qu'` — qui n'existent qu'en français. La majorité l'emporte, une égalité ne
compte pour rien.

**Ce n'est pas un caractère hors ASCII qui décide**, contrairement au contrôle
des intitulés de tests, et c'est la seule différence entre les deux. Les
commentaires anglais de ce dépôt sont pleins de tirets cadratins et de
guillemets français ; les compter ferait de chaque paragraphe anglais un
paragraphe français.

**Le seuil de douze caractères** écarte les fins de ligne — `// namespace`,
`// NOLINT` — dont aucun vote ne dirait rien.

Sortie : le compte. Code de retour 1 s'il n'est pas nul.
"""

from __future__ import annotations

import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
SOURCES = REPO_ROOT / "src"

# **Zéro, et ce n'est plus un cliquet.** La constante reste parce que le message
# d'échec la cite et qu'elle nomme ce qui est attendu ; elle ne bouge plus.
#
# L'histoire, pour qui se demanderait pourquoi ce fichier parle d'un cliquet :
# 1 161 lignes à la décision de #312, 1 095 après la pull request des #309 et
# #321, 1 093 après celle des #316 et #317, 984 après `screenshots.cpp`, et zéro
# à la fin de #325.
RATCHET = 0

# Les mots outils français sans homographe anglais, et les élisions. « on »,
# « son » et « sa » en sont écartés : ce sont des mots anglais courants, et les
# garder classerait français une ligne anglaise sur dix.
FRENCH = re.compile(
    r"\b(le|la|les|une|des|qui|que|qu|pour|dans|est|pas|ce|cette|cet|plutôt|donc"
    r"|elle|aux|du|sans|avec|ne|par|quand|même|leur|nous|où|être|fait|ici|tout"
    r"|tous|cela|au|il|ils|elles|deux|trois|chaque|celui|celle|ceux|puis|alors"
    r"|entre|sous|vers|depuis|jamais|toujours|rien|autre|autres|comme|mais)\b"
    r"|\b[lnjdmtcsq]'",
    re.IGNORECASE,
)

# Leurs équivalents anglais, choisis de la même façon.
ENGLISH = re.compile(
    r"\b(the|is|are|of|that|and|to|it|what|which|with|for|not|its|this|these"
    r"|those|has|have|had|does|do|but|from|would|will|be|been|when|so|than"
    r"|then|only|rather|because|one|two|three|each|both|there|here|where|who"
    r"|why|how|into|over|under|after|before|never|always|nothing|else|same)\b",
    re.IGNORECASE,
)

# Ce qui suit ce nombre de caractères vaut un vote ; en deçà, la ligne est un
# repère de fin de bloc et non une phrase.
SHORTEST_SENTENCE = 12


def french_lines(source: pathlib.Path) -> int:
    """Le nombre de lignes de commentaire françaises de ce fichier."""
    french = 0
    for line in source.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped.startswith("//"):
            continue

        sentence = stripped.lstrip("/").strip()
        if len(sentence) < SHORTEST_SENTENCE:
            continue

        if len(FRENCH.findall(sentence)) > len(ENGLISH.findall(sentence)):
            french += 1

    return french


def main() -> int:
    counted = {}
    for source in sorted(SOURCES.rglob("*")):
        if source.suffix not in (".cpp", ".hpp"):
            continue
        if lines := french_lines(source):
            counted[source.relative_to(REPO_ROOT)] = lines

    total = sum(counted.values())
    green, red, reset = "\033[32m", "\033[31m", "\033[0m"

    if total > RATCHET:
        print(
            f"{red}✗{reset} {total} ligne(s) de commentaire françaises dans le C++,"
            " qui s'écrit en anglais",
            file=sys.stderr,
        )
        for source, lines in sorted(counted.items(), key=lambda pair: -pair[1])[:10]:
            print(f"    {lines:5}  {source}", file=sys.stderr)
        print(
            "    le C++ de ce dépôt s'écrit en anglais — voir CLAUDE.md et l'issue #312.",
            file=sys.stderr,
        )
        return 1

    print(f"{green}✓{reset} pas une ligne de commentaire française dans le C++")
    return 0


if __name__ == "__main__":
    sys.exit(main())
