#!/usr/bin/env bash
# Les fixtures vidéo de src/test/data/videos/ : les fabriquer, et vérifier
# qu'elles déclarent bien ce que le projet dit qu'elles déclarent.
#
# Éprouver la lecture de la fréquence d'image et de la durée demande un vrai
# conteneur, et un conteneur est illisible dans un diff. Personne ne relira ces
# 3 Ko. Ce script est ce qui les rend **vérifiables plutôt que crus** : la
# commande qui les fabrique et les valeurs qu'on en attend vivent ici, à un seul
# endroit, et `--check` confronte l'une à l'autre.
#
#   --check       (défaut) confronte chaque fixture à la table ci-dessous
#   --generate    refabrique les fixtures depuis la table
#   --weight      écrit le poids total, en octets
#
# **Ce script exige ffprobe, et le binaire non.** La distinction est celle de
# tout le reste de la chaîne d'outils : `make check` exige déjà clang-tidy et
# gcovr d'une machine de développement. `subedit`, lui, tolère l'absence de
# ffprobe et se passe de ce qu'il apporte — c'est une promesse faite à un
# utilisateur, pas à qui développe.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly FIXTURE_DIR="${REPO_ROOT}/src/test/data/videos"

# Le format : MP4, et non Matroska. Matroska arrondit ses horodatages à la
# milliseconde et n'annonce aucune durée de flux — ffprobe y déduit la fréquence
# des timings, ce qui rend le nombre approché. MP4 porte une échelle de temps
# explicite et des écarts d'échantillon entiers : la fréquence en ressort comme
# le rationnel exact qu'on y a mis, et la durée aussi.
#
# Le codec : `mpeg4`, celui d'ffmpeg lui-même, et non libx264. Deux raisons, et
# aucune n'est la qualité d'image, qui n'a ici aucune importance — n'importe
# quelle construction d'ffmpeg sait le refaire, alors qu'une construction
# minimale peut être privée de libx264 ; et le fichier ne porte alors aucune
# chaîne de version d'une bibliothèque tierce. Le nôtre pèse 1,5 Ko là où
# libx264 en demandait 2,6.
#
# 16×16 est la plus petite image alignée sur un macrobloc. Deux secondes
# suffisent : ce qu'on lit dans ces fichiers, c'est une fréquence et une durée,
# jamais une image.
readonly FIXTURE_WIDTH=16
readonly FIXTURE_HEIGHT=16
readonly FIXTURE_SECONDS=2

# nom | fréquence déclarée | durée déclarée | taille maximale admise
#
# Les deux fréquences ne sont pas interchangeables : 25 est entière, 24000/1001
# ne l'est pas, et c'est la seconde qui dit quelque chose. Une fixture à
# fréquence entière seule laisserait croire à une lecture juste là où le noyau
# manipule des rationnels exacts depuis la phase 1.
#
# La taille maximale existe parce qu'une fixture qui grossit sans qu'on le voie
# est une dette silencieuse : le seuil est large — environ le double du poids
# actuel — et son rôle est d'attraper un ordre de grandeur, pas un octet.
readonly FIXTURES=(
    "cadence-25.mp4|25/1|2.000000|4096"
    "cadence-23-976.mp4|24000/1001|2.002000|4096"
)

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

failures=0

info() { printf '%s%s%s\n' "${BOLD}" "$*" "${RESET}"; }
ok() { printf '  %s✓%s %s\n' "${GREEN}" "${RESET}" "$*"; }
ko() {
    printf '  %s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

die() {
    printf '%s%s%s\n' "${RED}" "$*" "${RESET}" >&2
    exit 1
}

require() {
    command -v "$1" >/dev/null 2>&1 \
        || die "$1 est absent — ./src/scripts/setup-toolchain.sh l'installe."
}

# La commande de fabrication, écrite une fois. C'est elle que le dépôt versionne
# vraiment : les fichiers en sont la sortie, refaisable, et non la source.
#
# `-fflags +bitexact -flags:v +bitexact -map_metadata -1` retire de la sortie
# tout ce qui daterait la machine qui l'a produite. Ce qui reste est reproduit à
# l'octet près par une même version d'ffmpeg — d'une version à l'autre, le
# fichier peut différer sans que ce qu'il déclare change, et c'est bien ce que
# `--check` vérifie plutôt que la somme de contrôle.
generate_one() {
    local target="$1" rate="$2"
    ffmpeg -v error -y \
        -f lavfi -i "color=c=black:s=${FIXTURE_WIDTH}x${FIXTURE_HEIGHT}:r=${rate}:d=${FIXTURE_SECONDS}" \
        -c:v mpeg4 -qscale:v 31 -pix_fmt yuv420p \
        -fflags +bitexact -flags:v +bitexact -map_metadata -1 \
        "${target}"
}

probe() {
    local file="$1" entries="$2"
    ffprobe -v error -select_streams v:0 -show_entries "${entries}" \
        -of default=noprint_wrappers=1:nokey=1 "${file}" 2>/dev/null
}

generate() {
    require ffmpeg
    info "fabrication des fixtures vidéo"
    mkdir -p "${FIXTURE_DIR}"
    local entry name rate
    for entry in "${FIXTURES[@]}"; do
        IFS='|' read -r name rate _ _ <<<"${entry}"
        generate_one "${FIXTURE_DIR}/${name}" "${rate}"
        ok "${name} — ${rate}, ${FIXTURE_SECONDS} s, $(stat -c %s "${FIXTURE_DIR}/${name}") octets"
    done
}

check() {
    require ffprobe
    local entry name rate duration maximum path size actual before
    for entry in "${FIXTURES[@]}"; do
        IFS='|' read -r name rate duration maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — absente ; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        # `|| true` parce qu'un fichier illisible fait sortir ffprobe en
        # erreur, et qu'une fixture illisible est un écart à rapporter comme
        # les autres — pas une raison d'interrompre le contrôle sur une trace
        # de shell.
        actual="$(probe "${path}" stream=r_frame_rate || true)"
        [[ "${actual}" == "${rate}" ]] \
            || ko "${name} — fréquence ${actual:-illisible}, attendue ${rate}"

        actual="$(ffprobe -v error -show_entries format=duration \
            -of default=noprint_wrappers=1:nokey=1 "${path}" 2>/dev/null || true)"
        [[ "${actual}" == "${duration}" ]] \
            || ko "${name} — durée ${actual:-illisible}, attendue ${duration}"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) \
            || ko "${name} — ${size} octets, maximum ${maximum}"

        # Chaque fixture rend son propre verdict : une seconde en écart ne doit
        # pas faire taire la première, qui va bien.
        [[ "${failures}" == "${before}" ]] \
            && ok "${name} — ${rate}, ${duration} s, ${size} octets"
    done

    (( failures == 0 )) || die "${failures} écart(s) entre les fixtures et la table."
    printf '  poids total : %s octets\n' "$(weight)"
}

weight() {
    local entry name total=0
    for entry in "${FIXTURES[@]}"; do
        IFS='|' read -r name _ _ _ <<<"${entry}"
        [[ -f "${FIXTURE_DIR}/${name}" ]] || continue
        total=$((total + $(stat -c %s "${FIXTURE_DIR}/${name}")))
    done
    printf '%s\n' "${total}"
}

case "${1:---check}" in
    --check) check ;;
    --generate) generate ;;
    --weight) weight ;;
    *) die "usage : $(basename "$0") [--check|--generate|--weight]" ;;
esac
