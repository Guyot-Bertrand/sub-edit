#!/usr/bin/env python3
"""Les conversions faites à la mauvaise fréquence : un fichier sur une grille, converti
par un rapport faux, et la vérité de ce qui le remet d'aplomb.

La recherche de la phase 14 (D11, #386) essaie les cinquante-six paires ordonnées des huit
fréquences normalisées et laisse la déduction juger. L'éprouver demande de savoir ce qu'on a
posé : une fixture tirée de ce que la recherche trouve ne prouverait que son accord avec
elle-même. Ce script **pose** la conversion fausse, en arithmétique exacte, et écrit à côté
le rapport qui la défait.

    --check       (défaut) reconstruit chaque cas et le compare au disque
    --generate    réécrit les cas depuis la table

Chaque cas tient deux fichiers : `fichier.srt` et `verite.txt`, qui porte le rapport à
appliquer aux positions pour les remettre sur la grille (`6/5`), `aucun` quand rien ne
convient — un fichier propre, sans grille, ou converti par un rapport hors de l'ensemble —,
ou `egalite` quand deux rapports conviennent également.

**Les positions sont de l'arithmétique**, jamais un appel à nos commandes : convertir avec
`ConvertFrameRateCommand` prouverait subedit contre subedit. Chaque position vaut
`round(n × 1000 / R)` pour une image `n`, puis `round(position × f)` pour le rapport faux
`f` — les deux arrondis que fait un vrai fichier converti.
"""

from __future__ import annotations

import sys
from fractions import Fraction
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DIRECTORY = REPO_ROOT / "src" / "test" / "data" / "conversions-fausses"

LENGTH_MS = 600_000
SHORT_MS = 10_000


class Steps:
    """Le générateur congruentiel de `subtitle-fixtures.py` : un pas irrégulier."""

    def __init__(self, seed: int) -> None:
        self._state = seed

    def between(self, low: int, high: int) -> int:
        self._state = (1103515245 * self._state + 12345) % (2**31)
        return low + self._state % (high - low + 1)


def rounded(value: Fraction) -> int:
    """Au plus proche, la moitié vers le haut."""
    return (value + Fraction(1, 2)).__floor__()


def frame_starts(rate: Fraction, span_ms: int, seed: int) -> list[int]:
    # Une étendue courte est dense, comme `grille-24-courte` : dix positions au moins, sans quoi
    # la déduction ne rend aucun verdict.
    low, high = (150, 400) if span_ms <= SHORT_MS else (2000, 5000)
    steps = Steps(seed)
    starts: list[int] = []
    moment = Fraction(steps.between(low, high), 1000)
    while moment * 1000 < span_ms:
        frame = rounded(moment * rate)
        starts.append(rounded(Fraction(frame * 1000) / rate))
        moment += Fraction(steps.between(low, high), 1000)
    return starts


def stamp(milliseconds: int) -> str:
    return (
        f"{milliseconds // 3600000:02d}:{milliseconds // 60000 % 60:02d}:"
        f"{milliseconds // 1000 % 60:02d},{milliseconds % 1000:03d}"
    )


def srt(starts: list[int]) -> str:
    return "\n".join(
        f"{rank}\n{stamp(start)} --> {stamp(start + 1500)}\nLine {rank}.\n"
        for rank, start in enumerate(starts, 1)
    )


FPS_24 = Fraction(24)
FPS_25 = Fraction(25)
FPS_30 = Fraction(30)
FPS_23976 = Fraction(24000, 1001)

# Le nom du cas, la grille du fichier, le rapport faux posé dessus (ou `None` : aucun), l'étendue,
# et la vérité : le rapport qui le remet d'aplomb, `aucun` ou `egalite`.
# Une conversion fausse a rarement une seule réparation : une grille à 28,8 images par seconde se
# remet aussi bien sur 24 que sur 30 avec un rapport de l'ensemble, et un fichier dont tous les
# temps sont doublés reste sur la grille de 25 — la déduction ne distingue ni l'un ni l'autre —,
# si bien que la recherche ne propose alors rien. Les cas à vérité unique sont ceux où l'on a converti un fichier déjà à la fréquence visée
# — le geste le plus courant —, et un second que l'énumération des 320 combinaisons (grille × rapport) trouve seul de son espèce.
CASES: list[tuple[str, Fraction, Fraction | None, int, str]] = [
    ("propre-24", FPS_24, None, LENGTH_MS, "aucun"),
    ("faux-25-vers-24", FPS_24, Fraction(25, 24), LENGTH_MS, "24/25"),
    ("faux-25-vers-60", Fraction(60), Fraction(5, 12), LENGTH_MS, "12/5"),
    ("egalite-24-vers-25", FPS_25, Fraction(24, 25), LENGTH_MS, "egalite"),
    ("egalite-50-vers-30", FPS_30, Fraction(5, 3), LENGTH_MS, "egalite"),
    ("egalite-28-8", FPS_24, Fraction(5, 6), LENGTH_MS, "egalite"),
    ("egalite-23976", FPS_25, Fraction(1001, 1000), LENGTH_MS, "egalite"),
    ("egalite-courte", FPS_24, Fraction(25, 24), SHORT_MS, "egalite"),
    ("hors-ensemble", FPS_24, Fraction(21, 20), LENGTH_MS, "aucun"),
    ("sans-grille", Fraction(263, 10), None, LENGTH_MS, "aucun"),
]


def build(rate: Fraction, wrong: Fraction | None, span_ms: int, seed: int) -> str:
    starts = frame_starts(rate, span_ms, seed)
    if wrong is not None:
        starts = [rounded(start * wrong) for start in starts]
    return srt(starts)


def expected() -> dict[Path, str]:
    files: dict[Path, str] = {}
    for seed, (name, rate, wrong, span_ms, truth) in enumerate(CASES, 1):
        files[DIRECTORY / name / "fichier.srt"] = build(rate, wrong, span_ms, seed)
        files[DIRECTORY / name / "verite.txt"] = truth + "\n"
    return files


def main(argv: list[str]) -> int:
    wanted = expected()
    if "--generate" in argv:
        for path, content in wanted.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        return 0

    failures = []
    for path, content in wanted.items():
        if not path.exists():
            failures.append(f"manque : {path.relative_to(REPO_ROOT)}")
        elif path.read_text(encoding="utf-8") != content:
            failures.append(f"diffère : {path.relative_to(REPO_ROOT)}")
    known = {path.parent.name for path in wanted}
    if DIRECTORY.exists():
        for entry in sorted(DIRECTORY.iterdir()):
            if entry.is_dir() and entry.name not in known:
                failures.append(f"en trop : {entry.relative_to(REPO_ROOT)}")
    for failure in failures:
        print(failure, file=sys.stderr)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
