#!/usr/bin/env bash
# The video fixtures of src/test/data/videos/: make them, and check that they
# declare what the project says they declare.
#
# Testing the reading of the frame rate and the duration requires a real
# container, and a container is unreadable in a diff. Nobody will reread these
# 3 KB. This script is what makes them **checkable rather than believed**: the
# command that makes them and the values expected from them live here, in a
# single place, and `--check` holds one against the other.
#
#   --check       (default) hold each fixture against the table below
#   --generate    remake the fixtures from the table
#   --weight      print the total weight, in bytes
#   --film PATH   make, **outside the repository**, the film-sized numbered video of
#                 the seek and step bench (#611), and check that it is honest
#
# **Two families.** The cadence fixtures show nothing: only the frame rate and
# the duration are read from them. The **numbered-frame** fixtures (#610) are the
# opposite: every frame carries its own number, eight 16-pixel bars, light or
# dark, read as bits, so that a test reads **the frame the player shows** and
# compares it with the one it should show — an oracle that does not rest on
# what the player says of itself. `--check` checks that they carry their number
# **without libmpv**, through ffmpeg: a test's oracle is only worth something if
# the fixture it reads is honest.
#
# **This script requires ffprobe, and the binary does not.** The distinction is
# that of the whole rest of the toolchain: `make check` already requires
# clang-tidy and gcovr of a development machine. `subedit` tolerates the absence
# of ffprobe and does without what it brings — that is a promise made to a user,
# not to whoever develops.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly FIXTURE_DIR="${REPO_ROOT}/src/test/data/videos"

# The format: MP4, not Matroska. Matroska rounds its timestamps to the
# millisecond and announces no stream duration — ffprobe deduces the frame rate
# from the timings there, which makes the number approximate. MP4 carries an
# explicit time scale and integer sample deltas: the frame rate comes out as the
# exact rational put in, and so does the duration.
#
# The codec: `mpeg4`, ffmpeg's own, not libx264. Two reasons, and neither is
# picture quality, which does not matter at all here — any build of ffmpeg can
# redo it, whereas a minimal build may lack libx264; and the file then carries no
# version string of a third-party library. Ours weighs 1.5 KB where libx264
# needed 2.6.
#
# 16×16 is the smallest picture aligned on a macroblock. Two seconds are enough:
# what is read from these files is a frame rate and a duration, never a picture.
readonly FIXTURE_WIDTH=16
readonly FIXTURE_HEIGHT=16
readonly FIXTURE_SECONDS=2

# name | declared frame rate | declared duration | maximum size allowed
#
# The two frame rates are not interchangeable: 25 is an integer, 24000/1001 is
# not, and it is the second that says something. An integer-rate fixture alone
# would suggest a right reading where the core has handled exact rationals
# since phase 1.
#
# The maximum size exists because a fixture that grows unseen is a silent debt:
# the threshold is generous — about twice the current weight — and its role is
# to catch an order of magnitude, not a byte.
readonly FIXTURES=(
    "cadence-25.mp4|25/1|2.000000|4096"
    "cadence-23-976.mp4|24000/1001|2.002000|4096"
)

# The numbered-frame fixtures. 128×64, eight 16-pixel bars: one byte of frame number,
# from 0 to 255, and ten seconds at 25 frames per second hold 250 of them.
#
# **A single keyframe, at the start**: it is the hard case — position 249 is decoded
# from 0 — and the one a real film has. The `mpeg4` encoder does not give a single one by
# default: despite `-g 250` it places a keyframe every 32 frames when the content changes
# on every frame (eight out of 250, measured), hence `-keyint_min`, `-sc_threshold` and
# `-bf 0` below.
#
# name | declared frame rate | frame count | keyframes | maximum size allowed
#
# The second is at 24000/1001: the thousandth ratio is not the frames', and a case at
# 23.976 says so — frame 10 there spans 417.08 to 458.79 ms, which a whole millisecond
# does not write.
readonly NUMBERED_WIDTH=128
readonly NUMBERED_HEIGHT=64
readonly NUMBERED_BAR=16
readonly NUMBERED_SECONDS=10
readonly NUMBERED=(
    "images-25.mp4|25/1|250|1|24576"
    "images-23-976.mp4|24000/1001|240|1|24576"
)

# The **audio-track** fixtures (#614). They show a black screen, like the cadence ones:
# what is read from them is the number of sound tracks, their language and their title,
# which the player hands to the `Audio` menu. One track, and two — the case where there is
# a choice. A video **without** a track is `cadence-25.mp4`, which never had one.
#
# The sound is an A at 440 Hz: the content does not matter, a track exists or not.
#
# **Matroska, not MP4** — the opposite of the others' choice: MP4 has no track title
# (ffmpeg writes `handler_name` there), and that is precisely what these fixtures must
# carry. Rounded timestamps do no harm here: no frame is read from them.
#
# name | tracks (language:title, comma-separated) | maximum size allowed
readonly AUDIO=(
    "audio-1.mkv|fra:Original|12288"
    "audio-2.mkv|fra:Original,eng:Commentary|20480"
)

# And **a film without pictures** (#614): sound alone, without a video track. It has no frame rate,
# so nothing to step through — the case where `stepFrames` does not know what to do and says so by doing nothing.
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
        || die "$1 is missing — ./src/scripts/setup-toolchain.sh installs it."
}

# The making command, written once. It is what the repository really tracks: the
# files are its output, reproducible, not the source.
#
# `-fflags +bitexact -flags:v +bitexact -map_metadata -1` removes from the output
# everything that would date the machine that produced it. What remains is
# reproduced byte for byte by the same version of ffmpeg — from one version to
# the next, the file may differ without what it declares changing, and that is
# what `--check` checks rather than the checksum.
generate_one() {
    local target="$1" rate="$2"
    ffmpeg -v error -y \
        -f lavfi -i "color=c=black:s=${FIXTURE_WIDTH}x${FIXTURE_HEIGHT}:r=${rate}:d=${FIXTURE_SECONDS}" \
        -c:v mpeg4 -qscale:v 31 -pix_fmt yuv420p \
        -fflags +bitexact -flags:v +bitexact -map_metadata -1 \
        "${target}"
}

# Every frame carries its number N: bar X is light (235) if bit X of N is 1, dark (16)
# otherwise, with neutral chroma. The `geq` filter computes that frame by frame.
generate_numbered_one() {
    local target="$1" rate="$2"
    generate_numbered "${target}" "${rate}" "${NUMBERED_WIDTH}" "${NUMBERED_HEIGHT}" "${NUMBERED_BAR}" \
        "${NUMBERED_SECONDS}"
}

# The making, at any size: the bar is an eighth of the width.
generate_numbered() {
    local target="$1" rate="$2" width="$3" height="$4" bar="$5" seconds="$6"
    ffmpeg -v error -y \
        -f lavfi -i "color=c=gray:s=${width}x${height}:r=${rate}:d=${seconds},geq=lum='if(gte(mod(floor(N/pow(2\,floor(X/${bar})))\,2)\,1)\,235\,16)':cb=128:cr=128" \
        -c:v mpeg4 -q:v 2 -g 250 -keyint_min 250 -sc_threshold 1000000000 -bf 0 -pix_fmt yuv420p -an \
        -fflags +bitexact -flags:v +bitexact -map_metadata -1 \
        "${target}"
}

# One track per entry of `tracks`, each with its language and title. The same A at 440 Hz
# everywhere: what tells the tracks apart is their label, which the player shows as is.
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

# The number carried by frame `frame` of a file, read **by ffmpeg**: the frame decoded in
# greyscale, then the eight bars, in the middle of the middle row.
number_of() {
    local file="$1" frame="$2"
    number_in "${file}" "${frame}" "${NUMBERED_WIDTH}" "${NUMBERED_HEIGHT}" "${NUMBERED_BAR}"
}

number_in() {
    local file="$1" frame="$2" width="$3" height="$4" bar="$5" raw bit value number=0
    raw="$(mktemp)"
    ffmpeg -v error -y -i "${file}" -vf "select=eq(n\,${frame})" -frames:v 1 \
        -f rawvideo -pix_fmt gray "${raw}" 2>/dev/null || true
    for bit in 0 1 2 3 4 5 6 7; do
        value="$(od -An -tu1 -j $(( (height / 2) * width + bit * bar + bar / 2 )) -N1 "${raw}" 2>/dev/null | tr -d ' ')"
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
    info "making the video fixtures"
    mkdir -p "${FIXTURE_DIR}"
    local entry name rate
    for entry in "${FIXTURES[@]}"; do
        IFS='|' read -r name rate _ _ <<<"${entry}"
        generate_one "${FIXTURE_DIR}/${name}" "${rate}"
        ok "${name} — ${rate}, ${FIXTURE_SECONDS} s, $(stat -c %s "${FIXTURE_DIR}/${name}") bytes"
    done
    for entry in "${NUMBERED[@]}"; do
        IFS='|' read -r name rate _ _ _ <<<"${entry}"
        generate_numbered_one "${FIXTURE_DIR}/${name}" "${rate}"
        ok "${name} — ${rate}, numbered frames, $(stat -c %s "${FIXTURE_DIR}/${name}") bytes"
    done
    local tracks
    for entry in "${AUDIO[@]}"; do
        IFS='|' read -r name tracks _ <<<"${entry}"
        generate_audio_one "${FIXTURE_DIR}/${name}" "${tracks}"
        ok "${name} — tracks ${tracks}, $(stat -c %s "${FIXTURE_DIR}/${name}") bytes"
    done
    for entry in "${SOUND_ONLY[@]}"; do
        IFS='|' read -r name _ <<<"${entry}"
        generate_sound_only_one "${FIXTURE_DIR}/${name}"
        ok "${name} — sound alone, $(stat -c %s "${FIXTURE_DIR}/${name}") bytes"
    done
}

check_numbered() {
    local entry name rate frames keyframes maximum path size actual before last
    for entry in "${NUMBERED[@]}"; do
        IFS='|' read -r name rate frames keyframes maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — missing; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        actual="$(probe "${path}" stream=r_frame_rate || true)"
        [[ "${actual}" == "${rate}" ]] \
            || ko "${name} — frame rate ${actual:-unreadable}, expected ${rate}"

        actual="$(probe "${path}" stream=nb_frames || true)"
        [[ "${actual}" == "${frames}" ]] \
            || ko "${name} — ${actual:-unreadable} frames, expected ${frames}"

        actual="$(probe "${path}" stream=width,height | tr '\n' 'x' | sed 's/x$//' || true)"
        [[ "${actual}" == "${NUMBERED_WIDTH}x${NUMBERED_HEIGHT}" ]] \
            || ko "${name} — size ${actual:-unreadable}, expected ${NUMBERED_WIDTH}x${NUMBERED_HEIGHT}"

        # The number of keyframes: **one** — that is what makes the last frame a seek across
        # the whole duration. Counted on the packets, which carry the `K` flag.
        actual="$(ffprobe -v error -select_streams v:0 -show_entries packet=flags \
            -of csv=p=0 "${path}" 2>/dev/null | grep -c K || true)"
        [[ "${actual}" == "${keyframes}" ]] \
            || ko "${name} — ${actual} keyframe(s), expected ${keyframes}"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) \
            || ko "${name} — ${size} bytes, maximum ${maximum}"

        # **Honest**: the first, one in the middle and the last carry their number, read
        # by ffmpeg and not by the player whose tests use these files.
        last=$(( frames - 1 ))
        for frame in 0 1 $(( frames / 2 )) "${last}"; do
            actual="$(number_of "${path}" "${frame}")"
            [[ "${actual}" == "${frame}" ]] \
                || ko "${name} — frame ${frame} carries the number ${actual}"
        done

        [[ "${failures}" == "${before}" ]] \
            && ok "${name} — ${rate}, ${frames} frames, ${keyframes} keyframe, ${size} bytes"
    done
}

check_audio() {
    local entry name tracks maximum path size actual expected before
    for entry in "${AUDIO[@]}"; do
        IFS='|' read -r name tracks maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — missing; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        # Language and title of each sound track, in order, in the table's format.
        actual="$(ffprobe -v error -select_streams a \
            -show_entries stream_tags=language,title -of csv=p=0 "${path}" 2>/dev/null \
            | awk -F, '{ printf "%s%s:%s", (NR > 1 ? "," : ""), $1, $2 }' || true)"
        expected="${tracks}"
        [[ "${actual}" == "${expected}" ]] \
            || ko "${name} — tracks ${actual:-unreadable}, expected ${expected}"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) \
            || ko "${name} — ${size} bytes, maximum ${maximum}"

        [[ "${failures}" == "${before}" ]] \
            && ok "${name} — tracks ${tracks}, ${size} bytes"
    done
}

check_sound_only() {
    local entry name maximum path size video before
    for entry in "${SOUND_ONLY[@]}"; do
        IFS='|' read -r name maximum <<<"${entry}"
        path="${FIXTURE_DIR}/${name}"
        before="${failures}"

        if [[ ! -f "${path}" ]]; then
            ko "${name} — missing; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        video="$(ffprobe -v error -select_streams v -show_entries stream=index \
            -of csv=p=0 "${path}" 2>/dev/null | wc -l || true)"
        [[ "${video}" == "0" ]] || ko "${name} — ${video} video track(s), expected none"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) || ko "${name} — ${size} bytes, maximum ${maximum}"

        [[ "${failures}" == "${before}" ]] && ok "${name} — sound alone, ${size} bytes"
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
            ko "${name} — missing; ./src/scripts/video-fixtures.sh --generate"
            continue
        fi

        # `|| true` because an unreadable file makes ffprobe exit in error,
        # and an unreadable fixture is a difference to report like the
        # others — not a reason to stop the check on a shell trace.
        actual="$(probe "${path}" stream=r_frame_rate || true)"
        [[ "${actual}" == "${rate}" ]] \
            || ko "${name} — frame rate ${actual:-unreadable}, expected ${rate}"

        actual="$(ffprobe -v error -show_entries format=duration \
            -of default=noprint_wrappers=1:nokey=1 "${path}" 2>/dev/null || true)"
        [[ "${actual}" == "${duration}" ]] \
            || ko "${name} — duration ${actual:-unreadable}, expected ${duration}"

        size="$(stat -c %s "${path}")"
        (( size <= maximum )) \
            || ko "${name} — ${size} bytes, maximum ${maximum}"

        # Each fixture gives its own verdict: a second one that differs must
        # not silence the first, which is fine.
        [[ "${failures}" == "${before}" ]] \
            && ok "${name} — ${rate}, ${duration} s, ${size} bytes"
    done

    check_numbered
    check_audio
    check_sound_only

    (( failures == 0 )) || die "${failures} difference(s) between the fixtures and the table."
    printf '  total weight: %s bytes\n' "$(weight)"
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

# The video of the seek and step bench (#611): 1280×720, 250 frames at 25 per second, **a
# single keyframe** — a film's, where the last frame is decoded from the first.
#
# **Outside the repository, and that is the whole point**: the tracked 13 KB fixture only gives
# costs of a few milliseconds, which cannot be compared with anything, and a film-sized video
# has no place in a repository. `make bench` makes it in the build tree and hands it to the
# bench; without ffmpeg, the bench says so and abstains.
#
# 720p, the size the phase's initialisation measured (#608): enough for decoding a frame to
# cost tens of milliseconds, and little enough for the bench to fit in a minute or two.
# `mpeg4`, like the others: any build of ffmpeg has it, which is not the case of libx264.
# **A real film (H.264, HEVC) may cost something else**, and saying so is the journal's
# business, not this making's.
readonly FILM_WIDTH=1280
readonly FILM_HEIGHT=720
readonly FILM_BAR=160
readonly FILM_SECONDS=10
readonly FILM_RATE="25/1"
readonly FILM_FRAMES=250

film() {
    require ffmpeg
    require ffprobe
    local target="${1:?usage: $(basename "$0") --film PATH}"

    mkdir -p "$(dirname "${target}")"
    # Already made and honest: it is not remade, `make bench` asks for it every time.
    if [[ ! -f "${target}" ]] || ! film_is_honest "${target}"; then
        generate_numbered "${target}" "${FILM_RATE}" "${FILM_WIDTH}" "${FILM_HEIGHT}" "${FILM_BAR}" \
            "${FILM_SECONDS}"
    fi

    film_is_honest "${target}" || die "$(basename "${target}") — the video made is not the one expected"
    ok "$(basename "${target}") — ${FILM_WIDTH}x${FILM_HEIGHT}, ${FILM_FRAMES} frames, 1 keyframe, $(stat -c %s "${target}") bytes"
}

# Its size, its frame rate, its frame count, **a single keyframe**, and that the first, one in the
# middle and the last carry their number — read by ffmpeg, as for the tracked fixtures.
film_is_honest() {
    local file="$1" actual frame keyframes
    actual="$(probe "${file}" stream=width,height | tr '\n' 'x' | sed 's/x$//' || true)"
    [[ "${actual}" == "${FILM_WIDTH}x${FILM_HEIGHT}" ]] || return 1
    [[ "$(probe "${file}" stream=r_frame_rate || true)" == "${FILM_RATE}" ]] || return 1
    [[ "$(probe "${file}" stream=nb_frames || true)" == "${FILM_FRAMES}" ]] || return 1
    keyframes="$(ffprobe -v error -select_streams v:0 -show_entries packet=flags \
        -of csv=p=0 "${file}" 2>/dev/null | grep -c K || true)"
    [[ "${keyframes}" == "1" ]] || return 1
    for frame in 0 $(( FILM_FRAMES / 2 )) $(( FILM_FRAMES - 1 )); do
        [[ "$(number_in "${file}" "${frame}" "${FILM_WIDTH}" "${FILM_HEIGHT}" "${FILM_BAR}")" == "${frame}" ]] \
            || return 1
    done
}

case "${1:---check}" in
    --check) check ;;
    --generate) generate ;;
    --weight) weight ;;
    --film) film "${2:-}" ;;
    *) die "usage: $(basename "$0") [--check|--generate|--weight|--film PATH]" ;;
esac
