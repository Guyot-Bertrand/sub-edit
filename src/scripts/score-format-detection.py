#!/usr/bin/env python3
"""Ce qu'une détection de format répond, et **où elle cesse de savoir**.

    --detector 'CMD {}'   la détection à mesurer, répétable ; {} reçoit le chemin
    --prive               ajoute le corpus privé, dont on ne tire aucun taux
    --journal             confronte la mesure au relevé de docs/mesures/
    --record              réécrit ce relevé depuis la mesure ; implique --journal

## Pourquoi la question se pose maintenant

**Avec deux formats, elle était fermée d'avance** : un fichier porte la
signature `WEBVTT` ou une flèche à virgule, et rien d'autre ne s'en approche.
Avec neuf, c'est une classification, et les collisions sont réelles :

  .sub          désigne MicroDVD **et** SubViewer 2
  .txt          désigne TMPlayer, MPL2, et tout le reste du monde
  {0}{25}       ne diffère de [0][25] que par la forme des crochets
  SSA et ASS    partagent tout leur en-tête sauf une version et un nom de section

C'est le raisonnement de #290, transposé : **une détection est un classifieur,
pas une fonction.** La règle du projet — rien plutôt qu'une supposition —
demande à être vérifiée sur neuf candidates, pas crue.

## Trois issues, et deux d'entre elles ne se valent pas

Une réponse **juste**. Une réponse **fausse**, qui fait ouvrir un fichier comme
ce qu'il n'est pas. Un **refus**, qui dit ne pas savoir. Le refus est la règle du
projet appliquée ; la confusion est ce qu'on ne veut à aucun prix. Le relevé les
compte donc à part, et c'est ce qui fait qu'un taux dit ici quelque chose.

## Le nom du fichier ne fait pas partie de la question

Les fixtures s'appellent `scene.microdvd.sub` : leur nom **porte la réponse**,
puisqu'il faut bien étiqueter le corpus. Il est retiré avant que le détecteur
les voie — chaque fichier lui est passé sous un nom neutre, **en gardant son
extension réelle**, si bien que `.sub` reste aussi ambigu qu'il l'est dans la
nature.

Si le cadrage décide un jour que l'extension est une preuve recevable, c'est
ici qu'il faudra le dire, et délibérément.
"""

from __future__ import annotations

import argparse
import collections
import datetime
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
CORPUS = REPO_ROOT / "src" / "test" / "data"
PRIVATE = REPO_ROOT / "src" / "data"
JOURNAL = REPO_ROOT / "docs" / "mesures" / "detection-de-format.md"

BLOCK_OPEN = "<!-- relevé engendré : ne pas modifier à la main -->"
BLOCK_CLOSE = "<!-- fin du relevé -->"
RECORDED = re.compile(r"^\s*formats reconnus\s*:\s*(\d+)\s*/\s*(\d+)\s*$", re.MULTILINE)

# Le nom que porte une fixture, et celui que le produit imprime. Les deux tiennent
# ici parce que rien ne les rapproche ailleurs : le premier est un morceau de nom
# de fichier, le second sort de `nameOf(SubtitleFormat)`.
#
# **Un slug inconnu arrête le script.** Un rendu déposé dans `formats/` sans sa
# ligne ici serait mesuré contre rien, et le taux resterait beau.
NAMED = {
    "srt": "SubRip",
    "vtt": "WebVTT",
    "subviewer2": "SubViewer 2",
    "ssa": "Sub Station Alpha",
    "ass": "Advanced SSA",
    "microdvd": "MicroDVD",
    "mpl2": "MPL2",
    "tmplayer": "TMPlayer",
    "lrc": "LRC",
}

# Les extensions sous lesquelles un fichier de sous-titres se présente. `.txt`
# n'y est pas, et son absence est un choix : il nomme TMPlayer et MPL2, et tout
# le reste. Les seuls `.txt` du corpus privé sont des notes et des extraits de
# manuel — les compter reviendrait à mesurer des refus mérités.
SUBTITLE_SUFFIXES = (".srt", ".vtt", ".sub", ".ass", ".ssa", ".lrc")

# La forme d'un nom de format, et rien d'autre ne sera lu comme une réponse.
# C'est la garde de #290, reprise telle quelle : un détecteur qui se plaint sur
# sa sortie standard ne doit pas pouvoir faire imprimer un chemin.
FORMAT_NAME = re.compile(r"^[A-Za-z][A-Za-z0-9 +]{0,30}$")

BOLD = "\033[1m"
GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"


def fail(message):
    print(f"{RED}✗{RESET} {message}", file=sys.stderr)
    sys.exit(1)


def label_of(path):
    """Le format qu'un fichier du corpus annonce, par son nom.

    `scene.microdvd.sub` porte son slug entre le nom et l'extension ; les autres
    n'ont que leur extension, qui suffit quand elle ne désigne qu'un format.
    """
    parts = path.name.split(".")
    slug = parts[-2] if len(parts) > 2 else parts[-1]
    if slug not in NAMED:
        fail(f"« {slug} » n est pas un format connu de la table, et {path.name}"
             " en porte le nom : compléter NAMED avant de mesurer")
    return NAMED[slug]


def labelled_files():
    """Le corpus étiqueté : la scène en neuf formats, et les fichiers valides."""
    found = []
    for directory in ("formats", "valides"):
        for path in sorted((CORPUS / directory).iterdir()):
            if path.is_file() and path.suffix != ".md":
                found.append((path, label_of(path)))
    return found


# Les deux réponses qui ne sont pas un nom de format. **« Indécodable » n'est
# pas un refus de nommer** : le fichier n'a pas été lu du tout, et le compter
# comme une détection manquée ferait porter à la détection de format ce qui
# regarde celle de l'encodage.
REFUSED = "refusé"
UNDECODABLE = "indécodable"


def detect(command, path, work):
    """Ce que le détecteur dit de ce fichier, ou pourquoi il ne dit rien.

    **Le nom est retiré, l'extension est gardée.** Le fichier est recopié sous
    `sujet.<ext>` : le détecteur n'a donc plus l'étiquette que le corpus porte,
    et garde ce qu'un fichier du monde réel lui offrirait.

    **Sans shell, et la réponse est filtrée** — les deux gardes de #290, et
    elles ont été payées : un chemin passé au shell y était éclaté, et les
    plaintes du détecteur, qui citaient les noms, se lisaient comme des réponses.
    """
    subject = work / f"sujet{path.suffix}"
    shutil.copyfile(path, subject)

    try:
        parts = shlex.split(command)
    except ValueError:
        return REFUSED

    argv = [str(subject) if part == "{}" else part for part in parts]

    try:
        done = subprocess.run(argv, capture_output=True, text=True, timeout=30, check=False)
    except (OSError, subprocess.SubprocessError):
        return REFUSED

    if done.returncode == 3:
        return UNDECODABLE
    if done.returncode != 0:
        return REFUSED

    lines = done.stdout.strip().splitlines()
    said = lines[0].strip() if lines else ""
    return said if FORMAT_NAME.match(said) and said in NAMED.values() else REFUSED


def score(command):
    """Le taux, et les deux façons d'échouer, comptées séparément."""
    right = []
    refused = []
    confused = []

    with tempfile.TemporaryDirectory() as scratch:
        work = Path(scratch)
        for path, wanted in labelled_files():
            said = detect(command, path, work)
            if said == wanted:
                right.append((path, wanted))
                print(f"  {GREEN}✓{RESET} {path.name:<24} {said}")
            elif said in (REFUSED, UNDECODABLE):
                refused.append((path, wanted))
                print(f"  {YELLOW}·{RESET} {path.name:<24} {said}, attendu {wanted}")
            else:
                confused.append((path, wanted, said))
                print(f"  {RED}✗{RESET} {path.name:<24} {said}  au lieu de {wanted}")

    return right, refused, confused


def report(right, refused, confused):
    total = len(right) + len(refused) + len(confused)
    print(f"\n    formats reconnus : {len(right)}/{total}")
    print(f"    refusés : {len(refused)}   —   confondus : {len(confused)}")
    if confused:
        print(f"\n  {RED}la détection s est trompée, ce qui est autre chose qu un refus :{RESET}")
        for path, wanted, said in confused:
            print(f"    {path.name} : {said} au lieu de {wanted}")
    return len(right), total


def report_private(commands):
    """Ce que le corpus privé peut dire, et il ne peut pas dire un taux.

    **Aucun fichier n'y est nommé**, ni son titre, ni son nom de fichier : c'est
    la règle du dépôt, et #290 l'a apprise à ses dépens.
    """
    files = sorted(p for p in PRIVATE.rglob("*")
                   if p.is_file() and p.suffix.lower() in SUBTITLE_SUFFIXES)
    if not files:
        print(f"\n{BOLD}corpus privé — absent de cette machine{RESET}")
        return

    print(f"\n{BOLD}corpus privé — {len(files)} fichiers, sans étiquettes{RESET}")
    print("  aucun taux : il n'y a pas de vérité à confronter, seulement la")
    print("  distribution des réponses.")

    with tempfile.TemporaryDirectory() as scratch:
        work = Path(scratch)
        for command in commands:
            answers = collections.Counter()
            for path in files:
                answers[detect(command, path, work)] += 1
            print(f"\n  {BOLD}{command}{RESET}")
            for name, count in answers.most_common():
                print(f"    {count:3d}  {name}")


def journal_text():
    if not JOURNAL.is_file():
        fail(f"journal introuvable : {JOURNAL}")
    return JOURNAL.read_text(encoding="utf-8")


def recorded_score(text):
    """Le taux qu'affiche le journal, ou l'arrêt du script.

    **Une ancre absente ou dupliquée arrête** plutôt que de laisser passer : un
    journal réécrit à la main sur lequel plus rien ne se lit rendrait la
    comparaison muette, donc verte.
    """
    found = RECORDED.findall(text)
    if len(found) != 1:
        fail(f"« formats reconnus : N/M » apparaît {len(found)} fois dans"
             f" {JOURNAL.relative_to(REPO_ROOT)}, une seule attendue")
    return int(found[0][0]), int(found[0][1])


def version_of():
    for line in (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "VERSION":
            return parts[1]
    fail("version illisible dans CMakeLists.txt")
    return ""


def relative(path):
    return path.relative_to(CORPUS).as_posix()


def rewrite_journal(right, refused, confused):
    """Réécrit le seul bloc engendré du journal, et laisse la prose intacte."""
    text = journal_text()
    opened = text.find(BLOCK_OPEN)
    closed = text.find(BLOCK_CLOSE)
    if opened < 0 or closed < opened:
        fail(f"bornes du relevé introuvables dans {JOURNAL.relative_to(REPO_ROOT)}")

    total = len(right) + len(refused) + len(confused)
    body = [f"{BLOCK_OPEN}\n\n", f"    formats reconnus : {len(right)}/{total}\n\n",
            f"Relevé sur la version {version_of()}, le"
            f" {datetime.date.today().isoformat()}.\n\n"]

    body.append("| | Fichiers |\n| :--- | -------: |\n")
    body.append(f"| reconnus | {len(right)} |\n")
    body.append(f"| refusés, faute d un lecteur | {len(refused)} |\n")
    body.append(f"| **confondus** | **{len(confused)}** |\n\n")

    if refused:
        body.append("Refusés :\n\n")
        for path, wanted in refused:
            body.append(f"- `{relative(path)}` — {wanted}\n")
        body.append("\n")

    if confused:
        body.append("| Fichier | Est | Lu comme |\n| :------ | :-- | :------- |\n")
        for path, wanted, said in confused:
            body.append(f"| `{relative(path)}` | {wanted} | **{said}** |\n")
        body.append("\n")
    else:
        body.append("**Aucune confusion** : la détection refuse, elle ne se trompe pas.\n\n")

    written = text[:opened] + "".join(body) + text[closed:]
    if written == text:
        print(f"{GREEN}✓{RESET} relevé déjà à jour : rien à réenregistrer")
        return 0
    JOURNAL.write_text(written, encoding="utf-8")
    print(f"{GREEN}✓{RESET} relevé enregistré : {len(right)}/{total}")
    return 0


def compare_to_journal(right, refused, confused, record):
    """Les trois situations, séparément — la mécanique de `make score`."""
    if record:
        return rewrite_journal(right, refused, confused)

    kept = len(right)
    total = kept + len(refused) + len(confused)
    was, over = recorded_score(journal_text())
    command = "make score-format-record"

    # **Une confusion échoue à elle seule**, quel que soit le taux. Un format
    # ouvert comme ce qu'il n'est pas est ce que la règle du projet interdit ;
    # un taux qui monte pendant qu'une confusion apparaît ne doit pas la
    # racheter.
    if confused:
        print(f"{RED}✗{RESET} la détection confond {len(confused)} fichier(s) :"
              " rien plutôt qu une supposition", file=sys.stderr)
        for path, wanted, said in confused:
            print(f"    {relative(path)} : {said} au lieu de {wanted}", file=sys.stderr)
        return 1

    # Les produits en croix plutôt que les comptes bruts : un rendu ajouté au
    # corpus change le dénominateur, et deux comptes ne se comparent plus.
    if kept * over < was * total:
        print(f"{RED}✗{RESET} la détection a reculé : {kept}/{total} reconnus,"
              f" contre {was}/{over} au relevé", file=sys.stderr)
        print(f"\n  comparer à {JOURNAL.relative_to(REPO_ROOT)} pour voir ce qui a bougé.",
              file=sys.stderr)
        print(f"  corriger la détection, ou réenregistrer sciemment :\n    {command}",
              file=sys.stderr)
        return 1

    if (kept, total) != (was, over):
        print(f"{GREEN}✓{RESET} la détection progresse : {kept}/{total} reconnus,"
              f" contre {was}/{over} au relevé")
        print(f"  l enregistrer :\n    {command}")
        return 0

    print(f"{GREEN}✓{RESET} {kept}/{total} formats reconnus, comme au relevé")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--detector", action="append", required=True,
                        help="commande de détection ; {} reçoit le chemin")
    parser.add_argument("--prive", action="store_true",
                        help="ajoute le corpus privé, sans en tirer de taux")
    parser.add_argument("--journal", action="store_true",
                        help="confronte la mesure au relevé")
    parser.add_argument("--record", action="store_true",
                        help="réécrit le relevé depuis la mesure ; implique --journal")
    options = parser.parse_args()

    # **Un seul détecteur quand le journal est en jeu.** Le relevé porte le score
    # de la détection du projet, et non celui d'un témoin qu'on lui compare.
    if (options.journal or options.record) and len(options.detector) != 1:
        parser.error("--journal et --record demandent un seul --detector")

    right = refused = confused = None
    for command in options.detector:
        print(f"{BOLD}{command}{RESET}")
        right, refused, confused = score(command)
        report(right, refused, confused)

    if options.prive:
        report_private(options.detector)

    if options.journal or options.record:
        return compare_to_journal(right, refused, confused, options.record)
    return 0


if __name__ == "__main__":
    sys.exit(main())
