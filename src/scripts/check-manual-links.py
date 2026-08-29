#!/usr/bin/env python3
"""Vérifie que les renvois du manuel désignent quelque chose.

**Le manuel est la seule partie du dépôt dont la justesse repose entièrement sur
la relecture.** Ses blocs `console` sont engendrés en exécutant la commande, ses
images en photographiant la vraie fenêtre — mais ses renvois internes, eux, ne
sont vérifiés par rien. Une section renommée laisse derrière elle des ancres qui
ne mènent nulle part, et un lien mort s'affiche aussi proprement qu'un lien
juste.

Ce qui est confronté :

    ce que le manuel POINTE      docs/manual/**/*.md, liens Markdown relatifs
    ce qui EXISTE                le fichier visé, et l'ancre visée dans ce fichier

Erreurs (code 1)

    CIBLE ABSENTE  le lien désigne un fichier qui n'existe pas.
    ANCRE ABSENTE  le fichier existe, mais aucun titre n'y produit cette ancre.

Les liens externes — `http:`, `https:`, `mailto:` — ne sont pas suivis : un
contrôle qui sort sur le réseau échoue pour des raisons qui ne regardent pas le
dépôt. Les images sont laissées à `check-screenshots.py`, qui en sait plus.
"""

from __future__ import annotations

import pathlib
import re
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
MANUAL = REPO_ROOT / "docs/manual"

# [texte](cible) — en excluant les images, qui commencent par « ! ».
LINK = re.compile(r"(?<!!)\[[^\]]*\]\(([^)\s]+)\)")
# Un titre ATX : le niveau, puis son texte.
HEADING = re.compile(r"^(#{1,6})\s+(.*?)\s*$", re.MULTILINE)
# Ce qu'une ancre GitHub ne garde pas : tout sauf lettres, chiffres, espaces,
# tirets et soulignés. Les lettres accentuées sont des lettres, et restent.
DROPPED = re.compile(r"[^\w\s-]", re.UNICODE)

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"


def anchor(title: str) -> str:
    """L'ancre que GitHub donne à ce titre."""
    return DROPPED.sub("", title.strip().lower()).replace(" ", "-")


def anchors(page: pathlib.Path) -> set[str]:
    """Toutes les ancres d'une page, doublons numérotés compris."""
    found: set[str] = set()
    seen: dict[str, int] = {}
    for _, title in HEADING.findall(page.read_text(encoding="utf-8")):
        base = anchor(title)
        rank = seen.get(base, 0)
        seen[base] = rank + 1
        found.add(base if rank == 0 else f"{base}-{rank}")
    return found


def main() -> int:
    errors: list[str] = []
    checked = 0

    for page in sorted(MANUAL.rglob("*.md")):
        here = page.relative_to(REPO_ROOT)
        for target in LINK.findall(page.read_text(encoding="utf-8")):
            if target.startswith(("http:", "https:", "mailto:", "#")):
                # Une ancre nue vise la page elle-même : elle est vérifiée
                # comme les autres, sans partie fichier.
                if not target.startswith("#"):
                    continue
                path, _, fragment = page, None, target[1:]
            else:
                file_part, _, fragment = target.partition("#")
                path = (page.parent / file_part).resolve()

            checked += 1

            if not path.is_file():
                errors.append(f"CIBLE ABSENTE  {here} → {target}")
                continue

            if fragment and path.suffix == ".md" and fragment not in anchors(path):
                errors.append(f"ANCRE ABSENTE  {here} → {target}")

    for line in errors:
        print(f"{RED}✗{RESET} {line}")

    if errors:
        print(f"\n{RED}✗{RESET} {len(errors)} renvoi(s) du manuel ne mènent nulle part")
        return 1

    print(f"{GREEN}✓{RESET} {checked} renvoi(s) du manuel, tous valides")
    return 0


if __name__ == "__main__":
    sys.exit(main())
