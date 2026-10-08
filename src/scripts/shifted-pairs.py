#!/usr/bin/env python3
"""Les paires décalées : un principal, une traduction posée à un décalage connu, et
la vérité de ce décalage.

La détection d'une traduction décalée (phase 14, D10) cherche un décalage constant et
ne doit jamais en proposer un que le fichier n'a pas. L'éprouver demande de savoir ce
qu'on a posé : une fixture tirée de ce que la détection trouve ne prouverait que son
accord avec elle-même. Ce script **pose** le décalage, l'écrit à côté (`verite.txt`),
et la détection est confrontée à lui.

    --check       (défaut) reconstruit chaque cas et le compare au disque
    --generate    réécrit les cas depuis la table

Chaque cas tient trois fichiers : `principal.srt`, `traduction.srt` et `verite.txt`,
qui porte le décalage posé en millisecondes (positif quand la traduction est en
retard), ou `aucun` quand les lignes n'ont pas un décalage constant — une dérive, un
décalage partiel, ou pas de décalage du tout.

**Les positions sont de l'arithmétique**, jamais un appel à nos commandes : décaler
avec `ShiftCommand` prouverait subedit contre subedit.
"""

from __future__ import annotations

import sys
from collections.abc import Callable
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DIRECTORY = REPO_ROOT / "src" / "test" / "data" / "decalages"

# Début et fin, en millisecondes. Des écarts qui ne se répètent pas : un principal
# périodique rendrait deux décalages également bons, et la détection devrait alors,
# à raison, n'en proposer aucun.
MAIN = [
    (1000, 3000), (4200, 5800), (8000, 11000), (12500, 14000),
    (17000, 19500), (20200, 21700), (25000, 28000), (29500, 31000),
    (34000, 36500), (37300, 38800), (42000, 45000), (46500, 48000),
]  # fmt: skip


def constant(milliseconds: int) -> Callable[[int, int], int]:
    return lambda _rank, _start: milliseconds


def drift(total: int) -> Callable[[int, int], int]:
    """Un retard qui croît de zéro à `total` sur la longueur du fichier."""
    return lambda rank, _start: round(total * rank / (len(MAIN) - 1))


def partial(milliseconds: int) -> Callable[[int, int], int]:
    """Le retard ne touche que la seconde moitié du fichier."""
    return lambda rank, _start: milliseconds if rank >= len(MAIN) // 2 else 0


# Le nom du cas, ce que le décalage fait à la ligne de rang n, et la vérité.
CASES: list[tuple[str, Callable[[int, int], int], str]] = [
    ("sans-decalage", constant(0), "aucun"),
    ("constant-plus-2000", constant(2000), "2000"),
    ("constant-moins-1500", constant(-1500), "-1500"),
    ("constant-plus-2340", constant(2340), "2340"),
    ("derive", drift(3000), "aucun"),
    ("partiel", partial(2000), "aucun"),
]


def stamp(milliseconds: int) -> str:
    sign = "-" if milliseconds < 0 else ""
    value = abs(milliseconds)
    return f"{sign}{value // 3600000:02d}:{value // 60000 % 60:02d}:{value // 1000 % 60:02d},{value % 1000:03d}"


def srt(rows: list[tuple[int, int, str]]) -> str:
    return "\n".join(
        f"{rank}\n{stamp(start)} --> {stamp(end)}\n{text}\n"
        for rank, (start, end, text) in enumerate(rows, 1)
    )


def build(shift: Callable[[int, int], int], truth: str) -> dict[str, str]:
    main = [(s, e, f"Line {n}.") for n, (s, e) in enumerate(MAIN, 1)]
    translated = []
    for rank, (start, end) in enumerate(MAIN):
        late = shift(rank, start)
        translated.append((start + late, end + late, f"Ligne {rank + 1}."))
    return {
        "principal.srt": srt(main),
        "traduction.srt": srt(translated),
        "verite.txt": truth + "\n",
    }


def expected() -> dict[Path, str]:
    files: dict[Path, str] = {}
    for name, shift, truth in CASES:
        for file, content in build(shift, truth).items():
            files[DIRECTORY / name / file] = content
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
