#!/usr/bin/env python3
"""Checks that the icon is an image, and that it shows on both backgrounds.

**It was the only shipped file whose content nothing read.** The `.desktop`
goes through `desktop-file-validate`, the metadata through `appstreamcli`, the
manual page is checked against the binary — the icon was "validated by its
presence". Yet it was broken, and the defect was found by eye, in a menu, after
the merge. Issue #260.

## What it checks, and through which loader

**Through gdk-pixbuf, the one GTK desktops use**, and that is the point. The
faulty file opened without a word in Inkscape and in Qt; only gdk-pixbuf
refused it, because it alone applies the rule that was broken. A check that
handed over to any other reader would have declared the icon good.

    1. "<svg" starts within the first 256 bytes — the window in which
       gdk-pixbuf looks for something to recognise the format by. Checked
       separately and first, because it is a defect about which the loader's
       message says nothing useful: "couldn't recognize the image file format".
    2. the file loads at 16, 24, 48 and 128 pixels;
    3. the rendering is not empty: a large enough share of the pixels is opaque;
    4. what is drawn stands out from a light background **and** from a dark one.

The fourth is the one that catches the other half of the original defect: a dark
frame on a transparent background is a perfectly valid image, and perfectly
invisible on a dark desktop.

## What it does not check

**That the icon is beautiful, or that it looks like anything.** No check can
tell that; review does, and it was review that found this defect.
"""

from __future__ import annotations

import argparse
import pathlib
import sys

import gi

gi.require_version("GdkPixbuf", "2.0")
from gi.repository import GdkPixbuf  # noqa: E402

# The window in which gdk-pixbuf looks for the signature of an SVG. Measured by
# bisection on this machine: "<svg" at byte 256 still passes, one more and the
# file is no longer recognised.
SNIFF_WINDOW = 256

# The sizes at which an application icon is actually seen: the taskbar, the
# application list, the grid, the software centre page.
SIZES = (16, 24, 48, 128)

# The two backgrounds on which it must show — those of the two palettes the
# window sets itself, see gui/theme.cpp.
LIGHT = (239, 239, 239)
DARK = (30, 30, 30)

# Minimum share of opaque pixels. An icon that covers almost none is empty or
# nearly so; the repository's covers three quarters.
MINIMUM_INK = 0.20

# Minimum share of the drawn pixels that must stand out from the background,
# and by how much. The distance is the sum of the per-channel differences, out
# of 765.
MINIMUM_DISTANCE = 60
MINIMUM_VISIBLE = 0.50

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"


def sniffable(path: pathlib.Path) -> bool:
    """Tells whether "<svg" falls within the recognition window."""
    head = path.read_bytes()[:SNIFF_WINDOW]
    return b"<svg" in head


def rendered(path: pathlib.Path, size: int) -> GdkPixbuf.Pixbuf:
    return GdkPixbuf.Pixbuf.new_from_file_at_size(str(path), size, size)


def pixels(image: GdkPixbuf.Pixbuf) -> list[tuple[int, int, int, int]]:
    """The pixels, RGBA, read from the pixbuf's buffer."""
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
    """The share of the pixels that are not transparent."""
    seen = pixels(image)
    return sum(1 for *_, alpha in seen if alpha > 32) / len(seen)


def visible_on(image: GdkPixbuf.Pixbuf, background: tuple[int, int, int]) -> float:
    """The share of the drawn pixels that stand out from this background, once composited."""
    drawn = [p for p in pixels(image) if p[3] > 32]
    if not drawn:
        return 0.0

    far = 0
    for red, green, blue, alpha in drawn:
        # Composited on the background: that is what the eye sees, and a
        # half-transparent pixel stands out only by half.
        share = alpha / 255
        composed = tuple(
            round(channel * share + background[index] * (1 - share))
            for index, channel in enumerate((red, green, blue))
        )
        if sum(abs(composed[i] - background[i]) for i in range(3)) >= MINIMUM_DISTANCE:
            far += 1

    return far / len(drawn)


def main() -> int:
    parser = argparse.ArgumentParser(description="Checks the application icon.")
    parser.add_argument("icon", type=pathlib.Path, help="the SVG file to check")
    icon = parser.parse_args().icon

    failures: list[str] = []

    if not icon.is_file():
        print(f"{RED}✗{RESET} {icon}: missing")
        return 1

    if not sniffable(icon):
        failures.append(
            f"\"<svg\" is not within the first {SNIFF_WINDOW} bytes: gdk-pixbuf will not\n"
            f"    recognise the format, and the desktop will show an empty tile.\n"
            f"    A comment before the root element is enough to cause this defect;\n"
            f"    moving it after the root fixes it."
        )

    for size in SIZES:
        try:
            image = rendered(icon, size)
        except Exception as refused:  # noqa: BLE001 — the loader says what it likes
            failures.append(f"{size} px: the loader refuses the file — {refused}")
            continue

        covered = ink(image)
        if covered < MINIMUM_INK:
            failures.append(
                f"{size} px: {covered:.0%} of the pixels drawn, {MINIMUM_INK:.0%} needed —"
                f" the image is empty or nearly so"
            )
            continue

        for name, background in (("light", LIGHT), ("dark", DARK)):
            seen = visible_on(image, background)
            if seen < MINIMUM_VISIBLE:
                failures.append(
                    f"{size} px, {name} background: {seen:.0%} of what is drawn stands out from"
                    f" it, {MINIMUM_VISIBLE:.0%} needed — the icon gets lost on it"
                )

    for line in failures:
        print(f"{RED}✗{RESET} {line}")

    if failures:
        print(f"\n{RED}✗{RESET} the icon does not hold: {len(failures)} defect(s)")
        return 1

    print(f"{GREEN}✓{RESET} the icon loads at {', '.join(str(s) for s in SIZES)} px,"
          f" and shows on both backgrounds")
    return 0


if __name__ == "__main__":
    sys.exit(main())
