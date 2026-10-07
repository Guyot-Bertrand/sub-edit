#!/usr/bin/env python3
"""Vérifie que le manuel de `subedit-gui` dit les raccourcis que la fenêtre déclare.

**Ce que `make check` tenait déjà** : qu'aucune paire d'actions ne partage un raccourci
(`no two actions answer the same shortcut`, `window_actions_test.cpp`), donc les nouvelles
dès qu'elles entrent dans `WindowActions`. **Ce qu'il ne tenait pas** : que le manuel les
dise. Ses raccourcis sont écrits à la main, dans des tableaux, et rien ne les confrontait à
la fenêtre — le défaut que `check-cli-manual.py` a réglé pour la ligne de commande (#546),
et que la phase 14 aggrave : une quinzaine de gestes au clavier ne se tiennent pas à l'œil
(issue #612).

Ce qui est confronté :

    ce que la fenêtre DÉCLARE   `subedit_list_shortcuts` — la vraie `WindowActions`, sans écran,
                                chaque action avec ses raccourcis
    ce que le manuel ÉCRIT      docs/manual/subedit-gui/*.md

**La forme que le manuel doit avoir**, pour que la confrontation ne soit pas un `grep` : un
tableau dont **une colonne s'intitule « Raccourci »** et dont une autre — « Commande »,
« Entrée », « Action » ou « Entrée du menu » — nomme l'action telle que le menu la libelle,
éventuellement précédée de son menu (`Video ▸ Play / Pause`). Le raccourci est écrit entre
apostrophes inverses (`Ctrl+P`) ; « aucun » ou « — » dit qu'il n'y en a pas. Le contrôle lit
ces tableaux, et eux seuls : la prose cite des touches qui ne sont pas des actions de la
fenêtre (`F2` ouvre l'éditeur d'une cellule, par exemple).

Erreurs (code 1), chacune nommée :

    ACTION INCONNUE        une ligne de tableau nomme une action que la fenêtre n'a pas.
    RACCOURCI INCONNU      une ligne de tableau écrit un raccourci que son action ne répond pas.
    RACCOURCI OUBLIÉ       une ligne de tableau dit « aucun », ou n'écrit rien, d'une action
                           qui en a un.
    RACCOURCI NON DOCUMENTÉ   une action répond à un raccourci, et aucun de ses raccourcis
                           n'est écrit nulle part dans le manuel de la fenêtre.

**Un seul des raccourcis d'une action suffit à la documenter** : Qt en donne plusieurs selon la
plateforme (`Redo` répond à trois), et exiger le manuel de tous les dire serait lui faire
décrire une plateforme plutôt que le programme. Celui qu'il écrit, en revanche, doit en être un.

**Ce que le contrôle ne couvre pas, et ne prétend pas couvrir** : ce que fait un raccourci, où
il vit, les touches des éditeurs et des dialogues, qui ne sont pas des actions de la fenêtre.
Il dit *quels raccourcis existent*, jamais *ce qu'ils font*.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# Les en-têtes de la colonne qui nomme l'action.
LABEL_HEADERS = ("Commande", "Entrée", "Action", "Entrée du menu")
SHORTCUT_HEADER = "Raccourci"

# Ce que Qt écrit et ce que le manuel écrit pour la même touche : les deux se ramènent à une forme.
KEY_NAMES = {
    "PgUp": "PageUp",
    "PgDown": "PageDown",
    "Ins": "Insert",
    "Del": "Delete",
    "Esc": "Escape",
    "Return": "Enter",
}

NO_SHORTCUT = {"", "—", "-", "aucun", "aucune"}

# Un raccourci écrit comme du code : `Ctrl+Shift+S`, `F1`, `Del`.
CODE_SPAN = re.compile(r"`([^`]+)`")
# Une séquence de touches : une ou plusieurs touches liées par `+`.
SEQUENCE = re.compile(r"^(?:[A-Za-z0-9]+\+)*[A-Za-z0-9]+$")


def canonical(sequence: str) -> str:
    """La forme commune d'une séquence : les noms de touches longs, les modificateurs tels quels."""
    return "+".join(KEY_NAMES.get(part, part) for part in sequence.strip().split("+"))


def declared(binary: pathlib.Path) -> tuple[dict[str, set[str]], set[str]]:
    """Ce que la fenêtre déclare : les raccourcis de chaque action nommée, et tous les raccourcis."""
    result = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
    if result.returncode != 0:
        sys.exit(f"{binary} a échoué : {result.stderr.strip()}")

    by_label: dict[str, set[str]] = {}
    every: set[str] = set()
    for line in result.stdout.splitlines():
        label, _, sequences = line.partition("\t")
        keys = {canonical(part) for part in sequences.split(", ") if part.strip()}
        every |= keys
        if label:
            by_label.setdefault(label, set()).update(keys)
    return by_label, every


def code_spans(markdown: str) -> list[str]:
    """Les passages en code de la prose, **ligne à ligne** : un bloc de code ouvre et ferme des
    apostrophes inverses par trois, et lire le texte d'un trait désapparie tout ce qui le suit."""
    found: list[str] = []
    fenced = False
    for line in markdown.splitlines():
        if line.lstrip().startswith("```"):
            fenced = not fenced
        elif not fenced:
            found.extend(CODE_SPAN.findall(line))
    return found


def cells(line: str) -> list[str]:
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def written_shortcuts(cell: str) -> set[str] | None:
    """Les raccourcis d'une cellule, ou `None` quand elle dit qu'il n'y en a pas."""
    keys = {canonical(span) for span in code_spans(cell) if SEQUENCE.match(span.strip())}
    if keys:
        return keys
    return None if cell.strip().strip("`*").lower() in NO_SHORTCUT else set()


def action_label(cell: str) -> str:
    """Le libellé que le menu donne : sans les apostrophes, et sans le menu qui le précède."""
    return cell.replace("`", "").split("▸")[-1].strip()


def tables(markdown: str):
    """Les lignes des tableaux à colonne « Raccourci » : (numéro de ligne, libellé, cellule)."""
    lines = markdown.splitlines()
    index = 0
    while index < len(lines):
        header = lines[index]
        if not header.lstrip().startswith("|") or SHORTCUT_HEADER not in cells(header):
            index += 1
            continue

        names = cells(header)
        shortcut_column = names.index(SHORTCUT_HEADER)
        label_column = next((names.index(h) for h in LABEL_HEADERS if h in names), None)
        index += 2  # l'en-tête et sa ligne de séparation
        while index < len(lines) and lines[index].lstrip().startswith("|"):
            row = cells(lines[index])
            if label_column is not None and len(row) > max(shortcut_column, label_column):
                yield index + 1, action_label(row[label_column]), row[shortcut_column]
            index += 1


def check(binary: pathlib.Path, manual: pathlib.Path) -> list[str]:
    by_label, every = declared(binary)
    failures: list[str] = []
    pages = sorted(manual.glob("*.md"))
    all_text = ""

    for page in pages:
        text = page.read_text(encoding="utf-8")
        all_text += text + "\n"

        for number, label, cell in tables(text):
            where = f"{page.name}:{number}"
            if label not in by_label:
                failures.append(f"ACTION INCONNUE : {where} nomme `{label}`, que la fenêtre n'a pas")
                continue

            actual = by_label[label]
            written = written_shortcuts(cell)
            if written is None:
                if actual:
                    failures.append(
                        f"RACCOURCI OUBLIÉ : {where} dit que `{label}` n'en a pas, "
                        f"elle répond à {', '.join(sorted(actual))}"
                    )
                continue
            if not written and actual:
                failures.append(f"RACCOURCI OUBLIÉ : {where} n'écrit pas celui de `{label}` ({', '.join(sorted(actual))})")
            for key in sorted(written - actual):
                failures.append(
                    f"RACCOURCI INCONNU : {where} écrit `{key}` pour `{label}`, "
                    f"qui répond à {', '.join(sorted(actual)) or 'aucun'}"
                )

    # Chaque action qui répond à un raccourci le voit écrit quelque part — sous une des formes de la touche.
    spans = {canonical(span) for span in code_spans(all_text) if SEQUENCE.match(span.strip())}
    named = {key for keys in by_label.values() for key in keys}
    for label, keys in sorted(by_label.items()):
        if keys and not keys & spans:
            failures.append(
                f"RACCOURCI NON DOCUMENTÉ : `{label}` répond à {', '.join(sorted(keys))}, "
                f"qu'aucune page du manuel n'écrit"
            )
    # Les raccourcis des actions sans nom (les touches des onglets) : le manuel les écrit aussi.
    for key in sorted(every - named):
        if key not in spans:
            failures.append(f"RACCOURCI NON DOCUMENTÉ : la fenêtre répond à {key}, qu'aucune page du manuel n'écrit")
    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--binary", type=pathlib.Path,
                        default=REPO_ROOT / "build/dev/bin/subedit_list_shortcuts")
    parser.add_argument("--manual", type=pathlib.Path,
                        default=REPO_ROOT / "docs/manual/subedit-gui")
    arguments = parser.parse_args()

    if not arguments.binary.is_file():
        print(f"binaire introuvable : {arguments.binary}", file=sys.stderr)
        return 2

    failures = check(arguments.binary, arguments.manual)
    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        print(f"\n{len(failures)} écart(s) entre le manuel de subedit-gui et ses raccourcis",
              file=sys.stderr)
        return 1

    print("le manuel de subedit-gui écrit les raccourcis de la fenêtre, et seulement les siens")
    return 0


if __name__ == "__main__":
    sys.exit(main())
