#!/usr/bin/env python3
"""Garde-fou de cohérence des captures d'écran du manuel.

Le dispositif a trois pièces mobiles, et rien ne les confrontait :

    ce que le programme de capture ENGENDRE   src/test/tools/screenshots.cpp
    ce que le manuel MONTRE                   docs/manual/**/*.md
    ce qui EXISTE sur le disque               docs/manual/subedit-gui/captures/

Chacun des défauts ci-dessous est **silencieux dans une chaîne verte**, parce
que `compare-screenshots.py` ne voit jamais que les `*.new.png` qu'une exécution
a produits : il ne peut rien savoir d'un manuel qui pointe vers rien, ni d'une
référence que plus rien ne réengendre.

Erreurs (code 1)

    CONTOURNEMENT  une capture écrit directement le nom de la référence au lieu
                   de `<nom>.new.png`, donc le comparateur ne la compare jamais :
                   l'exécution écrase la référence sur place et une régression
                   visuelle entre au manuel sans que personne la voie.
    CASSÉE         le manuel montre une image qui n'existe pas sur le disque.
    FIGÉE          le manuel montre une image que plus rien n'engendre : la
                   référence ne peut plus être réengendrée, donc elle s'éloigne
                   de la vraie fenêtre pour toujours. C'est le défaut le plus
                   coûteux, parce qu'une image périmée s'affiche aussi
                   proprement qu'une image juste.
    PAIRE          une capture claire n'a pas sa sombre : le manuel promet, deux
                   fois, que chaque écran est montré sous les deux palettes, et
                   rien ne tenait cette promesse. Deux cas, une seule erreur :
                   `X-sombre` n'est pas engendrée, ou le manuel montre `X.png`
                   sans montrer `X-sombre.png`. Issue #400, née de deux écrans
                   de la phase 10 ajoutés avec leur seule palette claire.

Un écran qui n'a vraiment qu'une palette s'inscrit dans `SINGLE_PALETTE`, avec
sa raison : une exemption sans motif est une paire oubliée qui s'ignore.

Remarques (code 0)

    INUTILISÉE     une capture qu'aucun manuel ne montre — du temps de
                   construction dépensé pour rien.
    MORTE          un fichier que rien n'engendre et que personne ne montre.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# Les trois emplacements se lisent sous une racine, celle du dépôt par défaut :
# `--root` la déplace vers un jeu écrit à la main, ce qui est ce qui permet à
# `verify-gates.sh` d'éprouver ce contrôle sans toucher aux vraies captures.
TOOL = "src/test/tools/screenshots.cpp"
CAPTURES = "docs/manual/subedit-gui/captures"
MANUAL = "docs/manual"

# Le suffixe qui fait d'une capture la sombre de la claire : `table` et
# `table-sombre`. Un nom calculé échapperait à `produced()`, donc le programme
# de capture écrit les deux en toutes lettres et ce suffixe ne sert qu'ici.
DARK_SUFFIX = "-sombre"

# Les écrans qui n'ont vraiment qu'une palette : le nom de la capture claire,
# sans `.png`, et POURQUOI elle n'a pas de sombre. **Vide aujourd'hui** — les
# onze écrans photographiés le sont sous les deux palettes. Une entrée est une
# décision, pas un contournement : la raison s'écrit, et une relecture la lit.
SINGLE_PALETTE: dict[str, str] = {}

# `capture…(` — le début d'un appel qui photographie. Ce qui suit est lu en
# comptant les parenthèses plutôt qu'avec une expression rationnelle : un
# argument peut lui-même en contenir — `*window.table()` — et clang-format est
# libre de couper l'appel sur quatre lignes le jour où il s'allonge. Un motif
# qui supposerait « une ligne, un appel » cesserait de voir la moitié des
# captures à la première mise en forme, et sans rien dire.
#
# **Le suffixe est accepté depuis l'issue #321**, qui a donné à la boîte
# « Save As… » sa propre fonction — `captureSaveAsStrip`, parce qu'elle
# construit la boîte et n'en garde qu'un bandeau. Un motif ancré sur
# `capture(` exactement ne la voyait pas, et déclarait figées deux références
# que le programme engendre bel et bien : un contrôle qui se trompe de sens est
# pire qu'un contrôle absent, celui-ci ayant en plus le mérite de crier.
CAPTURE_CALL = re.compile(r"\bcapture[A-Za-z]*\(")
# Tout littéral qui nomme un PNG dans le programme de capture.
PNG_LITERAL = re.compile(r'"([^"]*\.png)"')
# ![texte](chemin.png) dans le manuel.
MARKDOWN_IMAGE = re.compile(r"!\[[^\]]*\]\(([^)\s]+\.png)\)")

RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def produced(root: pathlib.Path = REPO_ROOT) -> set[str]:
    """Les captures que le programme engendre, lues dans sa source.

    Le nom est le dernier littéral de l'appel, quels que soient les arguments
    qui le précèdent et la façon dont ils sont mis en forme.
    """
    source = (root / TOOL).read_text(encoding="utf-8")
    names: set[str] = set()

    for call in CAPTURE_CALL.finditer(source):
        arguments = balanced(source, call.end())
        literals = re.findall(r'"([^"]*)"', arguments)
        if literals:
            names.add(literals[-1])

    return names


def balanced(source: str, start: int) -> str:
    """Ce qui va de `start` jusqu'à la parenthèse qui referme celle d'avant."""
    depth = 1
    for index in range(start, len(source)):
        if source[index] == "(":
            depth += 1
        elif source[index] == ")":
            depth -= 1
            if depth == 0:
                return source[start:index]
    return ""


def bypasses(root: pathlib.Path = REPO_ROOT) -> list[str]:
    """Les littéraux PNG du programme qui ne passent pas par `.new.png`.

    Le programme ne doit connaître qu'un seul suffixe. Un `save()` qui écrirait
    « fenetre.png » court-circuiterait le comparateur, et c'est exactement le
    défaut qu'aucun autre contrôle ne verrait.
    """
    return [
        literal
        for literal in PNG_LITERAL.findall((root / TOOL).read_text(encoding="utf-8"))
        if not literal.endswith(".new.png")
    ]


def shown(root: pathlib.Path = REPO_ROOT) -> dict[str, list[pathlib.Path]]:
    """Les images que le manuel montre, et où il les montre."""
    where: dict[str, list[pathlib.Path]] = {}
    for page in sorted((root / MANUAL).rglob("*.md")):
        for reference in MARKDOWN_IMAGE.findall(page.read_text(encoding="utf-8")):
            target = (page.parent / reference).resolve()
            where.setdefault(target.name, []).append(page.relative_to(root))
    return where


def on_disk(root: pathlib.Path = REPO_ROOT) -> set[str]:
    captures = root / CAPTURES
    return {found.name for found in captures.glob("*.png")} if captures.is_dir() else set()


def unpaired(
    engendrées: set[str],
    montrées: dict[str, list[pathlib.Path]],
    exemptions: dict[str, str],
) -> list[str]:
    """Les captures claires dont la sombre manque, engendrée ou montrée.

    « Claire » veut dire : toute capture engendrée ou montrée dont le nom ne
    finit pas par `-sombre`. Les deux ensembles, et non le seul programme : une
    image que le manuel montre sans que rien ne l'engendre est déjà FIGÉE, mais
    la paire qui lui manque est un second défaut, et le dire est gratuit.
    """
    clairs = sorted(
        name
        for name in engendrées | {image.removesuffix(".png") for image in montrées}
        if not name.endswith(DARK_SUFFIX) and name not in exemptions
    )

    errors: list[str] = []
    for name in clairs:
        sombre = f"{name}{DARK_SUFFIX}"
        if sombre not in engendrées:
            errors.append(
                f"PAIRE          {name}.png — {sombre}.png n'est engendrée par personne ;\n"
                f"               chaque écran se montre sous les deux palettes, ou s'exempte\n"
                f"               dans SINGLE_PALETTE, avec sa raison"
            )
        elif f"{name}.png" in montrées and f"{sombre}.png" not in montrées:
            errors.append(
                f"PAIRE          {name}.png — montrée, mais {sombre}.png ne l'est par aucun manuel"
            )
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument(
        "--root",
        type=pathlib.Path,
        default=REPO_ROOT,
        help="la racine sous laquelle lire le programme, le manuel et les captures",
    )
    root = parser.parse_args(argv).root

    engendrées = produced(root)
    montrées = shown(root)
    présentes = on_disk(root)

    errors: list[str] = []
    notices: list[str] = []

    for literal in bypasses(root):
        errors.append(
            f"CONTOURNEMENT  {literal} — le programme écrirait la référence sans passer\n"
            f"               par le comparateur ; le suffixe est « .new.png », toujours"
        )

    for name, pages in sorted(montrées.items()):
        lieux = ", ".join(str(page) for page in pages)
        if name not in présentes:
            errors.append(f"CASSÉE         {name} — montrée par {lieux}, absente du disque")
        elif name.removesuffix(".png") not in engendrées:
            errors.append(
                f"FIGÉE          {name} — montrée par {lieux}, engendrée par personne ;\n"
                f"               plus rien ne la réengendre, elle s'éloignera de la fenêtre"
            )

    errors.extend(unpaired(engendrées, montrées, SINGLE_PALETTE))

    for name in sorted(engendrées):
        if f"{name}.png" not in montrées:
            notices.append(f"INUTILISÉE     {name}.png — engendrée, montrée par aucun manuel")

    for name in sorted(présentes):
        if name.removesuffix(".png") not in engendrées and name not in montrées:
            notices.append(f"MORTE          {name} — ni engendrée, ni montrée")

    for line in notices:
        print(f"{YELLOW}⚠{RESET} {line}")
    for line in errors:
        print(f"{RED}✗{RESET} {line}", file=sys.stderr)

    if errors:
        print(f"{RED}{len(errors)} incohérence(s) de captures{RESET}", file=sys.stderr)
        return 1

    print(
        f"{GREEN}✓{RESET} {len(engendrées)} capture(s) engendrée(s), "
        f"{len(montrées)} montrée(s), rien d'incohérent"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
