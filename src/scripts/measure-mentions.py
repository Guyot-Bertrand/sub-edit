#!/usr/bin/env python3
"""Mesure, sur un corpus de sous-titres, ce que valent les mentions pour
malentendants : combien de sous-titres en portent, sous quelle forme, et
combien deviennent vides si on les enlève.

C'est d'ici que viennent les proportions inscrites dans la fixture de mesure,
`src/test/bench/full_length_project.hpp`. Le corpus lu est celui de `src/data/`,
privé à chaque machine et ignoré par git : les chiffres, eux, sont recopiés
dans la fixture, qui ne lit aucun fichier à l'exécution. Ce script existe pour
qu'ils puissent être refaits — quand le corpus grandit, ou quand on doute d'eux.

    ./src/scripts/measure-mentions.py [répertoire]

Il n'entre dans aucune porte, et rien ne dépend de lui : c'est un instrument,
pas une vérification. Sur une machine sans corpus, il le dit et s'arrête.
"""

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_CORPUS = REPO_ROOT / "src" / "data"
EXTENSIONS = {".srt", ".vtt", ".sub"}

# Une mention tient sur une ligne : le motif de la phase 4 accepte le saut de
# ligne, mais le compter ici mélangerait deux formes qu'on veut distinguer.
MENTION = re.compile(r"\[[^\]\n]*\]|\([^)\n]*\)")
# La même, à cheval sur le saut de ligne — comptée à part.
STRADDLING = re.compile(r"\[[^\]]*\n[^\]]*\]|\([^)]*\n[^)]*\)")
# Une ligne entière qui n'est qu'une mention, tiret de dialogue toléré.
SOLE_LINE = re.compile(r"^\s*(?:-\s*)?(?:\[[^\]\n]*\]|\([^)\n]*\))\s*$")
# Les balises de mise en forme ne sont pas du texte : « <i>[soupir]</i> » est
# une mention seule, et le rester est ce qui rend le compte des vidés juste.
TAG = re.compile(r"</?[a-zA-Z][^>]*>")
TIMECODE = re.compile(r"\d{1,2}:\d{2}:\d{2}[.,]\d{1,3}\s*-->")


def decoded(raw: bytes) -> str:
    """Rend le texte du fichier, quelle que soit son encodage — un corpus réel
    en mêle plusieurs, et refuser de lire les uns fausserait les proportions."""
    for encoding in ("utf-8-sig", "cp1252"):
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError:
            continue
    return raw.decode("latin-1", "replace")


def texts_of(path: Path) -> list[str]:
    """Rend le texte de chaque sous-titre du fichier.

    Analyse volontairement grossière : un bloc, une ligne d'horodatage, et tout
    ce qui suit est du texte. Elle vaut pour SubRip comme pour WebVTT, et il
    s'agit de compter des mentions, pas de relire un lecteur qui existe."""
    content = decoded(path.read_bytes()).replace("\r\n", "\n").replace("\r", "\n")
    texts = []
    for block in re.split(r"\n[ \t]*\n", content):
        lines = [line for line in block.split("\n") if line.strip()]
        timecode = next((i for i, line in enumerate(lines) if TIMECODE.search(line)), None)
        if timecode is None:
            continue
        text = "\n".join(lines[timecode + 1:]).strip()
        if text:
            texts.append(TAG.sub("", text))
    return texts


def main() -> int:
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_CORPUS
    if not root.is_dir():
        print(f"corpus introuvable : {root}", file=sys.stderr)
        return 1

    files = sorted(
        path for path in root.rglob("*")
        if path.suffix.lower() in EXTENSIONS and "[write]" not in path.name)
    if not files:
        print(f"aucun fichier de sous-titres sous {root}", file=sys.stderr)
        return 1

    total = biting = emptied = sole_line = straddling = brackets = parentheses = 0
    two_lines = 0
    lengths: list[int] = []
    per_file: list[tuple[str, int, int]] = []

    for path in files:
        texts = texts_of(path)
        if not texts:
            continue
        file_biting = 0
        for text in texts:
            total += 1
            lengths.append(len(text.replace("\n", " ")))
            two_lines += 1 if "\n" in text else 0
            straddling += 1 if STRADDLING.search(text) else 0
            if not MENTION.search(text):
                continue
            file_biting += 1
            biting += 1
            brackets += len(re.findall(r"\[[^\]\n]*\]", text))
            parentheses += len(re.findall(r"\([^)\n]*\)", text))
            if not MENTION.sub("", text).strip(" -\t\n"):
                emptied += 1
            elif "\n" in text and any(SOLE_LINE.match(l) for l in text.split("\n")):
                sole_line += 1
        per_file.append((str(path.relative_to(root)), len(texts), file_biting))

    lengths.sort()
    count = len(lengths)

    def share(part: int, whole: int) -> str:
        return f"{100 * part / whole:.1f} %" if whole else "—"

    print(f"{len(per_file)} fichiers, {total} sous-titres")
    print()
    print(f"  portant une mention          {biting:6d}   {share(biting, total)} du total")
    print(f"  vidés par son retrait        {emptied:6d}   {share(emptied, biting)} des mordus")
    print(f"  dont une ligne entière       {sole_line:6d}   {share(sole_line, biting)} des mordus")
    print(f"  à cheval sur le saut         {straddling:6d}   {share(straddling, total)} du total")
    print(f"  crochets                     {brackets:6d}   "
          f"{share(brackets, brackets + parentheses)} des mentions")
    print(f"  sur deux lignes ou plus      {two_lines:6d}   {share(two_lines, total)} du total")
    if count:
        print(f"  longueur : médiane {lengths[count // 2]}, "
              f"neuvième décile {lengths[int(count * 0.9)]}, maximum {lengths[-1]}, "
              f"moyenne {sum(lengths) / count:.1f}")
    print()
    print("La proportion à retenir n'est pas la moyenne du corpus : un fichier qui")
    print("n'est pas sous-titré pour malentendants ne porte aucune mention, et il y")
    print("en a beaucoup. C'est l'étendue des fichiers qui en portent qui compte.")
    print()
    print("  part de mentions   sous-titres   fichier")
    for name, subtitles, file_biting in per_file:
        if file_biting:
            print(f"  {share(file_biting, subtitles):>16}   {subtitles:11d}   {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
