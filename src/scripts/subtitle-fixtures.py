#!/usr/bin/env python3
"""Les fixtures de sous-titres sur grille connue : les fabriquer, et vérifier
qu'elles sont bien ce que le projet dit qu'elles sont.

La phase 16 déduit la fréquence d'image d'un fichier depuis ses positions, en
mesurant si elles tombent sur une grille. L'éprouver demande de l'étendue et du
volume : la fixture la plus fournie du dépôt porte six horodatages, et deux
horodatages ne disent rien d'une grille. Il faut des fichiers de plusieurs
centaines de répliques, et un fichier de plusieurs centaines de répliques est
illisible dans un diff.

Ce script est ce qui les rend **vérifiables plutôt que crues** : la table de ce
que chaque fixture est vit ici, la fabrication en découle, et `--check`
reconstruit chaque fichier pour le confronter à celui du disque.

    --check       (défaut) reconstruit chaque fixture et la compare au disque
    --generate    réécrit les fixtures depuis la table
    --measure     écrit la concentration de chaque fixture sur les huit candidates
    --weight      écrit le poids total, en octets

**Les positions sont de l'arithmétique, jamais un appel à nos commandes.** Un
fichier « déjà décalé » vaut `round(n × 1000 / R) + décalage` ; un fichier
« déjà transformé » vaut une affine appliquée aux millisecondes. Passer par
`ShiftCommand` ou par la conversion de fréquence prouverait subedit contre
subedit : une erreur d'arrondi partagée entre l'opération et la déduction
passerait au vert. Ce qu'on ne prouve pas ainsi — que le décalage *de Gaupol*
préserve la grille — est une affirmation sur Gaupol, dont rien ici n'a besoin.

**Ce script n'exige aucun outil.** Contrairement à `video-fixtures.sh`, qui
demande ffmpeg et ffprobe, tout est ici de l'arithmétique exacte : la
reconstruction est reproductible partout, et `--check` peut donc comparer octet
à octet plutôt que d'interroger un tiers.
"""

from __future__ import annotations

import math
import re
import sys
from collections.abc import Callable
from fractions import Fraction
from functools import partial
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
FIXTURE_DIR = REPO_ROOT / "src" / "test" / "data" / "grilles"

# Le poids total admis. Il est large — environ le double du poids réel — et son
# rôle est d'attraper un ordre de grandeur, pas un octet : un jeu de fixtures
# qui grossit sans qu'on le voie est une dette silencieuse.
WEIGHT_CEILING = 270000

# Les huit fréquences normalisées, dans l'ordre où `StandardFrameRate` les
# énumère. Les trois NTSC sont une fréquence entière divisée par 1,001 et n'ont
# aucune représentation exacte en virgule flottante : c'est la raison pour
# laquelle tout ici passe par des rationnels.
STANDARD_RATES = {
    "23-976": Fraction(24000, 1001),
    "24": Fraction(24),
    "25": Fraction(25),
    "29-97": Fraction(30000, 1001),
    "30": Fraction(30),
    "50": Fraction(50),
    "59-94": Fraction(60000, 1001),
    "60": Fraction(60),
}

# Dix minutes, et la raison tient en une division. Les grilles à 23,976 et à 24
# dérivent d'une milliseconde par seconde de film, et une image en dure 41,7 :
# la phase sur la mauvaise candidate fait donc **un tour complet toutes les
# quarante-deux secondes**. Dix minutes, c'est quatorze tours, et la mauvaise
# candidate s'effondre sans appel.
#
# C'est aussi ce qui donne son sens à la fixture courte, à l'envers : dix
# secondes, c'est un quart de tour, et les deux candidates restent
# indiscernables. La déduction doit le dire plutôt que trancher.
FULL_EXTENT_SECONDS = 600
SHORT_EXTENT_SECONDS = 10


class Lcg:
    """Un générateur congruentiel linéaire, écrit ici plutôt qu'emprunté.

    Le pas entre deux répliques doit être **irrégulier et reproductible**. Sans
    irrégularité, les phases deviennent périodiques et le fichier à fréquence
    absurde ressortirait concentré sur une candidate qu'il ne touche pas ; sans
    reproductibilité, `--check` ne veut plus rien dire.

    `random` de Python offrirait la seconde et pas la première garantie sur le
    long terme : sa suite est stable en pratique, mais rien ne le promet d'une
    version à l'autre. Douze lignes écrites ici valent cette promesse-là, et
    elles rendent la suite refaisable par qui réimplémenterait le générateur
    dans une autre langue.
    """

    def __init__(self, seed: int) -> None:
        self._state = seed

    def _next(self) -> int:
        self._state = (1103515245 * self._state + 12345) % (2**31)
        # Les bits de poids faible d'un congruentiel sont les moins bons ; on
        # prend au-dessus d'eux.
        return self._state >> 8

    def between(self, low: int, high: int) -> int:
        """Un entier entre `low` et `high`, bornes comprises."""
        return low + self._next() % (high - low + 1)


def frames_of(seconds: float, rate: Fraction) -> int:
    """Combien d'images tiennent dans cette durée, à cette fréquence."""
    return int(Fraction(seconds).limit_denominator(1000) * rate)


def milliseconds_of(frame: int, rate: Fraction) -> int:
    """La position de cette image, en millisecondes entières.

    L'arrondi est au plus proche, la moitié vers le haut, et il est écrit à la
    main : `round` de Python arrondit à l'entier pair, ce qui n'est pas ce que
    fait un logiciel de sous-titres.
    """
    exact = Fraction(frame * 1000) / rate
    return (2 * exact.numerator + exact.denominator) // (2 * exact.denominator)


def timestamp_of(milliseconds: int) -> str:
    """La forme SubRip d'une position : `01:23:45,678`."""
    hours, rest = divmod(milliseconds, 3600000)
    minutes, rest = divmod(rest, 60000)
    seconds, rest = divmod(rest, 1000)
    return f"{hours:02d}:{minutes:02d}:{seconds:02d},{rest:03d}"


def grid_cues(
    rate: Fraction,
    extent_seconds: float,
    seed: int,
    gap_seconds: tuple[float, float] = (2.0, 5.0),
    shortest_cue_seconds: float = 1.0,
) -> list[tuple[int, int]]:
    """Les répliques d'un fichier parfaitement calé, en numéros d'image.

    Une réplique dure de une à quelques secondes et laisse un intervalle avant
    la suivante : c'est la densité d'un dialogue, et c'est elle qui donne au
    fichier un pas irrégulier.

    `gap_seconds` existe pour la fixture courte, et pour elle seule. Une étendue
    faible et un dialogue de densité normale ne donnent que huit répliques, et
    huit points ne disent rien : la concentration d'une candidate quelconque y
    vaut déjà `1/√8`, soit trente-cinq pour cent. Le hasard n'est pas
    l'ambiguïté.
    """
    steps = Lcg(seed)
    shortest_gap = frames_of(gap_seconds[0], rate)
    longest_gap = frames_of(gap_seconds[1], rate)
    shortest_cue = frames_of(shortest_cue_seconds, rate)
    breath = frames_of(shortest_cue_seconds / 2, rate)

    cues: list[tuple[int, int]] = []
    frame = frames_of(1.0, rate)
    last = frames_of(extent_seconds, rate)
    while frame <= last:
        gap = steps.between(shortest_gap, longest_gap)
        # La réplique s'arrête avant la suivante : la borne haute est
        # l'intervalle moins une respiration, jamais l'intervalle lui-même.
        # `max` plutôt que la soustraction nue : à densité serrée, l'intervalle
        # moins la respiration peut descendre sous la réplique la plus courte,
        # et une borne haute inférieure à la borne basse ferait un tirage vide.
        cues.append((frame, frame + steps.between(shortest_cue, max(shortest_cue, gap - breath))))
        frame += gap
    return cues


def render(lines: list[tuple[int, int, str]]) -> str:
    """Un fichier SubRip, en UTF-8 et fins de ligne à l'unix.

    Un bloc par réplique, séparés par une ligne vide, et une ligne vide après
    le dernier : c'est ce que portent déjà les fixtures de `valides/`.
    """
    blocks = []
    for index, (start, end, text) in enumerate(lines, start=1):
        blocks.append(f"{index}\n{timestamp_of(start)} --> {timestamp_of(end)}\n{text}\n")
    return "\n".join(blocks) + "\n"


def on_grid(rate: Fraction, extent: float, seed: int, offset: int = 0, **density
            ) -> list[tuple[int, int, str]]:
    """Un fichier dont chaque position est sur la grille, à `offset` près."""
    return [
        (
            milliseconds_of(start, rate) + offset,
            milliseconds_of(end, rate) + offset,
            f"Réplique {index}, image {start}.",
        )
        for index, (start, end) in enumerate(grid_cues(rate, extent, seed, **density), start=1)
    ]


def retimed_tail(rate: Fraction, extent: float, seed: int) -> list[tuple[int, int, str]]:
    """Deux tiers sur la grille, un tiers retimé — un fichier assemblé.

    La transformation par deux repères **détruit** la grille : c'est la seule
    des trois opérations de Gaupol dont la déduction ne se relève pas. Appliquée
    au dernier tiers seulement, elle donne le cas partiel dont les écarts sont
    *groupés*, celui qui ressemble à une section recalée.

    L'affine est écrite ici, en millisecondes, plutôt qu'obtenue en appelant
    `TransformCommand` : c'est la règle de tout ce fichier.
    """
    lines = on_grid(rate, extent, seed)
    boundary = 2 * len(lines) // 3

    def moved(value: int) -> int:
        # Une pente qui n'est le rapport d'aucune paire de fréquences
        # normalisées, et une ordonnée à l'origine qui n'est un multiple
        # d'aucune durée d'image : rien de ce que la déduction cherche ne peut
        # retomber dessus.
        return (value * 10007 + 1370000) // 10000

    return lines[:boundary] + [
        (moved(start), moved(end), f"Réplique {index}, hors grille.")
        for index, (start, end, _) in enumerate(lines[boundary:], start=boundary + 1)
    ]


def scattered_faults(rate: Fraction, extent: float, seed: int) -> list[tuple[int, int, str]]:
    """Une position sur cinq écartée de plusieurs images.

    C'est l'autre visage du fichier partiel : non pas une section recalée, mais
    l'édition à la main, réplique par réplique.

    **L'écart se compte en images, pas en millisecondes.** Une première version
    déplaçait de neuf millisecondes au plus, en croyant sortir de la grille : à
    25 images par seconde une image en dure quarante, et neuf millisecondes ne
    sont qu'un quarantième de tour. La position restait pratiquement sur la
    grille, et le fichier ressortait à 92 — au-dessus du seuil de grille nette
    qu'il est précisément censé manquer. Un humain qui corrige une position dans
    la table la déplace de ce qu'il voit, et la phase en devient quelconque.
    """
    lines = on_grid(rate, extent, seed)
    frame = milliseconds_of(1, rate)
    drift = Lcg(seed + 1)
    faulty = []
    for index, (start, end, text) in enumerate(lines, start=1):
        if index % 5 != 0:
            faulty.append((start, end, text))
            continue
        nudge = drift.between(frame + 1, 10 * frame - 1)
        # Un multiple exact de la durée d'une image retomberait sur la grille,
        # et l'écart ne se verrait pas.
        nudge += 1 if nudge % frame == 0 else 0
        nudge *= 1 if drift.between(0, 1) else -1
        faulty.append((start + nudge, end + nudge, f"Réplique {index}, hors grille."))
    return faulty


# nom | ce que la fixture est | comment elle se construit
#
# La graine diffère d'une fixture à l'autre pour qu'aucune paire de fichiers ne
# porte le même dialogue à deux fréquences près : deux fixtures jumelles se
# liraient comme une seule.
Fixture = Callable[[], list[tuple[int, int, str]]]

FIXTURES: list[tuple[str, str, Fixture]] = [
    *(
        (
            f"grille-{name}.srt",
            f"grille parfaite à {name.replace('-', ',')} images par seconde, dix minutes",
            partial(on_grid, rate, FULL_EXTENT_SECONDS, seed),
        )
        for seed, (name, rate) in enumerate(STANDARD_RATES.items(), start=1)
    ),
    (
        "grille-absurde.srt",
        "grille à 26,3 images par seconde — muette sur les huit candidates",
        partial(on_grid, Fraction(263, 10), FULL_EXTENT_SECONDS, 101),
    ),
    (
        "grille-24-decalee.srt",
        "grille parfaite à 24, translatée de 2 999 ms",
        partial(on_grid, Fraction(24), FULL_EXTENT_SECONDS, 102, offset=2999),
    ),
    (
        "grille-24-courte.srt",
        "grille parfaite à 24, dix secondes serrées — étendue insuffisante",
        partial(
            on_grid,
            Fraction(24),
            SHORT_EXTENT_SECONDS,
            103,
            gap_seconds=(0.2, 0.35),
            shortest_cue_seconds=0.1,
        ),
    ),
    (
        "melange-groupe.srt",
        "29,97 sur deux tiers, dernier tiers retimé — écarts groupés",
        partial(retimed_tail, Fraction(30000, 1001), FULL_EXTENT_SECONDS, 104),
    ),
    (
        "melange-disperse.srt",
        "25, une position sur cinq écartée — écarts dispersés",
        partial(scattered_faults, Fraction(25), FULL_EXTENT_SECONDS, 105),
    ),
]

# --------------------------------------------------------------------------
# L'instrument : ce que ces fixtures donnent, et pourquoi elles sont celles-là
# --------------------------------------------------------------------------
#
# `--measure` n'entre dans aucune porte et rien n'en dépend. Il existe pour que
# les chiffres du LISEZMOI soient **refaisables par une commande** plutôt que
# rapportés : sans lui, ils auraient le défaut qu'on reproche aux relevés du
# corpus privé — un nombre que le lecteur doit croire.
#
# Il réimplémente la méthode que la phase 16 écrira en C++, et c'est assumé. Ce
# n'est pas une porte et ce n'est pas la référence : la référence sera le code
# de production et les constantes de ses tests. C'est un second calcul,
# indépendant, sur des fichiers versionnés — et deux implémentations qui
# tombent d'accord valent mieux qu'une qui s'auto-atteste.

TIMESTAMP = re.compile(r"(\d\d):(\d\d):(\d\d),(\d\d\d) -->")


def starts_of(path: Path) -> list[int]:
    """Les débuts d'un fichier SubRip, en millisecondes.

    Les débuts seuls : partout où une grille existe, ils sont à 100 sans
    exception, là où les fins vont de 55 à 100 — un *cue-out* est souvent
    calculé par une règle de vitesse de lecture, et non posé sur une image.
    """
    return [
        ((int(h) * 60 + int(m)) * 60 + int(s)) * 1000 + int(ms)
        for h, m, s, ms in TIMESTAMP.findall(path.read_text(encoding="utf-8"))
    ]


def concentration(positions: list[int], rate: Fraction) -> float:
    """La concentration des phases sur la grille de `rate`, de 0 à 100.

    Chaque position devient un angle — sa phase sur la grille — et la
    concentration est la longueur du vecteur moyen. Cent : une grille parfaite,
    quelle que soit sa phase. Près de zéro : aucune structure.

    Aucun paramètre de tolérance, et insensible à un décalage : c'est ce qui la
    distingue d'une recherche de grille à phase nulle, laquelle notait à zéro un
    24 images par seconde parfait translaté d'une milliseconde et demie.
    """
    turns = [2 * math.pi * ((position * float(rate) / 1000.0) % 1.0) for position in positions]
    resultant = math.hypot(
        sum(math.cos(turn) for turn in turns), sum(math.sin(turn) for turn in turns)
    )
    return 100.0 * resultant / len(positions)


RED = "\033[31m"
GREEN = "\033[32m"
BOLD = "\033[1m"
RESET = "\033[0m"


def generate() -> int:
    print(f"{BOLD}fabrication des fixtures de grille{RESET}")
    FIXTURE_DIR.mkdir(parents=True, exist_ok=True)
    for name, description, fixture in FIXTURES:
        content = render(fixture())
        (FIXTURE_DIR / name).write_text(content, encoding="utf-8")
        cues = content.count(" --> ")
        print(f"  {GREEN}✓{RESET} {name} — {description}, {cues} répliques, {len(content)} octets")
    return 0


def check() -> int:
    failures = 0
    for name, description, fixture in FIXTURES:
        path = FIXTURE_DIR / name
        if not path.is_file():
            print(
                f"  {RED}✗{RESET} {name} — absente ;"
                " ./src/scripts/subtitle-fixtures.py --generate",
                file=sys.stderr,
            )
            failures += 1
            continue

        expected = render(fixture())
        actual = path.read_text(encoding="utf-8")
        if actual != expected:
            print(
                f"  {RED}✗{RESET} {name} — le fichier n'est plus ce que la table dit ;"
                " ./src/scripts/subtitle-fixtures.py --generate",
                file=sys.stderr,
            )
            failures += 1
            continue

        print(f"  {GREEN}✓{RESET} {name} — {description}, {expected.count(' --> ')} répliques")

    total = weight()
    if total > WEIGHT_CEILING:
        print(
            f"  {RED}✗{RESET} poids total {total} octets, maximum {WEIGHT_CEILING}",
            file=sys.stderr,
        )
        failures += 1

    if failures:
        print(f"{RED}{failures} écart(s) entre les fixtures et la table.{RESET}", file=sys.stderr)
        return 1

    print(f"  poids total : {total} octets")
    return 0


def measure() -> int:
    print(f"{BOLD}concentration de chaque fixture sur les huit candidates{RESET}")
    print("  " + " " * 24 + "  ".join(f"{name:>6s}" for name in STANDARD_RATES))
    for name, _, _ in FIXTURES:
        path = FIXTURE_DIR / name
        if not path.is_file():
            print(f"  {RED}✗{RESET} {name} — absente", file=sys.stderr)
            return 1
        positions = starts_of(path)
        scores = [concentration(positions, rate) for rate in STANDARD_RATES.values()]
        print(f"  {name:24s}" + "  ".join(f"{score:6.1f}" for score in scores))
    return 0


def weight() -> int:
    return sum(
        (FIXTURE_DIR / name).stat().st_size
        for name, _, _ in FIXTURES
        if (FIXTURE_DIR / name).is_file()
    )


def main() -> int:
    argument = sys.argv[1] if len(sys.argv) > 1 else "--check"
    if argument == "--check":
        return check()
    if argument == "--generate":
        return generate()
    if argument == "--measure":
        return measure()
    if argument == "--weight":
        print(weight())
        return 0
    print(
        f"usage : {Path(sys.argv[0]).name} [--check|--generate|--measure|--weight]",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
