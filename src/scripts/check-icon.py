#!/usr/bin/env python3
"""Vérifie que l'icône est une image, et qu'elle se voit sur les deux fonds.

**C'était le seul fichier livré dont rien ne lisait le contenu.** Le `.desktop`
passe par `desktop-file-validate`, les métadonnées par `appstreamcli`, la page
de manuel est confrontée au binaire — l'icône, elle, était « validée par sa
présence ». Elle était pourtant cassée, et le défaut a été trouvé à l'œil, dans
un menu, après la fusion. Issue #260.

## Ce qu'il vérifie, et par quel chargeur

**Par gdk-pixbuf, celui que les bureaux GTK emploient**, et c'est le point. Le
fichier fautif s'ouvrait sans un mot dans Inkscape et dans Qt ; seul gdk-pixbuf
le refusait, parce que lui seul applique la règle qui était enfreinte. Un
contrôle qui aurait rendu la main à n'importe quel autre lecteur aurait déclaré
l'icône bonne.

    1. « <svg » commence dans les 256 premiers octets — la fenêtre dans laquelle
       gdk-pixbuf cherche de quoi reconnaître le format. Vérifié à part et
       d'abord, parce que c'est un défaut dont le message du chargeur ne dit
       rien d'utile : « couldn't recognize the image file format ».
    2. le fichier se charge à 16, 24, 48 et 128 pixels ;
    3. le rendu n'est pas vide : une part suffisante des pixels est opaque ;
    4. ce qui est dessiné se distingue d'un fond clair **et** d'un fond sombre.

Le quatrième est celui qui attrape l'autre moitié du défaut d'origine : un
cadre sombre sur fond transparent est une image parfaitement valide, et
parfaitement invisible sur un bureau sombre.

## Ce qu'il ne vérifie pas

**Que l'icône soit belle, ou qu'elle ressemble à quelque chose.** Aucun contrôle
ne sait dire cela ; c'est la relecture qui le dit, et c'est elle qui a trouvé ce
défaut-ci.
"""

from __future__ import annotations

import argparse
import pathlib
import sys

import gi

gi.require_version("GdkPixbuf", "2.0")
from gi.repository import GdkPixbuf  # noqa: E402

# La fenêtre dans laquelle gdk-pixbuf cherche la signature d'un SVG. Mesurée
# par dichotomie sur cette machine : « <svg » à l'octet 256 passe encore, un de
# plus et le fichier n'est plus reconnu.
SNIFF_WINDOW = 256

# Les tailles auxquelles une icône d'application est réellement vue : la barre
# des tâches, la liste des applications, la grille, la fiche de logithèque.
SIZES = (16, 24, 48, 128)

# Les deux fonds sur lesquels elle doit se voir — ceux des deux palettes que la
# fenêtre pose elle-même, voir gui/theme.cpp.
LIGHT = (239, 239, 239)
DARK = (30, 30, 30)

# Part minimale de pixels opaques. Une icône qui n'en couvre presque aucun est
# vide ou quasi vide ; celle du dépôt en couvre les trois quarts.
MINIMUM_INK = 0.20

# Part minimale des pixels dessinés qui doivent trancher sur le fond, et de
# combien. La distance est la somme des écarts par canal, sur 765.
MINIMUM_DISTANCE = 60
MINIMUM_VISIBLE = 0.50

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"


def sniffable(path: pathlib.Path) -> bool:
    """Dit si « <svg » tombe dans la fenêtre de reconnaissance."""
    head = path.read_bytes()[:SNIFF_WINDOW]
    return b"<svg" in head


def rendered(path: pathlib.Path, size: int) -> GdkPixbuf.Pixbuf:
    return GdkPixbuf.Pixbuf.new_from_file_at_size(str(path), size, size)


def pixels(image: GdkPixbuf.Pixbuf) -> list[tuple[int, int, int, int]]:
    """Les pixels, RVBA, lus dans le tampon du pixbuf."""
    data = image.get_pixels()
    stride = image.get_rowstride()
    channels = image.get_n_channels()
    out: list[tuple[int, int, int, int]] = []
    for y in range(image.get_height()):
        row = y * stride
        for x in range(image.get_width()):
            at = row + x * channels
            alpha = data[at + 3] if channels == 4 else 255
            out.append((data[at], data[at + 1], data[at + 2], alpha))
    return out


def ink(image: GdkPixbuf.Pixbuf) -> float:
    """La part des pixels qui ne sont pas transparents."""
    seen = pixels(image)
    return sum(1 for *_, alpha in seen if alpha > 32) / len(seen)


def visible_on(image: GdkPixbuf.Pixbuf, background: tuple[int, int, int]) -> float:
    """La part des pixels dessinés qui tranchent sur ce fond, une fois composés."""
    drawn = [p for p in pixels(image) if p[3] > 32]
    if not drawn:
        return 0.0

    far = 0
    for red, green, blue, alpha in drawn:
        # Composé sur le fond : c'est ce que l'œil voit, et un pixel à demi
        # transparent ne tranche qu'à moitié.
        share = alpha / 255
        composed = tuple(
            round(channel * share + background[index] * (1 - share))
            for index, channel in enumerate((red, green, blue))
        )
        if sum(abs(composed[i] - background[i]) for i in range(3)) >= MINIMUM_DISTANCE:
            far += 1

    return far / len(drawn)


def main() -> int:
    parser = argparse.ArgumentParser(description="Vérifie l'icône de l'application.")
    parser.add_argument("icon", type=pathlib.Path, help="le fichier SVG à vérifier")
    icon = parser.parse_args().icon

    failures: list[str] = []

    if not icon.is_file():
        print(f"{RED}✗{RESET} {icon} : absente")
        return 1

    if not sniffable(icon):
        failures.append(
            f"« <svg » n'est pas dans les {SNIFF_WINDOW} premiers octets : gdk-pixbuf ne\n"
            f"    reconnaîtra pas le format, et le bureau affichera une tuile vide.\n"
            f"    Un commentaire avant la balise racine suffit à produire ce défaut ;\n"
            f"    le mettre après la racine le corrige."
        )

    for size in SIZES:
        try:
            image = rendered(icon, size)
        except Exception as refused:  # noqa: BLE001 — le chargeur dit ce qu'il veut
            failures.append(f"{size} px : le chargeur refuse le fichier — {refused}")
            continue

        covered = ink(image)
        if covered < MINIMUM_INK:
            failures.append(
                f"{size} px : {covered:.0%} de pixels dessinés, il en faut {MINIMUM_INK:.0%} —"
                f" l'image est vide ou presque"
            )
            continue

        for name, background in (("clair", LIGHT), ("sombre", DARK)):
            seen = visible_on(image, background)
            if seen < MINIMUM_VISIBLE:
                failures.append(
                    f"{size} px, fond {name} : {seen:.0%} de ce qui est dessiné tranche sur le"
                    f" fond, il en faut {MINIMUM_VISIBLE:.0%} — l'icône s'y perd"
                )

    for line in failures:
        print(f"{RED}✗{RESET} {line}")

    if failures:
        print(f"\n{RED}✗{RESET} l'icône ne tient pas : {len(failures)} défaut(s)")
        return 1

    print(f"{GREEN}✓{RESET} l'icône se charge à {', '.join(str(s) for s in SIZES)} px,"
          f" et se voit sur les deux fonds")
    return 0


if __name__ == "__main__":
    sys.exit(main())
