#!/usr/bin/env bash
# The whisper.cpp samples of src/test/data/whisper/: capture them, and check that
# they are still what the tool writes.
#
# The adapter of phase 22 reads what `whisper-cli` writes. A fixture written from
# the documentation proves that the adapter reads what we believe the tool
# writes; these are **what the tool wrote**, captured once from a real run, and
# the command, the version and the model live here and in the README next to
# them, so that anybody can replay the capture.
#
#   --capture   run the tool and write the samples into src/test/data/whisper/
#   --check     run the tool and compare its output with the versioned samples
#
# **Neither runs in the gate.** The tool and a model weigh far more than the
# repository is willing to carry, and nothing is shipped. Both are given on the
# command line, and the script says what it used:
#
#   whisper-fixtures.sh --capture --whisper PATH --model PATH --sample PATH
#
#   --whisper  the `whisper-cli` executable
#   --model    a multilingual model, `ggml-tiny.bin` for the versioned samples
#   --sample   `samples/jfk.wav` of whisper.cpp: eleven seconds of speech, public
#              domain, which is why it can be used without being versioned here
#
# Two things the tool writes are the machine's and not the tool's, and are
# replaced by a fixed text before anything is compared or written: the path of
# the model, and the line that lists the CPU features.
#
# `ffmpeg` is needed to make the long input (the sample, eight times over) that
# shows the progress lines: one is printed per decoded 30-second window, and the
# sample alone fits in one.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly TARGET="${REPO_ROOT}/src/test/data/whisper"

mode=""
whisper=""
model=""
sample=""

usage() {
    sed -n '2,/^set -euo/p' "${BASH_SOURCE[0]}" | sed '$d' | sed 's/^# \{0,1\}//' >&2
    exit 2
}

while (($# > 0)); do
    case "$1" in
    --capture | --check) mode="${1#--}" ;;
    --whisper) whisper="${2:?--whisper needs a path}" && shift ;;
    --model) model="${2:?--model needs a path}" && shift ;;
    --sample) sample="${2:?--sample needs a path}" && shift ;;
    *) usage ;;
    esac
    shift
done

[[ -n "${mode}" && -n "${whisper}" && -n "${model}" && -n "${sample}" ]] || usage
for tool in "${whisper}" "${model}" "${sample}"; do
    [[ -e "${tool}" ]] || {
        printf 'not found: %s\n' "${tool}" >&2
        exit 1
    }
done
command -v ffmpeg >/dev/null || {
    printf 'ffmpeg is needed to make the long input\n' >&2
    exit 1
}

work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT
out="${work}/out"
mkdir -p "${out}"

# Replaces what belongs to the machine in a JSON file, keeps everything else.
sanitize_json() {
    python3 - "$1" <<'PY'
import json, sys
path = sys.argv[1]
with open(path, encoding="utf-8") as handle:
    document = json.load(handle)
document["systeminfo"] = "WHISPER : (machine-specific, replaced by whisper-fixtures.sh)"
document["params"]["model"] = "ggml-tiny.bin"
with open(path, "w", encoding="utf-8") as handle:
    json.dump(document, handle, indent="\t", ensure_ascii=False)
    handle.write("\n")
PY
}

run_json() { # name, extra options…
    local name="$1"
    shift
    "${whisper}" -m "${model}" -f "${sample}" -l auto "$@" -of "${work}/${name}" >/dev/null 2>&1
    sanitize_json "${work}/${name}.json"
    mv "${work}/${name}.json" "${out}/${name}.json"
}

run_json jfk -oj
run_json jfk.full -oj -ojf
run_json jfk.words -oj -ml 1 -sow

# The long input, and the two streams of a run that prints its progress.
long="${work}/long.wav"
ffmpeg -v error -y -i "${sample}" -i "${sample}" -i "${sample}" -i "${sample}" \
    -i "${sample}" -i "${sample}" -i "${sample}" -i "${sample}" \
    -filter_complex "[0][1][2][3][4][5][6][7]concat=n=8:v=0:a=1" "${long}"
"${whisper}" -m "${model}" -f "${long}" -l auto -pp -of "${work}/long" \
    >"${work}/long.stdout" 2>"${work}/long.stderr"
grep -E 'auto-detected language|progress =' "${work}/long.stderr" | sed 's/ (p = [0-9.]*)//' \
    >"${out}/progress.stderr.txt"
cp "${work}/long.stdout" "${out}/segments.stdout.txt"

# What it says when it fails: the exit code and the error line, nothing else.
failure() { # name, arguments…
    local name="$1" code=0
    shift
    "${whisper}" "$@" >/dev/null 2>"${work}/${name}.stderr" || code=$?
    {
        printf 'exit code %d\n' "${code}"
        grep -E '^error:' "${work}/${name}.stderr" | sed "s#'[^']*/not-audio.txt'#'not-audio.txt'#"
    } >"${out}/${name}.txt"
}

printf 'not audio\n' >"${work}/not-audio.txt"
failure missing-model -m /nonexistent.bin -f "${sample}"
failure missing-input -m "${model}" -f /nonexistent.wav
failure not-audio -m "${model}" -f "${work}/not-audio.txt"

version="$("${whisper}" --version 2>&1 | head -1 || true)"
printf 'captured with %s\n' "${version}"

case "${mode}" in
capture)
    mkdir -p "${TARGET}"
    cp "${out}"/* "${TARGET}/"
    printf 'wrote %d samples into %s\n' "$(find "${out}" -type f | wc -l)" "${TARGET}"
    ;;
check)
    differs=0
    for sample_file in "${out}"/*; do
        name="$(basename "${sample_file}")"
        if ! diff -q "${sample_file}" "${TARGET}/${name}" >/dev/null 2>&1; then
            printf 'differs: %s\n' "${name}" >&2
            differs=1
        fi
    done
    ((differs == 0)) || exit 1
    printf 'the samples are what the tool writes\n'
    ;;
esac
