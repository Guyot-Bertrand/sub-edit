#!/usr/bin/env python3
"""Promeut ou efface les captures d'écran fraîchement engendrées.

**Pourquoi ce détour plutôt qu'écrire la référence directement.** Une capture
engendrée à chaque `make manual` et versionnée telle quelle produirait un
binaire modifié à chaque exécution, sur chaque machine : le journal du dépôt se
remplirait d'images que personne n'a changées, et un vrai changement d'interface
s'y noierait. Le programme de capture n'écrit donc jamais la référence — il
écrit `<nom>.new.png`, et c'est ici qu'on décide.

    l'image a bougé   →  le `.new` REMPLACE la référence, git voit une
                         modification, et elle veut dire quelque chose
    l'image n'a pas   →  le `.new` est EFFACÉ, et git ne voit rien

La forme est celle d'un projet voisin qui la tient depuis longtemps ; ce qui
change ici, c'est qu'il n'y a ni Node ni bibliothèque d'images. Le décodage PNG
est écrit à la main, parce que le format que Qt écrit est le sous-ensemble le
plus simple qui soit — huit bits par canal, sans entrelacement — et parce
qu'ajouter numpy ou Pillow à la chaîne d'outils pour compter des pixels
différents serait payer cher une soustraction.

**Le seuil, et pourquoi il y en a deux.** Un seuil unique sur le nombre de
pixels différents ne distingue pas une image entièrement redessinée d'un
antialiasing qui a bougé d'un rien sur tout le texte. On sépare donc la
*sévérité* — de combien un pixel diffère — de la *surface* — combien de pixels
diffèrent à ce point. C'est ce qui laisse passer le bruit d'encodage sans
laisser passer une ligne de texte changée.

Modes :

    (défaut)   promeut ou efface
    --check    ne promeut rien, efface tout, et échoue si une image a bougé
    --dry-run  ne touche à rien, et réussit quoi qu'il arrive

`--check` efface aussi ce qu'il a examiné, y compris les images qui ont bougé :
un contrôle qui laisserait derrière lui les `.new.png` qu'il vient de refuser
transformerait chaque échec en fichiers non suivis à ramasser à la main, et
`check-untracked.sh` les verrait apparaître à la porte suivante.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import sys
import zlib

# Sévérité : de combien un canal doit différer pour que le pixel compte. Huit
# sur deux cent cinquante-cinq, soit trois pour cent — au-dessus de ce qu'un
# ré-encodage déplace, très en dessous d'un changement de couleur voulu.
DEFAULT_PIXEL_THRESHOLD = 8
# Surface : la proportion de pixels ainsi différents au-delà de laquelle l'image
# a changé.
#
# **Un dix-millième, et le chiffre d'avant était mille fois trop généreux.** Il
# valait un millième — mille pixels sur 1280×800 — au motif que c'était « moins
# qu'un mot ». Mesuré : la mention `Encoding: UTF-8, no BOM` ajoutée à la barre
# d'état de la fenêtre, vingt-trois caractères, en allume **830**. Le seuil
# déclarait donc « inchangée » une image dont une ligne entière avait changé, et
# la référence gardée par le manuel montrait une fenêtre qui n'existait plus.
# Issue #313, où le défaut s'est vu parce qu'il tombait juste sous la barre.
#
# Un caractère de cette police coûte une trentaine de pixels ; cent trois
# pixels, c'est trois caractères. En dessous, il n'y a plus rien de voulu à
# voir.
#
# **Ce n'est pas ce seuil-ci qui filtre l'antialiasing**, et c'est ce qui permet
# de le baisser : la sévérité s'en charge, et les dix-sept autres captures de ce
# dépôt rendent 0,000 % d'une exécution à l'autre. Le bruit qu'on redoutait
# n'existe pas sur une machine, et entre deux machines c'est la police nommée
# qui l'empêche — ADR 0024.
DEFAULT_RATIO_THRESHOLD = 0.0001

GREEN = "\033[32m"
RED = "\033[31m"
BOLD = "\033[1m"
RESET = "\033[0m"

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
# Les deux seuls types de couleur que Qt écrit pour une capture : RVB et RVBA.
CHANNELS_OF_COLOUR_TYPE = {2: 3, 6: 4}


class Unreadable(Exception):
    """Un PNG que ce décodeur ne sait pas lire, dit plutôt que deviné."""


def read_png_size(path: pathlib.Path) -> tuple[int, int]:
    """Largeur et hauteur, lues dans le seul en-tête.

    L'IHDR est le premier bloc et il fait vingt-cinq octets : rien de plus n'est
    lu, là où `read_png` décompresse l'image entière.
    """
    with path.open("rb") as handle:
        header = handle.read(len(PNG_SIGNATURE) + 16)
    if not header.startswith(PNG_SIGNATURE):
        raise Unreadable(f"{path} : ce n'est pas un PNG")
    return struct.unpack(">II", header[len(PNG_SIGNATURE) + 8 : len(PNG_SIGNATURE) + 16])


def read_png(path: pathlib.Path) -> tuple[int, int, int, bytes]:
    """Rend (largeur, hauteur, canaux, pixels), les pixels non filtrés.

    Volontairement étroit. Un décodeur qui accepterait l'entrelacement et les
    palettes serait trois fois plus long et n'aurait jamais rien à décoder :
    ce qu'il lit vient d'un seul écrivain, `QPixmap::save`.
    """
    data = path.read_bytes()
    if not data.startswith(PNG_SIGNATURE):
        raise Unreadable(f"{path} : ce n'est pas un PNG")

    width = height = channels = 0
    compressed = bytearray()
    offset = len(PNG_SIGNATURE)

    while offset < len(data):
        (length,) = struct.unpack(">I", data[offset : offset + 4])
        kind = data[offset + 4 : offset + 8]
        payload = data[offset + 8 : offset + 8 + length]
        offset += 12 + length  # longueur, type, données, CRC

        if kind == b"IHDR":
            width, height, depth, colour, compression, filtering, interlace = struct.unpack(
                ">IIBBBBB", payload
            )
            if depth != 8:
                raise Unreadable(f"{path} : {depth} bits par canal, 8 attendus")
            if colour not in CHANNELS_OF_COLOUR_TYPE:
                raise Unreadable(f"{path} : type de couleur {colour}, 2 ou 6 attendus")
            if compression != 0 or filtering != 0 or interlace != 0:
                raise Unreadable(f"{path} : compression, filtrage ou entrelacement inattendu")
            channels = CHANNELS_OF_COLOUR_TYPE[colour]
        elif kind == b"IDAT":
            compressed += payload
        elif kind == b"IEND":
            break

    if width == 0 or height == 0:
        raise Unreadable(f"{path} : en-tête absent ou image vide")

    return width, height, channels, unfilter(zlib.decompress(bytes(compressed)), width, height, channels)


def unfilter(raw: bytes, width: int, height: int, channels: int) -> bytes:
    """Défait les cinq filtres du PNG, ligne par ligne.

    Chaque ligne est préfixée du numéro de son filtre, et chaque filtre se
    défait en fonction du pixel de gauche, de celui du dessus, ou des deux. La
    ligne précédente est déjà défiltrée quand on arrive à la suivante, ce qui
    est toute l'astuce du format.
    """
    stride = width * channels
    out = bytearray(stride * height)
    previous = bytearray(stride)

    for row in range(height):
        start = row * (stride + 1)
        kind = raw[start]
        line = bytearray(raw[start + 1 : start + 1 + stride])

        if kind == 0:
            pass  # aucune prédiction : la ligne est déjà ce qu'elle vaut
        elif kind == 1:  # Sub — le pixel de gauche
            for i in range(channels, stride):
                line[i] = (line[i] + line[i - channels]) & 0xFF
        elif kind == 2:  # Up — le pixel du dessus
            for i in range(stride):
                line[i] = (line[i] + previous[i]) & 0xFF
        elif kind == 3:  # Average — la moyenne des deux
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                line[i] = (line[i] + ((left + previous[i]) >> 1)) & 0xFF
        elif kind == 4:  # Paeth — celui des trois qui prédit le mieux
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                upleft = previous[i - channels] if i >= channels else 0
                line[i] = (line[i] + paeth(left, previous[i], upleft)) & 0xFF
        else:
            raise Unreadable(f"filtre de ligne {kind} inconnu")

        out[row * stride : (row + 1) * stride] = line
        previous = line

    return bytes(out)


def paeth(left: int, up: int, upleft: int) -> int:
    estimate = left + up - upleft
    to_left = abs(estimate - left)
    to_up = abs(estimate - up)
    to_upleft = abs(estimate - upleft)
    if to_left <= to_up and to_left <= to_upleft:
        return left
    if to_up <= to_upleft:
        return up
    return upleft


def difference(new: pathlib.Path, reference: pathlib.Path, severity: int) -> tuple[int, int]:
    """Rend (pixels différents, pixels au total), ou (-1, 0) si les tailles diffèrent.

    **Deux fichiers identiques ne sont pas décodés.** C'est le cas courant, et de
    loin : les captures sont engendrées sous des réglages fixés, donc deux
    exécutions d'un même code rendent les mêmes octets. Lire cinquante kilo-octets
    coûte une milliseconde, là où décoder puis comparer six images en coûte
    quelques milliers — et cette comparaison-là est enchaînée par
    `make check-local`.
    """
    if new.read_bytes() == reference.read_bytes():
        width, height = read_png_size(new)
        return 0, width * height

    new_width, new_height, new_channels, new_pixels = read_png(new)
    old_width, old_height, old_channels, old_pixels = read_png(reference)

    if (new_width, new_height, new_channels) != (old_width, old_height, old_channels):
        return -1, 0

    total = new_width * new_height

    # Des octets différents peuvent porter les mêmes pixels — un encodeur qui a
    # changé de version, par exemple. La boucle ne sert alors à rien.
    if new_pixels == old_pixels:
        return 0, total

    differing = 0
    for pixel in range(total):
        base = pixel * new_channels
        for channel in range(new_channels):
            if abs(new_pixels[base + channel] - old_pixels[base + channel]) > severity:
                differing += 1
                break

    return differing, total


def verdict(differing: int, total: int, surface: float) -> str:
    if differing < 0:
        return "modifiée"
    return "modifiée" if total > 0 and differing / total > surface else "inchangée"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dir", required=True, help="le répertoire des captures")
    parser.add_argument("--pixel-threshold", type=int, default=DEFAULT_PIXEL_THRESHOLD)
    parser.add_argument("--ratio-threshold", type=float, default=DEFAULT_RATIO_THRESHOLD)
    parser.add_argument(
        "--check", action="store_true", help="échoue si une image a bougé, sans rien promouvoir"
    )
    parser.add_argument("--dry-run", action="store_true", help="ne touche à rien")
    options = parser.parse_args()

    directory = pathlib.Path(options.dir)
    if not directory.is_dir():
        print(f"{RED}✗ {directory} : répertoire absent{RESET}", file=sys.stderr)
        return 1

    fresh = sorted(directory.glob("*.new.png"))
    if not fresh:
        print(f"{RED}✗ {directory} : aucune capture fraîche à comparer{RESET}", file=sys.stderr)
        print("  engendrer d'abord, avec « make screenshots »", file=sys.stderr)
        return 1

    writes = not (options.check or options.dry_run)
    changed = []

    for new in fresh:
        reference = new.with_name(new.name.removesuffix(".new.png") + ".png")

        if not reference.exists():
            if writes:
                new.rename(reference)
            changed.append(reference.name)
            print(f"  {GREEN}+{RESET} {reference.name} — nouvelle")
            continue

        differing, total = difference(new, reference, options.pixel_threshold)
        said = verdict(differing, total, options.ratio_threshold)

        if said == "modifiée":
            if writes:
                new.rename(reference)
            changed.append(reference.name)
            share = "dimensions" if differing < 0 else f"{100 * differing / total:.3f} %"
            print(f"  {GREEN}~{RESET} {reference.name} — modifiée ({share})")
        else:
            if writes:
                new.unlink()
            print(f"  {reference.name} — inchangée ({100 * differing / total:.3f} %)")

    if options.check:
        # Rien n'est promu, et rien ne reste : voir l'en-tête.
        for new in fresh:
            new.unlink(missing_ok=True)

    if options.check and changed:
        print(
            f"{RED}✗ {len(changed)} capture(s) ne correspondent plus à la fenêtre "
            f"— lancer « make screenshots »{RESET}",
            file=sys.stderr,
        )
        return 1

    if changed:
        print(f"{GREEN}{len(changed)} capture(s) promue(s){RESET}")
    else:
        print(f"{GREEN}aucune capture n'a bougé{RESET}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
