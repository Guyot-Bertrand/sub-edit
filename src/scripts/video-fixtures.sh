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
# **Deux familles.** Les fixtures de cadence ne montrent rien : on n'y lit que
# la fréquence et la durée. Les fixtures **à images numérotées** (#610) sont
# l'inverse : chaque image y porte son propre numéro, huit barres de 16 pixels
# claires ou sombres lues comme des bits, de sorte qu'un test lit **l'image que
# le lecteur affiche** et la compare à celle qu'il devait afficher — un oracle qui
# ne repose pas sur ce que le lecteur dit de lui-même. `--check` vérifie qu'elles
# portent bien leur numéro **sans libmpv**, par ffmpeg : l'oracle d'un test ne vaut
# que si la fixture qu'il lit est honnête.
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

# Les fixtures à images numérotées. 128×64, huit barres de 16 pixels : un octet de
# numéro d'image, de 0 à 255, et dix secondes à 25 images par seconde en tiennent 250.
#
# **Une seule image-clé, au début** : c'est le cas difficile — la position 249 se décode
# depuis la 0 — et celui qu'un film réel connaît. L'encodeur `mpeg4` n'en donne pas
# l'unique par défaut : malgré `-g 250` il place une image-clé toutes les 32 images quand
# le contenu change à chaque image (huit sur 250, mesuré), d'où `-keyint_min`,
# `-sc_threshold` et `-bf 0` ci-dessous.
#
# nom | fréquence déclarée | nombre d'images | images-clés | taille maximale admise
#
# La seconde est à 24000/1001 : le rapport du millième n'est pas celui des images, et un
# cas à 23,976 le dit — l'image 10 y occupe 417,08 à 458,79 ms, que la milliseconde
# entière n'écrit pas.
readonly NUMBERED_WIDTH=128
readonly NUMBERED_HEIGHT=64
readonly NUMBERED_BAR=16
readonly NUMBERED_SECONDS=10
readonly NUMBERED=(
    "images-25.mp4|25/1|250|1|24576"
    "images-23-976.mp4|24000/1001|240|1|24576"
)

# Les fixtures **à pistes audio** (#614). Elles montrent un écran noir, comme celles de
# cadence : ce qu'on y lit, c'est le nombre de pistes sonores, leur langue et leur titre,
# que le lecteur rend au menu `Audio`. Une seule piste, et deux — le cas où il y a un choix.
# Une vidéo **sans** piste est `cadence-25.mp4`, qui n'en a jamais eu.
#
# Le son est un la à 440 Hz : le contenu n'importe pas, une piste existe ou non.
#
# **Matroska, et non MP4** — l'inverse du choix des autres : le MP4 n'a pas de titre de piste
# (ffmpeg y écrit `handler_name`), et c'est précisément ce que ces fixtures doivent porter.
# Ici les horodatages arrondis ne gênent pas : on n'y lit aucune image.
#
# nom | pistes (langue:titre, séparées par des virgules) | taille maximale admise
readonly AUDIO=(
    "audio-1.mkv|fra:Original|12288"
    "audio-2.mkv|fra:Original,eng:Commentary|20480"
)

# Et **un film sans image** (#614) : du son seul, sans piste vidéo. Il n'a pas de fréquence d'image,
# donc rien à avancer d'un pas — le cas où `stepFrames` ne sait pas quoi faire et le dit en ne faisant rien.
readonly SOUND_ONLY=("sound-only.mkv|12288")

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

# Chaque image porte son numéro N : la barre X est claire (235) si le bit X de N vaut 1,
# sombre (16) sinon, la chrominance neutre. Le filtre `geq` calcule cela image par image.
generate_numbered_one() {
    local target="$1" rate="$2"
    ffmpeg -v error -y \
        -f lavfi -i "color=c=gray:s=${NUMBERED_WIDTH}x${NUMBERED_HEIGHT}:r=${rate}:d=${NUMBERED_SECONDS},geq=lum='if(gte(mod(floor(N/pow(2\,floor(X/${NUMBERED_BAR})))\,2)\,1)\,235\,16)':cb=128:cr=128" \
        -c:v mpeg4 -q:v 2 -g 250 -keyint_min 250 -sc_threshold 1000000000 -bf 0 -pix_fmt yuv420p -an \
        -fflags +bitexact -flags:v +bitexact -map_metadata -1 \
        "${target}"
}

# Une piste par entrée de `tracks`, chacune avec sa langue et son titre. Le même la à 440 Hz
# partout : ce qui distingue les pistes est leur étiquette, que le lecteur rend telle quelle.
generate_sound_only_one() {
    local target="$1"
    ffmpeg -v error -y \
        -f lavfi -i "sine=frequency=440:duration=${FIXTURE_SECONDS}" \
        -c:a aac -b:a 16k -ac 1 -ar 8000 \
        -fflags +bitexact -flags:a +bitexact -map_metadata -1 \
        "${target}"
}

generate_audio_one() {
    local target="$1" tracks="$2" track language title index=0
    local -a inputs=() maps=() tags=()
    IFS=',' read -r -a list <<<"${tracks}"
    for track in "${list[@]}"; do
        language="${track%%:*}"
        title="${track#*:}"
        inputs+=(-f lavfi -i "sine=frequency=440:duration=${FIXTURE_SECONDS}")
        maps+=(-map "$((index + 1)):a")
        tags+=("-metadata:s:a:${index}" "language=${language}" "-metadata:s:a:${index}" "title=${title}")
        index=$((index + 1))
    done
    ffmpeg -v error -y \
        -f lavfi -i "color=c=black:s=${FIXTURE_WIDTH}x${FIXTURE_HEIGHT}:r=25:d=${FIXTURE_SECONDS}" \
        "${inputs[@]}" -map 0:v "${maps[@]}" \
        -c:v mpeg4 -qscale:v 31 -pix_fmt yuv420p -c:a aac -b:a 16k -ac 1 -ar 8000 \
        "${tags[@]}" \
        -fflags +bitexact -flags:v +bitexact -flags:a +bitexact -map_metadata -1 \
        "${target}"
}

# Le numéro que porte l'image `frame` d'un fichier, lu **par ffmpeg** : l'image décodée en
# niveaux de gris, puis les huit barres, au milieu de la rangée du milieu.
number_of() {
    local file="$1" frame="$2" raw bit value number=0
    raw="$(mktemp)"
    ffmpeg -v error -y -i "${file}" -vf "select=eq(n\,${frame})" -frames:v 1 \
        -f rawvideo -pix_fmt gray "${raw}" 2>/dev/null || true
    for bit in 0 1 2 3 4 5 6 7; do
        value="$(od -An -tu1 -j $(( (NUMBERED_HEIGHT / 2) * NUMBERED_WIDTH + bit * NUMBERED_BAR + NUMBERED_BAR / 2 )) -N1 "${raw}" 2>/dev/null | tr -d ' ')"
        (( ${value:-0} > 128 )) && number=$(( number + (1 << bit) ))
    done
    rm -f "${raw}"
    printf '%s\n' "${number}"
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
    for entry in "${NUMBERED[@]}"; do
        IFS='|' read -r name rate _ _ _ <<<"${entry}"
        generate_numbered_one "${FIXTURE_DIR}/${name}" "${rate}"
        ok "${name} — ${rate}, images numérotées, $(stat -c %s "${FIXTURE_DIR}/${name}") octets"
    done
    local tracks
    for entry in "${AUDIO[@]}"; do
        IFS='|' read -r name tracks _ <<<"${entry}"
        generate_audio_one "${FIXTURE_DIR}/${name}" "${tracks}"
        ok "${name} — pistes ${tracks}, $(stat -c %s "${FIXTURE_DIR}/${name}") octets"
    done
    for entry in "${SOUND_ONLY[@]}"; do
        IFS='|' read -r name _ <<<"${entry}"
        generate_sound_only_one "${FIXTURE_DIR}/${name}"
        ok "${name} — du son seul, $(stat -c %s "${FIXTURE_DIR}/${name}") octets"
    done
}

check_numbered() {
    local entry name rate frames keyframes maximum path size actual before last
    for entry in "${NUMBERED[@]}"; do
        IFS='|' read -r name rate frames keyframes maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — absente ; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        actual="$(probe "${path}" stream=r_frame_rate || true)"
        [[ "${actual}" == "${rate}" ]] \
            || ko "${name} — fréquence ${actual:-illisible}, attendue ${rate}"

        actual="$(probe "${path}" stream=nb_frames || true)"
        [[ "${actual}" == "${frames}" ]] \
            || ko "${name} — ${actual:-illisible} images, attendues ${frames}"

        actual="$(probe "${path}" stream=width,height | tr '\n' 'x' | sed 's/x$//' || true)"
        [[ "${actual}" == "${NUMBERED_WIDTH}x${NUMBERED_HEIGHT}" ]] \
            || ko "${name} — taille ${actual:-illisible}, attendue ${NUMBERED_WIDTH}x${NUMBERED_HEIGHT}"

        # Le nombre d'images-clés : **une** — c'est ce qui fait de la dernière image un saut
        # de toute la durée. Compté sur les paquets, qui portent le drapeau `K`.
        actual="$(ffprobe -v error -select_streams v:0 -show_entries packet=flags \
            -of csv=p=0 "${path}" 2>/dev/null | grep -c K || true)"
        [[ "${actual}" == "${keyframes}" ]] \
            || ko "${name} — ${actual} image(s)-clé, attendues ${keyframes}"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) \
            || ko "${name} — ${size} octets, maximum ${maximum}"

        # **Honnêtes** : la première, une du milieu et la dernière portent leur numéro, lu
        # par ffmpeg et non par le lecteur dont un test se sert de ces fichiers.
        last=$(( frames - 1 ))
        for frame in 0 1 $(( frames / 2 )) "${last}"; do
            actual="$(number_of "${path}" "${frame}")"
            [[ "${actual}" == "${frame}" ]] \
                || ko "${name} — l'image ${frame} porte le numéro ${actual}"
        done

        [[ "${failures}" == "${before}" ]] \
            && ok "${name} — ${rate}, ${frames} images, ${keyframes} image-clé, ${size} octets"
    done
}

check_audio() {
    local entry name tracks maximum path size actual expected before
    for entry in "${AUDIO[@]}"; do
        IFS='|' read -r name tracks maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — absente ; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        # Langue et titre de chaque piste sonore, dans l'ordre, au format de la table.
        actual="$(ffprobe -v error -select_streams a \
            -show_entries stream_tags=language,title -of csv=p=0 "${path}" 2>/dev/null \
            | awk -F, '{ printf "%s%s:%s", (NR > 1 ? "," : ""), $1, $2 }' || true)"
        expected="${tracks}"
        [[ "${actual}" == "${expected}" ]] \
            || ko "${name} — pistes ${actual:-illisibles}, attendues ${expected}"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) \
            || ko "${name} — ${size} octets, maximum ${maximum}"

        [[ "${failures}" == "${before}" ]] \
            && ok "${name} — pistes ${tracks}, ${size} octets"
    done
}

check_sound_only() {
    local entry name maximum path size video before
    for entry in "${SOUND_ONLY[@]}"; do
        IFS='|' read -r name maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — absente ; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        video="$(ffprobe -v error -select_streams v -show_entries stream=index \
            -of csv=p=0 "${path}" 2>/dev/null | wc -l || true)"
        [[ "${video}" == "0" ]] || ko "${name} — ${video} piste(s) vidéo, attendues aucune"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) || ko "${name} — ${size} octets, maximum ${maximum}"

        [[ "${failures}" == "${before}" ]] && ok "${name} — du son seul, ${size} octets"
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

    check_numbered
    check_audio
    check_sound_only

    (( failures == 0 )) || die "${failures} écart(s) entre les fixtures et la table."
    printf '  poids total : %s octets\n' "$(weight)"
}

weight() {
    local entry name total=0
    for entry in "${FIXTURES[@]}" "${NUMBERED[@]}" "${AUDIO[@]}" "${SOUND_ONLY[@]}"; do
        IFS='|' read -r name _ <<<"${entry}"
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
