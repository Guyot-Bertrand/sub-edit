#!/usr/bin/env python3
"""Vérifie que le manuel de `subedit-cli` et son `--help` disent les mêmes noms.

**Ce que `manual-check` tenait déjà** : chaque page de sous-commande porte un
exemple `subedit-cli <sous-commande> --help`, rejoué, si bien qu'une option
ajoutée change l'aide, périme le bloc, et fait échouer la porte. **Ce qu'il ne
tenait pas** : les tableaux « Options » écrits à la main, une sous-commande
sans page, la liste des sous-commandes d'`invocation.md`, et les options de
la page `subedit-cli(1)`. La phase 13 ajoute des sous-commandes et des dizaines
d'options : c'est là que la dérive se paie (issue #546).

Ce qui est confronté :

    ce que le binaire DIT      `subedit-cli --help` et `subedit-cli <sc> --help`
    ce que le manuel ÉCRIT     docs/manual/subedit-cli/*.md, et packaging/subedit-cli.1.in

Erreurs (code 1), chacune nommée :

    PAGE ABSENTE        une sous-commande du binaire n'a pas de page `<nom>.md`.
    OPTION NON DOCUMENTÉE   une option longue de l'aide ne figure dans aucune ligne
                        de tableau de la page de sa sous-commande.
    OPTION INCONNUE     une option du tableau « Option » d'une page n'existe pas
                        dans l'aide de la sous-commande.
    SOUS-COMMANDE NON LISTÉE / INCONNUE   le tableau d'`invocation.md` et le binaire
                        ne nomment pas les mêmes sous-commandes.
    LANGUE SANS VOCABULAIRE  une page est dans une langue qui n'a pas de table (`doc_vocabulary.py`) :
                        elle ne serait pas lue, et le contrôle ne dirait rien.
    OPTION GLOBALE ABSENTE   une option globale du binaire n'est écrite ni dans
                        `invocation.md` ni dans la page `subedit-cli(1)`.

**Ce que le contrôle ne couvre pas, et ne prétend pas couvrir** : les valeurs
acceptées, les défauts, les bornes et la section « Erreurs » restent écrits à la
main — l'aide ne les porte pas sous une forme qu'on puisse lire sans la
deviner. Le contrôle dit donc *quels noms existent*, jamais *ce qu'ils font*.
"""

from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

import doc_vocabulary

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# Une option longue de l'aide : « --by TEXT », « -q,--quiet », « --to TEXT:{…} ».
LONG_OPTION = re.compile(r"(?<![\w-])(--[a-z][a-z0-9-]*)")
# Une option longue écrite entre apostrophes inverses dans le manuel.
BACKTICKED_OPTION = re.compile(r"`(--[a-z][a-z0-9-]*)[^`]*`")
# Le nom, avec ou sans le lien vers sa page : « | [`shift`](shift.md) | … ».
SUBCOMMAND_ROW = re.compile(r"^\|\s*\[?`([a-z][a-z0-9-]*)`\]?")


def run_help(binary: pathlib.Path, *args: str) -> str:
    """La sortie de `binary args --help`."""
    result = subprocess.run(
        [str(binary), *args, "--help"], capture_output=True, text=True, check=False
    )
    return result.stdout + result.stderr


def section(help_text: str, title: str) -> list[str]:
    """Les lignes d'une section de l'aide (`Options:`, `Subcommands:`…)."""
    lines: list[str] = []
    inside = False
    for line in help_text.splitlines():
        if line.rstrip().endswith(":") and not line.startswith(" "):
            inside = line.strip() == title
            continue
        if inside:
            if not line.strip():
                if lines:
                    break
                continue
            lines.append(line)
    return lines


def long_options(help_text: str) -> set[str]:
    """Les options longues de l'aide, `--help` exclue : chaque sous-commande a la sienne."""
    options: set[str] = set()
    for line in section(help_text, "Options:"):
        # La seule colonne des noms : les descriptions citent d'autres mots.
        names = re.split(r"\s{2,}", line.strip(), maxsplit=1)[0]
        options.update(LONG_OPTION.findall(names))
    options.discard("--help")
    return options


def subcommands(help_text: str) -> list[str]:
    names: list[str] = []
    for line in section(help_text, "Subcommands:"):
        names.append(line.split()[0])
    return names


def table_rows(markdown: str) -> list[str]:
    return [line for line in markdown.splitlines() if line.lstrip().startswith("|")]


def documented_options(markdown: str) -> set[str]:
    """Toute option longue écrite entre apostrophes inverses dans une ligne de tableau."""
    found: set[str] = set()
    for row in table_rows(markdown):
        found.update(BACKTICKED_OPTION.findall(row))
        for cell in re.findall(r"`(--[a-z][a-z0-9-]*)", row):
            found.add(cell)
    return found


def option_column(markdown: str, option_header: str) -> set[str]:
    """Les options de la première colonne des tableaux dont l'en-tête est celui des options."""
    found: set[str] = set()
    inside = False
    for line in markdown.splitlines():
        stripped = line.strip()
        if not stripped.startswith("|"):
            inside = False
            continue
        cells = [cell.strip() for cell in stripped.strip("|").split("|")]
        if cells and cells[0] == option_header:
            inside = True
            continue
        if inside and cells:
            found.update(re.findall(r"`(--[a-z][a-z0-9-]*)", cells[0]))
    return found


def check(binary: pathlib.Path, manual: pathlib.Path, man_page: pathlib.Path,
          vocabularies: dict[str, dict]) -> list[str]:
    failures: list[str] = []
    unread: dict[str, list[str]] = {}
    top_help = run_help(binary)
    names = subcommands(top_help)
    if not names:
        return [f"AUCUNE SOUS-COMMANDE : `{binary} --help` n'en liste aucune"]

    for name in names:
        page = manual / f"{name}.md"
        if not page.is_file():
            failures.append(f"PAGE ABSENTE : la sous-commande `{name}` n'a pas de {page.name}")
            continue
        text = page.read_text(encoding="utf-8")
        language = doc_vocabulary.language_of(page, text)
        if language not in vocabularies:
            unread.setdefault(language, []).append(page.name)
            continue
        from_help = long_options(run_help(binary, name))
        written = documented_options(text)
        for option in sorted(from_help - written):
            failures.append(
                f"OPTION NON DOCUMENTÉE : `{name} {option}` est dans l'aide, "
                f"pas dans un tableau de {page.name}"
            )
        for option in sorted(option_column(text, vocabularies[language]["option_header"]) - from_help):
            failures.append(
                f"OPTION INCONNUE : {page.name} documente `{option}`, "
                f"que l'aide de `{name}` ne connaît pas"
            )

    invocation = manual / "invocation.md"
    invocation_text = invocation.read_text(encoding="utf-8") if invocation.is_file() else ""
    listed: set[str] = set()
    inside = False
    invocation_language = doc_vocabulary.language_of(invocation, invocation_text)
    if invocation_language not in vocabularies:
        unread.setdefault(invocation_language, []).append(invocation.name)
        subcommand_header = None
    else:
        subcommand_header = vocabularies[invocation_language]["subcommand_header"]
    for line in invocation_text.splitlines():
        if subcommand_header is not None and line.startswith(f"| {subcommand_header}"):
            inside = True
            continue
        if inside:
            match = SUBCOMMAND_ROW.match(line)
            if match:
                listed.add(match.group(1))
            elif not line.startswith("|"):
                inside = False
    failures.extend(doc_vocabulary.missing_table(vocabularies, unread))
    if subcommand_header is not None:
        for name in sorted(set(names) - listed):
            failures.append(f"SOUS-COMMANDE NON LISTÉE : `{name}` n'est pas dans le tableau d'{invocation.name}")
        for name in sorted(listed - set(names)):
            failures.append(f"SOUS-COMMANDE INCONNUE : {invocation.name} liste `{name}`, que le binaire ne connaît pas")

    man_text = man_page.read_text(encoding="utf-8") if man_page.is_file() else ""
    for option in sorted(long_options(top_help)):
        if f"`{option}" not in invocation_text:
            failures.append(f"OPTION GLOBALE ABSENTE : `{option}` n'est pas écrite dans {invocation.name}")
        if option.replace("-", r"\-") not in man_text:
            failures.append(f"OPTION GLOBALE ABSENTE : `{option}` n'est pas écrite dans {man_page.name}")
    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--binary", type=pathlib.Path,
                        default=REPO_ROOT / "build/dev/bin/subedit-cli")
    parser.add_argument("--manual", type=pathlib.Path,
                        default=REPO_ROOT / "docs/manual/subedit-cli")
    parser.add_argument("--man-page", type=pathlib.Path,
                        default=REPO_ROOT / "packaging/subedit-cli.1.in")
    parser.add_argument("--vocabulary", type=pathlib.Path,
                        help="un fichier JSON {langue: table} qui complète les tables livrées")
    arguments = parser.parse_args()

    if not arguments.binary.is_file():
        print(f"binaire introuvable : {arguments.binary}", file=sys.stderr)
        return 2

    failures = check(arguments.binary, arguments.manual, arguments.man_page,
                     doc_vocabulary.load_tables(arguments.vocabulary))
    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        print(f"\n{len(failures)} écart(s) entre le manuel de subedit-cli et son --help",
              file=sys.stderr)
        return 1

    print("le manuel de subedit-cli et son --help nomment les mêmes sous-commandes et options")
    return 0


if __name__ == "__main__":
    sys.exit(main())
