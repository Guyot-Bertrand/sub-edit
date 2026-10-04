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

**Ce qu'il ne fait pas**, et c'est délibéré : appliquer l'ajustement. Le tableau
prédit vaut parce qu'il ne réimplémente pas `adjustDurations` : il compte ce que
les **définitions** disent, sur les positions écrites, et ne sait pas dans quel
ordre le noyau résout. S'il le faisait, il cesserait d'être un témoin
indépendant de celui du noyau.

## Le recoupement avec `subedit-cli adjust` — issue #560, qui tient le renvoi de #407

Le noyau déclare ce qu'il sacrifie (`counts.sacrificed.{speed,minimum,gap}`) ;
ce script **prédit** les mêmes trois comptes depuis les définitions. S'ils
diffèrent, l'un des deux a tort. La correspondance, établie en relisant
`adjustDurations` et ce parcours :

    place = suivant.début − début − écart            (millisecondes entières)

    sacrificed.minimum  ↔  place < minimum           (suivant existant)
                           ou maximum < minimum      (le noyau le compte ici)
    sacrificed.speed    ↔  place < besoin   OU   besoin > maximum
                           — l'UNION des deux paires, jamais leur somme : un
                           sous-titre dans les deux compte une fois
    sacrificed.gap      ↔  place < 0

    besoin = longueur × 1000 / vitesse, ARRONDI à la milliseconde

- **Le besoin est arrondi, comme le noyau l'arrondit** (`readingTimeOf`) : sans
  cela, « Hi » à 15 caractères par seconde a besoin de 133,33 ms et une place de
  133 ms passerait pour insuffisante ici, suffisante là-bas. C'est la première
  divergence que la fixture a montrée, et c'était ce script qui avait tort.
- **« Suivant » est le suivant dans le fichier**, des deux côtés : le noyau ne
  trie pas à la lecture. Un sous-titre sans suivant n'a ni place ni écart à tenir.
- **La place ne dépend pas de la durée écrite.** Un sous-titre dont la fin
  précède le début (« hors sujet » pour les durées *telles quelles*) compte quand
  même pour ce qu'il est : le noyau le traite comme les autres.
- **Le maximum** est posé des deux côtés (`--maximum 6`) : le noyau ne l'allume
  pas par défaut et le compte ne le nomme pas — il tient toujours — mais il
  décide du compte de la vitesse. Les comptes coïncident si le maximum est éteint
  ou au moins égal au minimum ; sinon le noyau compte un minimum que le script
  voit aussi, et la correspondance ci-dessus le dit.
- **Une réserve, sur un cas qu'aucun fichier sain ne produit** : si le minimum et
  la vitesse sont éteints et qu'une fin précède son début *et* que la place est
  négative, le noyau ne compte pas l'écart, parce que la fin n'a pas été relevée.
  Avec le minimum actif — le défaut — la fin est toujours relevée au début.

Deux contrôles, qui sont les deux moitiés de la preuve :

    # le script, contre les comptes écrits à la main (aucun binaire requis)
    ./src/scripts/measure-duration-constraints.py --check-fixtures

    # le noyau, contre le script, sur un corpus
    ./src/scripts/measure-duration-constraints.py [répertoire] --crosscheck ./build/dev/bin/subedit-cli

Le premier tourne dans `make check-local` ; le noyau, lui, est confronté aux
mêmes attendus par un cas de bout en bout (`CLI-ADJUST-06`). Le second est une
observation : il dit **globalement** si les deux comptes s'accordent sur un
corpus, et un corpus privé ne se nomme jamais.
"""

import argparse
import json
import math
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
        "sacrificed_speed",
        "sacrificed_minimum",
        "sacrificed_gap",
    )

    def __init__(self) -> None:
        for field in self.FIELDS:
            setattr(self, field, 0)

    def add(self, other: "Tally") -> None:
        for field in self.FIELDS:
            setattr(self, field, getattr(self, field) + getattr(other, field))


def rounded(value: float) -> int:
    """Au plus proche, les moitiés s'éloignant de zéro : `llround` du noyau.

    `round()` de Python arrondit les moitiés au pair, ce qui n'est pas la même
    fonction et n'a aucune raison d'être celle d'un instrument de recoupement."""
    return int(math.floor(value + 0.5)) if value >= 0 else -int(math.floor(-value + 0.5))


def milliseconds_of(seconds: float) -> int | None:
    """Une borne en secondes, en millisecondes — rien quand elle est à zéro, la
    convention de ce script pour « ne pas l'appliquer »."""
    return rounded(seconds * 1000) if seconds else None


def needed_of(length: int, speed: float) -> int | None:
    """La durée que `length` caractères demandent, à la milliseconde.

    **Arrondie comme le noyau l'arrondit** (`readingTimeOf`) : la comparer en
    flottants dirait « trop court » d'une place de 133 ms pour un besoin de
    133,33 ms que le noyau tient pour 133."""
    if not speed or not length:
        return None
    return rounded(length * 1000 / speed)


def census(cues: list[tuple[int, int, int]], limits: argparse.Namespace) -> Tally:
    """Le recensement d'un fichier.

    Une contrainte posée à zéro ne s'applique pas — c'est la convention de ce
    script, et c'est aussi ce que Gaupol fait par accident pour les deux
    durées : son `minimum and …` est faux quand le minimum vaut zéro.

    Tout est en millisecondes entières : ce sont celles du noyau, et un compte
    qui dépend d'un arrondi flottant n'est pas comparable au sien."""
    tally = Tally()
    minimum = milliseconds_of(limits.minimum)
    maximum = milliseconds_of(limits.maximum)
    gap = rounded(limits.gap * 1000)

    for position, (start, end, length) in enumerate(cues):
        tally.subtitles += 1

        needed = needed_of(length, limits.speed)
        following = cues[position + 1][0] if position + 1 < len(cues) else None
        room = following - start - gap if following is not None else None

        # **Ce que l'ajustement sacrifierait**, depuis les définitions seules. La
        # place ne dépend pas de la durée écrite : ces trois comptes se font avant
        # le tri de ce qui est « hors sujet » pour les durées telles quelles.
        if minimum is not None and ((room is not None and room < minimum)
                                    or (maximum is not None and maximum < minimum)):
            tally.sacrificed_minimum += 1
        if needed is not None and ((room is not None and room < needed)
                                   or (maximum is not None and needed > maximum)):
            tally.sacrificed_speed += 1
        if room is not None and room < 0:
            tally.sacrificed_gap += 1

        duration = end - start
        if duration <= 0:
            # Une fin avant son début n'a pas de durée à comparer, et compter
            # une durée négative comme « trop courte » noierait le vrai chiffre.
            tally.not_ordered += 1
            continue

        if minimum is not None and duration < minimum:
            tally.too_short += 1
        if maximum is not None and duration > maximum:
            tally.too_long += 1
        if needed is not None and duration < needed:
            tally.too_fast += 1

        contradicted = False
        if needed is not None and maximum is not None and needed > maximum:
            tally.speed_against_maximum += 1
            contradicted = True

        if following is not None:
            if following - end < gap:
                tally.gap_too_small += 1
            if minimum is not None and room < minimum:
                tally.minimum_against_gap += 1
                contradicted = True
            if needed is not None and room < needed:
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


def sacrificed_of(tally: Tally) -> tuple[int, int, int]:
    """Les trois comptes que `adjust` déclare, dans l'ordre de son objet."""
    return (tally.sacrificed_speed, tally.sacrificed_minimum, tally.sacrificed_gap)


FIXTURES = REPO_ROOT / "src" / "test" / "data" / "durees"
EXPECTED = REPO_ROOT / "src" / "test" / "data" / "attendus" / "json"


def limits_of(constraints: dict) -> argparse.Namespace:
    """Les réglages qu'une ligne attendue déclare, comme ce script les lit.

    **L'attendu porte ses propres réglages** (`constraints`), si bien qu'une
    ligne ne peut pas être confrontée à d'autres que ceux pour lesquels elle a
    été écrite. Une contrainte éteinte (`null`) est une borne à zéro ici."""
    speed = constraints["speed"]
    return argparse.Namespace(
        speed=float(speed["cps"]) if speed else 0.0,
        minimum=(constraints["minimum_ms"] or 0) / 1000.0,
        maximum=(constraints["maximum_ms"] or 0) / 1000.0,
        gap=(constraints["gap_ms"] or 0) / 1000.0)


def check_fixtures() -> int:
    """Confronte ce script aux comptes **écrits à la main** pour la fixture.

    C'est la moitié de la preuve qui n'exige aucun binaire : les attendus de
    `src/test/data/attendus/json/recoupement-*.jsonl` sont calculés sur le papier
    à partir des définitions, et le même test de bout en bout les confronte au
    noyau. Que les deux outils s'accordent avec eux, c'est qu'ils s'accordent."""
    expectations = sorted(EXPECTED.glob("recoupement-*.jsonl"))
    if not expectations:
        print(f"aucun attendu de recoupement sous {EXPECTED}", file=sys.stderr)
        return 1

    wrong = 0
    checked = 0
    for expectation in expectations:
        for number, line in enumerate(expectation.read_text(encoding="utf-8").splitlines(), 1):
            record = json.loads(line)
            path = FIXTURES / Path(record["file"]).name
            cues = cues_of(path)
            tally = census(cues, limits_of(record["constraints"]))
            counts = record["counts"]
            expected = (counts["sacrificed"]["speed"], counts["sacrificed"]["minimum"],
                        counts["sacrificed"]["gap"])
            checked += 1
            if sacrificed_of(tally) != expected or tally.subtitles != counts["subtitles"]:
                wrong += 1
                print(f"  {expectation.name}:{number} {path.name}\n"
                      f"    attendu à la main {expected}\n"
                      f"    prédit ici        {sacrificed_of(tally)}", file=sys.stderr)

    if wrong:
        print(f"{wrong} compte(s) sur {checked} diffèrent de ceux écrits à la main",
              file=sys.stderr)
        return 1
    print(f"{checked} fichiers de la fixture, prédits comme les comptes écrits à la main")
    return 0


def arguments_of(limits: argparse.Namespace) -> list[str]:
    """Les réglages de ce script, dits à `subedit-cli adjust`.

    Zéro éteint ici une borne, et `off` fait de même là-bas ; le maximum, lui,
    n'a pas de `off` : ne pas le donner l'éteint."""
    flags = ["--speed", f"{limits.speed:g}" if limits.speed else "off",
             "--minimum", f"{limits.minimum:g}" if limits.minimum else "off",
             "--gap", f"{limits.gap:g}"]
    if limits.maximum:
        flags += ["--maximum", f"{limits.maximum:g}"]
    return flags


def crosscheck(files: list[Path], binary: str, limits: argparse.Namespace) -> int:
    """Confronte les trois comptes prédits à ceux que le noyau déclare.

    Un fichier à la fois, `--dry-run` : rien n'est écrit. **Le résultat se dit
    globalement** — combien de fichiers s'accordent, combien divergent, et la
    somme de chaque compte des deux côtés — parce qu'un corpus privé ne se nomme
    pas. Les divergences sont détaillées fichier par fichier sur la sortie
    d'erreur, pour qui cherche lequel des deux a tort : c'est un instrument, et
    sa sortie n'est pas un document."""
    agree = 0
    diverge: list[tuple[Path, tuple[int, int, int], tuple[int, int, int]]] = []
    refused = 0
    predicted_total = [0, 0, 0]
    core_total = [0, 0, 0]

    for path in files:
        cues = cues_of(path)
        if not cues:
            continue
        mine = sacrificed_of(census(cues, limits))
        run = subprocess.run([binary, "--format", "json", "adjust", "--dry-run",
                              *arguments_of(limits), str(path)],
                             capture_output=True, text=True, check=False)
        try:
            record = json.loads(run.stdout.splitlines()[0])
        except (IndexError, json.JSONDecodeError):
            record = {"ok": False}
        if not record.get("ok"):
            refused += 1
            continue

        sacrificed = record["counts"]["sacrificed"]
        theirs = (sacrificed["speed"], sacrificed["minimum"], sacrificed["gap"])
        for rank in range(3):
            predicted_total[rank] += mine[rank]
            core_total[rank] += theirs[rank]
        if mine == theirs:
            agree += 1
        else:
            diverge.append((path, mine, theirs))

    names = ("vitesse", "minimum", "écart")
    print(f"{agree + len(diverge)} fichiers comparés"
          f" — {agree} s'accordent, {len(diverge)} divergent, {refused} refusé(s) par le noyau")
    print("  somme des comptes     " + "  ".join(f"{name:>8}" for name in names))
    print("  prédite par le script " + "  ".join(f"{count:8d}" for count in predicted_total))
    print("  déclarée par le noyau " + "  ".join(f"{count:8d}" for count in core_total))

    for path, mine, theirs in diverge:
        print(f"  {path}\n    script {mine}\n    noyau  {theirs}", file=sys.stderr)
    return 1 if diverge else 0


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

    print()
    print(f"{BOLD}sacrifié (prédit){RESET}   — ce que `adjust` déclarerait, depuis les définitions")
    line("vitesse (l'union des deux paires)", total.sacrificed_speed, total.subtitles)
    line("minimum", total.sacrificed_minimum, total.subtitles)
    line("écart (place négative)", total.sacrificed_gap, total.subtitles)

    if total.not_ordered:
        print()
        print(f"{BOLD}hors sujet{RESET}")
        line("fin avant ou sur le début", total.not_ordered, total.subtitles)

    print()
    print("Une contradiction n'est pas un cas de bord : c'est un sous-titre pour lequel")
    print("toute fin viole au moins une contrainte. Ce que l'ajustement sacrifie, il le")
    print("compte lui-même ; `--crosscheck` confronte ses trois comptes à ceux qui sont")
    print("prédits ci-dessus (voir l'en-tête de ce script).")

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
    parser.add_argument("--check-fixtures", action="store_true",
                        help="confronte ce script aux comptes écrits à la main pour la fixture "
                             "versionnée, et ne mesure rien")
    parser.add_argument("--crosscheck", metavar="SUBEDIT_CLI",
                        help="confronte les trois comptes prédits à ceux que `subedit-cli "
                             "adjust --dry-run` déclare, fichier par fichier")
    limits = parser.parse_args()

    if limits.check_fixtures:
        return check_fixtures()

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

    if limits.crosscheck:
        return crosscheck(read, limits.crosscheck, limits)

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
