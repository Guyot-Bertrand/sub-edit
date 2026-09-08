#!/usr/bin/env python3
"""Ce qu'une conversion perd, mesuré plutôt que cru.

L'[ADR 0009](../../docs/adr/0009-texte-en-chaine-brute.md) annonce depuis la
phase 1 qu'un modèle structuré sert de pivot « uniquement lors d'une conversion
entre formats, où la perte est inévitable et **assumée** ». *Assumée* suppose
qu'on la connaisse, et rien ne la disait.

    --binary CHEMIN   le subedit-cli à mesurer (défaut : build/dev/bin/subedit-cli)
    --journal         confronte la mesure au relevé de docs/mesures/conversion.md
    --record          réécrit ce relevé depuis la mesure ; implique --journal
    --diff            montre, pour chaque fichier altéré, ce qui ne revient pas

## Ce qui est mesuré, et pourquoi c'est un aller-retour

Une conversion ne se regarde pas de l'extérieur : le fichier d'arrivée est dans
un autre format, et il n'y a rien à quoi le comparer. **Le format de départ est
donc la toise.** Un fichier part en `A`, passe par `B`, revient en `A`, et ce
qui ne revient pas est ce que `B` n'a pas su porter.

C'est exact, et ça ne l'est que grâce à une propriété que le harnais de #338
tient par ailleurs : **un fichier du corpus est déjà ce que notre écriture
produit**, octet pour octet. Sans elle, un écart entre le départ et l'arrivée
pourrait venir de la mise en forme plutôt que du passage, et la mesure ne dirait
plus rien.

## Le nombre, et ce qu'il vaut

    aller-retour intacts : N/M

**Il mélange sciemment deux choses** — ce qu'un format ne *peut* pas porter, et
ce que notre conversion perd sans y être obligée. Les séparer demanderait de
déclarer les soixante-douze cases à la main, et une déclaration écrite à la main
se périme en silence : c'est le défaut que `check-installation.sh` nomme pour le
manuel, et que #289 a corrigé pour le corpus.

Ce que le nombre garantit est plus étroit et se tient tout seul : **une perte ne
s'aggrave pas sans qu'on le voie.** Ce qu'elle vaut en droit se lit à côté, en
prose, dans le relevé — et c'est là que la politique de dégradation se décide.

## Ce qui n'est pas lu

**Le corpus privé.** Il est absent de toute machine qui ne l'a pas, donc une
porte qui le lirait dirait deux choses selon le poste. Il porte pourtant deux
formats de la phase ; ce qu'on en fera est l'affaire de #340.
"""

from __future__ import annotations

import argparse
import datetime
import difflib
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
JOURNAL = REPO_ROOT / "docs" / "mesures" / "conversion.md"
DEFAULT_BINARY = REPO_ROOT / "build" / "dev" / "bin" / "subedit-cli"

# Les répertoires parcourus, dans l'ordre où le relevé les nomme. `formats/`
# porte la même scène dans les neuf formats — c'est lui qui rendra la matrice
# comparable d'un format à l'autre ; `valides/` porte des fichiers qui ne se
# ressemblent pas, et c'est lui qui trouve aujourd'hui ce que la scène ne montre
# pas : des coordonnées SubRip, un en-tête WebVTT, des réglages de cellule.
CORPUS = ("valides", "formats")

BLOCK_OPEN = "<!-- relevé engendré : ne pas modifier à la main -->"
BLOCK_CLOSE = "<!-- fin du relevé -->"
RECORDED = re.compile(r"^\s*aller-retour intacts\s*:\s*(\d+)\s*/\s*(\d+)\s*$", re.MULTILINE)

# Le nom qu'`inspect` imprime, et la valeur que `--to` attend. Les deux viennent
# du binaire ; ce qui n'en vient pas est leur correspondance, et c'est pourquoi
# un nom inconnu des deux côtés **arrête** au lieu d'être ignoré. Un format
# livré sans sa ligne ici serait mesuré à moitié, en silence.
TARGET_OF = {
    "SubRip": "srt",
    "WebVTT": "vtt",
    "SubViewer 2": "subviewer2",
    "Sub Station Alpha": "ssa",
    "Advanced SSA": "ass",
    "MPL2": "mpl2",
}

GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def fail(message):
    print(f"{RED}✗{RESET} {message}", file=sys.stderr)
    sys.exit(1)


def run(binary, arguments):
    """Lance le binaire sans passer par un shell.

    **Jamais de shell**, et #290 dit pourquoi : les chemins peuvent porter des
    espaces et des crochets, et une ligne de commande recomposée les éclate.
    """
    return subprocess.run([str(binary), *arguments], capture_output=True, text=True, check=False)


def targets(binary):
    """Les formats que `--to` accepte, lus dans l'aide du binaire.

    **Dérivés plutôt qu'écrits.** Une liste de formats recopiée ici resterait à
    deux le jour où le binaire en accepterait neuf, et la mesure serait
    silencieusement partielle.
    """
    help_text = run(binary, ["convert", "--help"]).stdout
    found = re.search(r"--to TEXT:\{([^}]*)\}", help_text)
    if found is None:
        fail(f"« --to » introuvable dans l aide de {binary}")
    return [name.strip() for name in found.group(1).split(",") if name.strip()]


def format_of(binary, path):
    """Le format qu'`inspect` reconnaît, ou rien s'il n'ouvre pas."""
    done = run(binary, ["--quiet", "inspect", str(path)])
    if done.returncode != 0:
        return None
    # **`(.+?)` et non `(\S+)`** : « SubViewer 2 » porte une espace, et le motif
    # d'origine ne pouvait pas le lire. Il rendait alors « pas de format », donc
    # un fichier compté parmi ceux qui ne s'ouvrent pas — un faux négatif
    # silencieux, trouvé le jour où le premier format à nom composé est arrivé.
    found = re.search(r"^\s*format:\s*(.+?)\s*$", done.stdout, re.MULTILINE)
    return found.group(1) if found else None


def corpus_files():
    found = []
    for directory in CORPUS:
        base = REPO_ROOT / "src" / "test" / "data" / directory
        for path in sorted(base.iterdir()):
            if path.is_file() and path.suffix != ".md":
                found.append(path)
    return found


def round_trip(binary, path, home, via, work):
    """Envoie le fichier en `via`, le ramène en `home`, et rend ce qui revient.

    Rend `None` quand l'une des deux conversions refuse — un refus n'est pas
    une perte, et le confondre avec une perte ferait passer une panne pour une
    mesure.
    """
    outward = work / f"aller.{via}"
    back = work / f"retour.{home}"
    for target, source, output in ((via, path, outward), (home, outward, back)):
        done = run(binary, ["--quiet", "convert", "--to", target,
                            "--output", str(output), str(source)])
        if done.returncode != 0:
            return None
    return back.read_bytes()


def measure(binary, known):
    """Chaque fichier lisible, envoyé dans chacun des autres formats."""
    trips = []
    unreadable = 0

    with tempfile.TemporaryDirectory() as scratch:
        work = Path(scratch)
        for path in corpus_files():
            name = format_of(binary, path)
            if name is None:
                unreadable += 1
                continue
            if name not in TARGET_OF:
                fail(f"« {name} » est lu par le binaire et absent de TARGET_OF :"
                     " compléter la table avant de mesurer")
            home = TARGET_OF[name]
            if home not in known:
                fail(f"« {home} » n est pas une valeur de --to : la table et le binaire"
                     " ne disent plus la même chose")

            original = path.read_bytes()
            for via in known:
                if via == home:
                    continue
                came_back = round_trip(binary, path, home, via, work)
                if came_back is None:
                    fail(f"la conversion a échoué sur {path.name}, en passant par {via}")
                trips.append((path, name, via, came_back == original,
                              None if came_back == original else came_back))

    return trips, unreadable


def relative(path):
    return path.relative_to(REPO_ROOT / "src" / "test" / "data").as_posix()


def first_difference(path, came_back):
    """La première ligne du départ qui ne revient pas, telle qu'elle est écrite.

    **Une ligne, et non un compte.** Un nombre de lignes qui bougent ne se lit
    pas ; la ligne elle-même dit ce qui a été perdu, dans le vocabulaire du
    format de départ, à qui la regarde.
    """
    before = path.read_bytes().decode("utf-8", "replace").splitlines()
    after = came_back.decode("utf-8", "replace").splitlines()
    for line in difflib.unified_diff(before, after, n=0, lineterm=""):
        if line.startswith("-") and not line.startswith("---"):
            return line[1:]
    # Le départ ne perd rien : ce qui a bougé est un ajout, une fin inventée par
    # exemple. On rend alors la première ligne ajoutée, pour ne pas rester muet.
    for line in difflib.unified_diff(before, after, n=0, lineterm=""):
        if line.startswith("+") and not line.startswith("+++"):
            return line[1:]
    return ""


def show_diff(path, came_back):
    before = path.read_bytes().decode("utf-8", "replace").splitlines()
    after = came_back.decode("utf-8", "replace").splitlines()
    for line in difflib.unified_diff(before, after, fromfile=relative(path),
                                     tofile=f"{relative(path)} (revenu)", lineterm=""):
        print(f"    {line}")


def matrix(trips):
    """Les paires ordonnées, comptées : c'est la matrice que la phase remplira."""
    cells = {}
    for _, name, via, intact, _ in trips:
        home = TARGET_OF[name]
        kept, total = cells.get((home, via), (0, 0))
        cells[(home, via)] = (kept + int(intact), total + 1)
    return cells


def report(trips, unreadable):
    kept = sum(1 for *_, intact, _ in trips if intact)
    total = len(trips)
    print(f"    aller-retour intacts : {kept}/{total}")
    if unreadable:
        print(f"    {unreadable} fichier(s) du corpus ne s ouvrent pas encore")
    return kept, total


def journal_text():
    if not JOURNAL.is_file():
        fail(f"journal introuvable : {JOURNAL}")
    return JOURNAL.read_text(encoding="utf-8")


def recorded_count(text):
    """Le compte qu'affiche le journal, ou l'arrêt du script.

    **Une ancre absente ou dupliquée arrête**, comme pour le score de détection :
    un journal réécrit à la main sur lequel plus rien ne se lit rendrait la
    comparaison muette, donc verte.
    """
    found = RECORDED.findall(text)
    if len(found) != 1:
        fail(f"« aller-retour intacts : N/M » apparaît {len(found)} fois dans"
             f" {JOURNAL.relative_to(REPO_ROOT)}, une seule attendue")
    return int(found[0][0]), int(found[0][1])


def version_of():
    for line in (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "VERSION":
            return parts[1]
    fail("version illisible dans CMakeLists.txt")
    return ""


def rewrite_journal(trips, unreadable, known, kept, total):
    """Réécrit le seul bloc engendré du journal, et laisse la prose intacte."""
    text = journal_text()
    opened = text.find(BLOCK_OPEN)
    closed = text.find(BLOCK_CLOSE)
    if opened < 0 or closed < opened:
        fail(f"bornes du relevé introuvables dans {JOURNAL.relative_to(REPO_ROOT)}")

    body = [f"{BLOCK_OPEN}\n\n", f"    aller-retour intacts : {kept}/{total}\n\n",
            f"Relevé sur la version {version_of()}, le"
            f" {datetime.date.today().isoformat()}.\n\n"]

    cells = matrix(trips)
    names = sorted({TARGET_OF[name] for _, name, *_ in trips} | set(known))
    body.append("| Départ \\ Arrivée | " + " | ".join(f"`{name}`" for name in names) + " |\n")
    body.append("| :--- | " + " | ".join("---:" for _ in names) + " |\n")
    for home in names:
        row = [f"| `{home}` "]
        for via in names:
            if home == via:
                row.append("| — ")
            elif (home, via) in cells:
                right, over = cells[(home, via)]
                row.append(f"| {right}/{over} ")
            else:
                row.append("| · ")
        body.append("".join(row) + "|\n")
    body.append("\n")

    lost = [(path, name, via, came_back)
            for path, name, via, intact, came_back in trips if not intact]
    if lost:
        body.append("| Fichier | Passage par | Première ligne qui ne revient pas |\n")
        body.append("| :------ | :---------- | :-------------------------------- |\n")
        for path, _, via, came_back in lost:
            line = first_difference(path, came_back).replace("|", "\\|")
            body.append(f"| `{relative(path)}` | `{via}` | `{line}` |\n")
    else:
        body.append("Aucun aller-retour altéré.\n")
    body.append("\n")

    if unreadable:
        body.append(f"{unreadable} fichier(s) du corpus ne s'ouvrent pas encore et"
                    " n'entrent dans aucune mesure.\n\n")

    written = text[:opened] + "".join(body) + text[closed:]
    if written == text:
        print(f"{GREEN}✓{RESET} relevé déjà à jour : rien à réenregistrer")
        return 0
    JOURNAL.write_text(written, encoding="utf-8")
    print(f"{GREEN}✓{RESET} relevé enregistré : {kept}/{total}")
    return 0


def compare_to_journal(trips, unreadable, known, kept, total, record):
    """Les trois situations, séparément — la mécanique de `make score`."""
    if record:
        return rewrite_journal(trips, unreadable, known, kept, total)

    was, over = recorded_count(journal_text())
    command = "make conversion-record"

    # Les produits en croix plutôt que les comptes bruts : un fichier ajouté au
    # corpus change le dénominateur, et deux comptes ne se comparent plus.
    if kept * over < was * total:
        print(f"{RED}✗{RESET} la conversion perd davantage : {kept}/{total} intacts,"
              f" contre {was}/{over} au relevé", file=sys.stderr)
        for path, _, via, intact, came_back in trips:
            if not intact:
                print(f"    {relative(path)} en passant par {via} :"
                      f" {first_difference(path, came_back)}", file=sys.stderr)
        print(f"\n  comparer à {JOURNAL.relative_to(REPO_ROOT)} pour voir ce qui a bougé.",
              file=sys.stderr)
        print(f"  corriger la conversion, ou réenregistrer sciemment :\n    {command}",
              file=sys.stderr)
        return 1

    if (kept, total) != (was, over):
        print(f"{GREEN}✓{RESET} la conversion perd moins : {kept}/{total} intacts,"
              f" contre {was}/{over} au relevé")
        print(f"  l enregistrer :\n    {command}")
        return 0

    print(f"{GREEN}✓{RESET} {kept}/{total} allers-retours intacts, comme au relevé")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--binary", default=str(DEFAULT_BINARY),
                        help="le subedit-cli à mesurer")
    parser.add_argument("--journal", action="store_true",
                        help="confronte la mesure au relevé")
    parser.add_argument("--record", action="store_true",
                        help="réécrit le relevé depuis la mesure ; implique --journal")
    parser.add_argument("--diff", action="store_true",
                        help="montre ce qui ne revient pas, fichier par fichier")
    options = parser.parse_args()

    binary = Path(options.binary)
    if not binary.is_file() or not os.access(binary, os.X_OK):
        fail(f"binaire introuvable ou non exécutable : {binary}\n"
             "  le construire : cmake --build --preset dev --target subedit-cli")

    known = targets(binary)
    trips, unreadable = measure(binary, known)
    if not trips:
        fail("aucun aller-retour à mesurer : le corpus ou le binaire ne rendent rien")

    kept, total = report(trips, unreadable)

    if options.diff:
        for path, _, via, intact, came_back in trips:
            if not intact:
                print(f"\n  {relative(path)}, en passant par {via} :")
                show_diff(path, came_back)

    if options.journal or options.record:
        return compare_to_journal(trips, unreadable, known, kept, total, options.record)
    return 0


if __name__ == "__main__":
    sys.exit(main())
