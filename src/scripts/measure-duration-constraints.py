#!/usr/bin/env python3
"""Recense, sur un corpus de sous-titres, ce que les quatre contraintes de
durée de Gaupol violent et ce qu'elles se contredisent — issue #371.

`adjust_durations` en porte quatre : durée minimale, durée maximale, écart
minimal entre sous-titres, vitesse de lecture en caractères par seconde. La
feuille de route les dit « potentiellement contradictoires » ; *potentiellement*
est le mot d'avant la mesure, et c'est celui-ci.

Deux tables, et la seconde est celle qui n'existait nulle part.

**Ce qui est violé tel quel** — un compte par contrainte, sur les positions du
fichier telles qu'elles sont écrites.

**Ce qui ne peut pas être satisfait ensemble** — les contradictions, qui ne
dépendent d'aucun algorithme parce qu'elles portent sur la place disponible :
seule la fin bouge chez Gaupol, donc la place d'un sous-titre est ce qui sépare
son début de celui du suivant, moins l'écart exigé.

    place = suivant.début − début − écart

    minimum contre écart      place < minimum
    vitesse contre écart      place < longueur / vitesse
    vitesse contre maximum    longueur / vitesse > maximum

Aucune des trois ne se règle par un ordre de résolution : ce sont les
sous-titres pour lesquels **toute réponse en viole une**.

    ./src/scripts/measure-duration-constraints.py [répertoire] [options]

**Il n'entre dans aucune porte, et rien ne dépend de lui** : c'est un
instrument, pas une vérification. Le corpus qui a du sens ici est celui de
`src/data/`, privé à chaque machine et ignoré par git — d'où l'absence de
relevé dans `docs/mesures/` : un chiffre qu'on ne peut pas rejouer est un
témoignage, pas une mesure. Sur une machine sans corpus, il le dit et s'arrête.

**Ce que vaut son analyse, et comment le revérifier.** Les positions sont lues
par un parcours écrit ici plutôt que par le lecteur du projet — comme le fait
`measure-mentions.py`, et pour la même raison : un instrument qu'on lance à la
main ne doit pas exiger un arbre construit. Le prix est qu'il peut se tromper,
et une première version de ce parcours s'est trompée — elle lisait « ,050 »
comme cinq cents millisecondes. `--against` confronte donc le parcours au
lecteur du projet, fichier par fichier :

    ./src/scripts/measure-duration-constraints.py --against ./build/dev/bin/subedit-cli

**Ce qu'il ne fait pas**, et c'est délibéré : appliquer un ajustement puis
compter ce qui reste insatisfait. Cela demande l'ajustement, que la phase n'a
pas encore écrit, et l'ordre de résolution, que le cadrage n'a pas encore
choisi. Simuler celui de Gaupol ici reviendrait à le préjuger.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_CORPUS = REPO_ROOT / "src" / "data"

# Les deux formats que ce parcours lit. Les sept autres demandent le lecteur du
# projet — et pour MicroDVD, une fréquence d'images : un fichier compté en
# images n'a pas de durée tant que personne n'a dit à quelle cadence. Les
# fichiers laissés de côté sont comptés et dits.
EXTENSIONS = {".srt", ".vtt"}

# Un horodatage, avec ou sans ses heures : WebVTT les omet sous l'heure.
STAMP = r"(?:(\d{1,3}):)?(\d{1,2}):(\d{2})[,.](\d{1,3})"
CUE = re.compile(STAMP + r"\s*-->\s*" + STAMP)

# Ce qui n'est pas du texte visible. Les deux formes, parce qu'un corpus réel
# mêle les vocabulaires et qu'une accolade d'Advanced SSA se retrouve recopiée
# dans un SubRip plus souvent qu'on ne le croit.
TAG = re.compile(r"</?[a-zA-Z][^>]*>|\{[^}]*\}")

# Les valeurs par défaut sont celles de `gaupol/config.py`. Le maximum y est
# **inactif** par défaut, et il est posé ici quand même : une colonne éteinte
# ne dit rien, et savoir combien de sous-titres passeraient six secondes est
# une information même quand personne ne s'en sert.
DEFAULT_MINIMUM = 1.5
DEFAULT_MAXIMUM = 6.0
DEFAULT_GAP = 0.0
DEFAULT_SPEED = 15.0

BOLD = "\033[1m"
RESET = "\033[0m"


def decoded(raw: bytes) -> str:
    """Rend le texte du fichier, quel que soit son encodage — un corpus réel en
    mêle plusieurs, et refuser de lire les uns fausserait les proportions."""
    for encoding in ("utf-8-sig", "cp1252"):
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError:
            continue
    return raw.decode("latin-1", "replace")


def milliseconds(hours: str, minutes: str, seconds: str, fraction: str) -> int:
    """Un horodatage en millisecondes.

    La fraction est écrite sur un à trois chiffres : « .5 » vaut cinq cents
    millisecondes et non cinq. La compléter à droite est la seule lecture juste."""
    thousandths = int(fraction.ljust(3, "0"))
    return ((int(hours or 0) * 60 + int(minutes)) * 60 + int(seconds)) * 1000 + thousandths


def cues_of(path: Path) -> list[tuple[int, int, int]]:
    """Les répliques du fichier : début, fin, et longueur du texte visible.

    Analyse volontairement grossière — une ligne d'horodatage, et tout ce qui
    suit jusqu'à la ligne vide est du texte. Elle vaut pour SubRip comme pour
    WebVTT, et il s'agit de compter des durées, pas de relire un lecteur qui
    existe. C'est le choix qu'a fait `measure-mentions.py` avant celui-ci.

    **La longueur compte le saut de ligne**, parce que `get_text_length` le
    compte : Gaupol retire les balises et prend `len` de ce qui reste."""
    content = decoded(path.read_bytes()).replace("\r\n", "\n").replace("\r", "\n")
    lines = content.split("\n")

    cues: list[tuple[int, int, int]] = []
    index = 0
    while index < len(lines):
        found = CUE.search(lines[index])
        if found is None:
            index += 1
            continue

        fields = found.groups()
        start = milliseconds(*fields[0:4])
        end = milliseconds(*fields[4:8])

        index += 1
        body: list[str] = []
        while index < len(lines) and lines[index].strip() and CUE.search(lines[index]) is None:
            body.append(lines[index])
            index += 1

        cues.append((start, end, len(TAG.sub("", "\n".join(body)))))
    return cues


class Tally:
    """Ce qu'un parcours compte, et qui s'additionne d'un fichier à l'autre."""

    FIELDS = (
        "subtitles",
        "too_short",
        "too_long",
        "gap_too_small",
        "too_fast",
        "minimum_against_gap",
        "speed_against_gap",
        "speed_against_maximum",
        "contradicted",
        "not_ordered",
    )

    def __init__(self) -> None:
        for field in self.FIELDS:
            setattr(self, field, 0)

    def add(self, other: "Tally") -> None:
        for field in self.FIELDS:
            setattr(self, field, getattr(self, field) + getattr(other, field))


def census(cues: list[tuple[int, int, int]], limits: argparse.Namespace) -> Tally:
    """Le recensement d'un fichier.

    Une contrainte posée à zéro ne s'applique pas — c'est la convention de ce
    script, et c'est aussi ce que Gaupol fait par accident pour les deux
    durées : son `minimum and …` est faux quand le minimum vaut zéro."""
    tally = Tally()

    for position, (start, end, length) in enumerate(cues):
        tally.subtitles += 1

        duration = (end - start) / 1000.0
        if duration <= 0:
            # Une fin avant son début n'a pas de durée à comparer, et compter
            # une durée négative comme « trop courte » noierait le vrai chiffre.
            tally.not_ordered += 1
            continue

        wanted = length / limits.speed if limits.speed else 0.0

        if limits.minimum and duration < limits.minimum:
            tally.too_short += 1
        if limits.maximum and duration > limits.maximum:
            tally.too_long += 1
        if wanted and duration < wanted:
            tally.too_fast += 1

        contradicted = False
        if limits.maximum and wanted > limits.maximum:
            tally.speed_against_maximum += 1
            contradicted = True

        if position + 1 < len(cues):
            room = (cues[position + 1][0] - start) / 1000.0 - limits.gap
            if (cues[position + 1][0] - end) / 1000.0 < limits.gap:
                tally.gap_too_small += 1
            if limits.minimum and room < limits.minimum:
                tally.minimum_against_gap += 1
                contradicted = True
            if wanted and room < wanted:
                tally.speed_against_gap += 1
                contradicted = True

        if contradicted:
            tally.contradicted += 1

    return tally


def timestamp(total: int) -> str:
    """Un nombre de millisecondes, tel que `inspect` l'écrit."""
    hours, rest = divmod(total, 3600000)
    minutes, rest = divmod(rest, 60000)
    seconds, thousandths = divmod(rest, 1000)
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}.{thousandths:03d}"


def confront(files: list[Path], binary: str) -> int:
    """Confronte le parcours de ce script au lecteur du projet.

    Trois nombres par fichier, et ce sont ceux qu'`inspect` écrit déjà : combien
    de sous-titres, où commence le premier, où finit le dernier. Ils ne prouvent
    pas que chaque position est juste ; ils prouvent que le découpage en
    répliques et la lecture d'un horodatage le sont, ce qui est exactement là où
    un parcours écrit à la main se trompe."""
    span = re.compile(r"span:\s*(\S+) -> (\S+)")
    count = re.compile(r"subtitles:\s*(\d+)")

    diverging = 0
    for path in files:
        cues = cues_of(path)
        if not cues:
            continue
        told = subprocess.run([binary, "inspect", str(path)],
                              capture_output=True, text=True, check=False).stdout
        counted = count.search(told)
        bounds = span.search(told)
        if counted is None or bounds is None:
            print(f"  le lecteur n'a rien dit de {path}", file=sys.stderr)
            diverging += 1
            continue

        mine = (len(cues), timestamp(cues[0][0]), timestamp(max(end for _, end, _ in cues)))
        theirs = (int(counted.group(1)), bounds.group(1), bounds.group(2))
        if mine != theirs:
            print(f"  {path}\n    ici    {mine}\n    lecteur {theirs}", file=sys.stderr)
            diverging += 1

    if diverging:
        print(f"{diverging} fichier(s) lus différemment du lecteur du projet", file=sys.stderr)
    else:
        print(f"{len(files)} fichiers, lus comme le lecteur du projet les lit")
    return diverging


def share(part: int, whole: int) -> str:
    return f"{100 * part / whole:.1f} %" if whole else "—"


def report(total: Tally, per_file: list[tuple[str, Tally]], limits: argparse.Namespace) -> None:
    def line(label: str, count: int, whole: int) -> None:
        print(f"    {label:<34}{count:8d}   {share(count, whole):>7}")

    counted = total.subtitles - total.not_ordered

    print()
    print(f"{BOLD}violé tel quel{RESET}   — sur {counted} sous-titres dont la durée se compare")
    line("plus court que le minimum", total.too_short, counted)
    line("plus long que le maximum", total.too_long, counted)
    line("écart au suivant trop petit", total.gap_too_small, counted)
    line("plus lent que la vitesse", total.too_fast, counted)

    print()
    print(f"{BOLD}contradictoire{RESET}   — aucune fin ne satisfait les deux")
    line("minimum contre écart", total.minimum_against_gap, counted)
    line("vitesse contre écart", total.speed_against_gap, counted)
    line("vitesse contre maximum", total.speed_against_maximum, counted)
    line("au moins une des trois", total.contradicted, counted)

    if total.not_ordered:
        print()
        print(f"{BOLD}hors sujet{RESET}")
        line("fin avant ou sur le début", total.not_ordered, total.subtitles)

    print()
    print("Une contradiction n'est pas un cas de bord : c'est un sous-titre pour lequel")
    print("le cadrage devra dire laquelle des quatre contraintes est sacrifiée. Ce que")
    print("l'ordre de résolution choisi en fait se comptera quand l'ajustement existera.")

    print()
    print("  contredits   sous-titres   fichier")
    for name, tally in sorted(per_file, key=lambda pair: -pair[1].contradicted):
        counted_here = tally.subtitles - tally.not_ordered
        if tally.contradicted:
            print(f"  {share(tally.contradicted, counted_here):>10}"
                  f"   {tally.subtitles:11d}   {name}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("corpus", nargs="?", default=str(DEFAULT_CORPUS),
                        help="répertoire à parcourir (défaut : src/data)")
    parser.add_argument("--minimum", type=float, default=DEFAULT_MINIMUM,
                        help="durée minimale en secondes, zéro pour ne pas l'appliquer")
    parser.add_argument("--maximum", type=float, default=DEFAULT_MAXIMUM,
                        help="durée maximale en secondes, zéro pour ne pas l'appliquer")
    parser.add_argument("--gap", type=float, default=DEFAULT_GAP,
                        help="écart minimal entre deux sous-titres, en secondes")
    parser.add_argument("--speed", type=float, default=DEFAULT_SPEED,
                        help="vitesse de lecture en caractères par seconde, zéro pour ne pas "
                             "l'appliquer")
    parser.add_argument("--against", metavar="SUBEDIT_CLI",
                        help="confronte le parcours de ce script au lecteur du projet, "
                             "et ne mesure rien")
    limits = parser.parse_args()

    root = Path(limits.corpus)
    if not root.is_dir():
        print(f"corpus introuvable : {root}", file=sys.stderr)
        return 1

    read = sorted(path for path in root.rglob("*")
                  if path.is_file() and path.suffix.lower() in EXTENSIONS)
    skipped = sorted(path for path in root.rglob("*")
                     if path.is_file() and path.suffix.lower() not in EXTENSIONS)

    if not read:
        print(f"aucun fichier SubRip ni WebVTT sous {root}", file=sys.stderr)
        return 1

    if limits.against:
        return 1 if confront(read, limits.against) else 0

    total = Tally()
    per_file: list[tuple[str, Tally]] = []
    for path in read:
        cues = cues_of(path)
        if not cues:
            continue
        tally = census(cues, limits)
        total.add(tally)
        per_file.append((str(path.relative_to(root)), tally))

    print(f"{BOLD}{len(per_file)} fichiers, {total.subtitles} sous-titres{RESET}"
          f" — {len(skipped)} fichier(s) laissé(s) de côté, que ce parcours ne lit pas")
    print(f"minimum {limits.minimum} s · maximum {limits.maximum} s"
          f" · écart {limits.gap} s · vitesse {limits.speed} car/s")
    print("les défauts sont ceux de Gaupol, qui laisse cependant le maximum inactif")

    report(total, per_file, limits)
    return 0


if __name__ == "__main__":
    sys.exit(main())
