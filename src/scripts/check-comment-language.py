#!/usr/bin/env python3
"""Compte les commentaires français restés dans le C++, et refuse qu'ils montent.

**La frontière du projet est celle du fichier, pas celle du sujet.** Le C++ —
identifiants et commentaires — est en anglais ; tout le reste est en français :
documentation, specs, ADR, manuels, messages de commit, scripts, système de
construction. Voir `CLAUDE.md`.

Ce contrôle-ci ne tient qu'une moitié de cette frontière, celle qui a dérivé.
L'autre n'en a pas besoin : les scripts sont français à 1 971 lignes contre 6,
et le système de construction à 386 contre 4. Rien n'y bouge.

## Pourquoi un cliquet plutôt qu'un refus

Mille cent soixante et une lignes françaises vivent encore dans le C++ au jour
où ce contrôle est écrit. Un refus sec rendrait l'arbre rouge à l'instant même
où l'on pose la règle, et la règle serait débranchée avant d'avoir servi. Le
cliquet, lui, tient dès le premier jour : **le compte ne peut que descendre.**

C'est la mécanique du cliquet de couverture — `docs/mesures/couverture.md` — et
elle a la même faiblesse, qu'il vaut mieux écrire que taire : **un compte global
se compense.** Un fichier traduit paie une dérive ailleurs, et le total ne bouge
pas. Ce qui l'empêche en pratique est que l'échec nomme les fichiers, et qu'une
traduction et une dérive dans la même pull request se voient à la relecture.

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

Sortie : le compte. Code de retour 1 s'il dépasse le cliquet.
"""

from __future__ import annotations

import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
SOURCES = REPO_ROOT / "src"

# **Le cliquet, et il ne remonte jamais.** Le baisser est le geste normal d'une
# pull request qui traduit : le nouveau compte s'écrit ici, dans le même diff
# que la traduction. Il n'y a pas de `make ratchet` pour celui-ci — ce serait
# offrir un bouton à ce qui doit rester une ligne relue.
#
# Mesuré le 2026-09-05, à la décision de l'issue #312. La descente est l'issue
# #325 ; le jour où ce nombre vaut zéro, le cliquet devient un refus sec et la
# moitié de cet en-tête tombe.
RATCHET = 1161

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
            f"{red}✗{reset} {total} lignes de commentaire françaises dans le C++,"
            f" contre {RATCHET} au cliquet",
            file=sys.stderr,
        )
        for source, lines in sorted(counted.items(), key=lambda pair: -pair[1])[:10]:
            print(f"    {lines:5}  {source}", file=sys.stderr)
        print(
            "    le C++ de ce dépôt s'écrit en anglais — voir CLAUDE.md et l'issue #312.",
            file=sys.stderr,
        )
        return 1

    print(f"{green}✓{reset} {total} lignes de commentaire françaises dans le C++,"
          f" cliquet à {RATCHET}")
    if total < RATCHET:
        print(f"    il a baissé : porter {total} dans RATCHET, "
              f"{sys.argv[0].split('/')[-1]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
