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

Remarques (code 0)

    INUTILISÉE     une capture qu'aucun manuel ne montre — du temps de
                   construction dépensé pour rien.
    MORTE          un fichier que rien n'engendre et que personne ne montre.
"""

from __future__ import annotations

import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

TOOL = REPO_ROOT / "src/test/tools/screenshots.cpp"
CAPTURES = REPO_ROOT / "docs/manual/subedit-gui/captures"
MANUAL = REPO_ROOT / "docs/manual"

# `capture(` — le début d'un appel. Ce qui suit est lu en comptant les
# parenthèses plutôt qu'avec une expression rationnelle : un argument peut
# lui-même en contenir — `*window.table()` — et clang-format est libre de
# couper l'appel sur quatre lignes le jour où il s'allonge. Un motif qui
# supposerait « une ligne, un appel » cesserait de voir la moitié des captures
# à la première mise en forme, et sans rien dire.
CAPTURE_CALL = re.compile(r"\bcapture\(")
# Tout littéral qui nomme un PNG dans le programme de capture.
PNG_LITERAL = re.compile(r'"([^"]*\.png)"')
# ![texte](chemin.png) dans le manuel.
MARKDOWN_IMAGE = re.compile(r"!\[[^\]]*\]\(([^)\s]+\.png)\)")

RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def produced() -> set[str]:
    """Les captures que le programme engendre, lues dans sa source.

    Le nom est le dernier littéral de l'appel, quels que soient les arguments
    qui le précèdent et la façon dont ils sont mis en forme.
    """
    source = TOOL.read_text(encoding="utf-8")
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


def bypasses() -> list[str]:
    """Les littéraux PNG du programme qui ne passent pas par `.new.png`.

    Le programme ne doit connaître qu'un seul suffixe. Un `save()` qui écrirait
    « fenetre.png » court-circuiterait le comparateur, et c'est exactement le
    défaut qu'aucun autre contrôle ne verrait.
    """
    return [
        literal
        for literal in PNG_LITERAL.findall(TOOL.read_text(encoding="utf-8"))
        if not literal.endswith(".new.png")
    ]


def shown() -> dict[str, list[pathlib.Path]]:
    """Les images que le manuel montre, et où il les montre."""
    where: dict[str, list[pathlib.Path]] = {}
    for page in sorted(MANUAL.rglob("*.md")):
        for reference in MARKDOWN_IMAGE.findall(page.read_text(encoding="utf-8")):
            target = (page.parent / reference).resolve()
            where.setdefault(target.name, []).append(page.relative_to(REPO_ROOT))
    return where


def on_disk() -> set[str]:
    return {found.name for found in CAPTURES.glob("*.png")} if CAPTURES.is_dir() else set()


def main() -> int:
    engendrées = produced()
    montrées = shown()
    présentes = on_disk()

    errors: list[str] = []
    notices: list[str] = []

    for literal in bypasses():
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
