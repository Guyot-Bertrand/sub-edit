#!/usr/bin/env python3
"""Le vocabulaire que les contrôles du manuel cherchent, une table par langue.

**Un contrôle qui cherche un mot français et n'en trouve pas sur une page anglaise ne
échoue pas : il dit « rien à vérifier ».** C'est la panne que `docs/exigences.md` nomme
(#657). Les contrôles qui lisent le manuel — `check-gui-manual.py`, `check-cli-manual.py` —
ne portent donc aucun mot de page dans leur logique : ils les demandent ici, par langue.

    langue d'une page   déclarée par `<!-- language: en -->` dans ses cinq premières lignes,
                        sinon celle de son arbre : `docs/i18n/<langue>/manual/`, sinon la
                        langue source (`SOURCE_LANGUAGE`)
    vocabulaire         `TABLES[langue]`, complété par un fichier JSON (`--vocabulary`)

**Une page dont la langue n'a pas de table fait échouer le contrôle**, une seule fois par
langue. Ajouter une langue au manuel, c'est ajouter sa table, et rien d'autre ne le permet.
Aucune table n'est livrée pour l'anglais avant que le dépôt ne le parle (#660).

Clés d'une table :

    shortcut_header     l'en-tête de la colonne des raccourcis
    label_headers       les en-têtes possibles de la colonne qui nomme l'action
    no_shortcut_words   les mots qui disent « pas de raccourci » (en plus de « », « — », « - »)
    option_header       l'en-tête de la première colonne des tableaux d'options
    subcommand_header   l'en-tête du tableau des sous-commandes de `invocation.md`
"""

from __future__ import annotations

import json
import pathlib
import re

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent

# La langue des pages qui ne sont ni déclarées ni sous `docs/i18n/` : celle de `docs/manual/`.
# #660 la fera passer à « en » et déplacera le français sous `docs/i18n/fr/`.
SOURCE_LANGUAGE = "fr"

KEYS = ("shortcut_header", "label_headers", "no_shortcut_words", "option_header", "subcommand_header")

TABLES: dict[str, dict] = {
    "fr": {
        "shortcut_header": "Raccourci",
        "label_headers": ["Commande", "Entrée", "Action", "Entrée du menu"],
        "no_shortcut_words": ["aucun", "aucune"],
        "option_header": "Option",
        "subcommand_header": "Sous-commande",
    },
}

DECLARATION = re.compile(r"<!--\s*language:\s*([A-Za-z][A-Za-z_-]*)\s*-->")
TREE = re.compile(r"(?:^|/)i18n/([^/]+)/manual(?:/|$)")


def load_tables(extra: pathlib.Path | None) -> dict[str, dict]:
    """Les tables livrées, plus celles d'un fichier JSON `{langue: table}`."""
    tables = {language: dict(table) for language, table in TABLES.items()}
    if extra is not None:
        for language, table in json.loads(extra.read_text(encoding="utf-8")).items():
            missing = [key for key in KEYS if key not in table]
            if missing:
                raise SystemExit(f"{extra} : la table « {language} » n'a pas {', '.join(missing)}")
            tables[language] = table
    return tables


def language_of(page: pathlib.Path, text: str) -> str:
    """La langue d'une page : sa déclaration, sinon son arbre, sinon la langue source."""
    for line in text.splitlines()[:5]:
        if match := DECLARATION.search(line):
            return match.group(1)
    if match := TREE.search(page.resolve().as_posix()):
        return match.group(1)
    return SOURCE_LANGUAGE


def missing_table(tables: dict[str, dict], pages_by_language: dict[str, list[str]]) -> list[str]:
    """Un écart par langue de page qui n'a pas de table, avec les pages qui la parlent."""
    return [
        f"LANGUE SANS VOCABULAIRE : les pages en « {language} » ({', '.join(sorted(pages))}) "
        f"n'ont pas de table de vocabulaire (doc_vocabulary.py, ou --vocabulary)"
        for language, pages in sorted(pages_by_language.items())
        if language not in tables
    ]
