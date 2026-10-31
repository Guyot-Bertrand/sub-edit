#!/usr/bin/env bash
# `sync` on a real film, by hand — the run of phase 23 that the gate cannot make (#815).
#
# The alignment is proven in the gate on versioned cases and a fake engine. What those cannot say
# is how it behaves on a real voice, with a real model, over minutes of sound. This script says it:
#
#   1. it transcribes the first minutes of the film with a model of its own (the "truth"): the text
#      is what an engine writes, and the times are where it heard it;
#   2. it makes from that file four files at a known fault — late by a constant, a drift that grows
#      along the film, the second half late, and a line that nobody says;
#   3. it runs `sync` on each, with another model, and measures how far each start lands from the truth.
#
# **What it proves, and what it does not.** The truth comes from an engine, not from a person:
# the figures say whether the matching finds its way back from a known fault with the words a
# second model heard, not whether the engine's times are those of the speech. The text being the
# engine's own, it is also cleaner than a subtitle written by a person; a real subtitle file is a
# harder case, and is judged by eye on the report this script writes.
#
# **It runs by hand, never in the gate**: a model weighs far more than the repository carries, and
# a run takes minutes. When something it needs is missing it says SKIPPED, names what, and exits 77:
# a skipped run is never reported as a passed one.
#
#   alignment-by-hand.sh --cli PATH --film FILE --model PATH --truth-model PATH --vad-model PATH
#                        [--end 5:00] [--language CODE] [--keep DIR] [--control]
#
#   --cli          the `subedit-cli` executable
#   --film         a video or an audio file with speech in it; nothing of it is kept
#   --model        the model `sync` listens with (small enough to be the everyday one)
#   --truth-model  the model that writes the reference (a larger one is better)
#   --vad-model    a voice-activity model, for both
#   --end          how much of the film, from its beginning (default 5:00)
#   --language     the language of the speech (default: detected)
#   --keep         a directory to leave the files in; a temporary one, removed, otherwise
#   --control      one run more, with the model that wrote the truth: the same words are heard, so
#                  what remains is the matching alone (slow: the larger model listens again)
#
# `ffmpeg` and `whisper-cli` are looked for on the PATH, as the program looks for them.

set -euo pipefail

skip() {
    echo "SKIPPED: $1" >&2
    exit 77
}

cli="" film="" model="" truth_model="" vad="" end="5:00" language="" keep="" control=0
while [[ $# -gt 0 ]]; do
    case "$1" in
    --cli) cli="$2" ;;
    --film) film="$2" ;;
    --model) model="$2" ;;
    --truth-model) truth_model="$2" ;;
    --vad-model) vad="$2" ;;
    --end) end="$2" ;;
    --language) language="$2" ;;
    --keep) keep="$2" ;;
    --control)
        control=1
        shift
        continue
        ;;
    *)
        echo "unknown option: $1" >&2
        exit 2
        ;;
    esac
    shift 2
done

[[ -x "$cli" ]] || skip "--cli does not name an executable"
for pair in "film:$film" "model:$model" "truth-model:$truth_model" "vad-model:$vad"; do
    [[ -f "${pair#*:}" ]] || skip "--${pair%%:*} does not name a file"
done
command -v ffmpeg >/dev/null || skip "ffmpeg is not on the PATH"
command -v whisper-cli >/dev/null || skip "whisper-cli is not on the PATH"

work="${keep:-$(mktemp -d)}"
mkdir -p "$work"
[[ -n "$keep" ]] || trap 'rm -rf "$work"' EXIT

lang=()
[[ -z "$language" ]] || lang=(--language "$language")

echo "Truth: the first $end of the film, transcribed with $(basename "$truth_model")"
"$cli" transcribe "$film" --end "$end" --model "$truth_model" --vad-model "$vad" "${lang[@]}" \
    --output "$work/truth.srt" 2>/dev/null

python3 -I - "$work" "$end" <<'PY'
import re, sys
work, end = sys.argv[1], sys.argv[2]
def seconds(text):
    parts = [float(p) for p in text.split(":")]
    total = 0.0
    for p in parts:
        total = total * 60 + p
    return total
span = seconds(end)
stamp = re.compile(r"(\d+):(\d+):(\d+),(\d+) --> (\d+):(\d+):(\d+),(\d+)")
def ms(h, m, s, f): return ((int(h) * 60 + int(m)) * 60 + int(s)) * 1000 + int(f)
def clock(v):
    v = max(0, int(v)); h, v = divmod(v, 3600000); m, v = divmod(v, 60000); s, f = divmod(v, 1000)
    return f"{h:02d}:{m:02d}:{s:02d},{f:03d}"
lines = []
for block in open(f"{work}/truth.srt", encoding="utf-8").read().strip().split("\n\n"):
    rows = block.split("\n")
    m = stamp.match(rows[1])
    lines.append((ms(*m.groups()[:4]), ms(*m.groups()[4:]), "\n".join(rows[2:])))
def write(name, entries):
    with open(f"{work}/{name}.srt", "w", encoding="utf-8") as out:
        for number, (a, b, text) in enumerate(entries, 1):
            out.write(f"{number}\n{clock(a)} --> {clock(b)}\n{text}\n\n")
write("late", [(a + 2400, b + 2400, t) for a, b, t in lines])
write("drift", [(a + 3000 * a / (span * 1000), b + 3000 * a / (span * 1000), t) for a, b, t in lines])
half = lines[len(lines) // 2][0]
write("half", [(a + (2000 if a >= half else 0), b + (2000 if a >= half else 0), t) for a, b, t in lines])
middle = len(lines) // 2
extra = list(lines)
gap = (lines[middle - 1][1] + lines[middle][0]) // 2
extra.insert(middle, (gap, gap + 1500, "Subtitles by an unknown hand."))
write("extra", extra)
print(f"{len(lines)} subtitles in the truth")
PY

# The audio is listened to ten seconds past the span, so that a late file still finds its last words.
limit="$(python3 -I -c '
import sys
total = 0.0
for part in sys.argv[1].split(":"):
    total = total * 60 + float(part)
print(int(total) + 10)' "$end")"

cases=(late drift half extra)
if [[ "$control" == 1 ]]; then
    cp "$work/late.srt" "$work/control.srt"
    cases+=(control)
fi
for case in "${cases[@]}"; do
    started=$(date +%s)
    listening="$model"
    [[ "$case" != control ]] || listening="$truth_model"
    "$cli" --format json sync "$work/$case.srt" --video "$film" --end "$limit" --model "$listening" \
        --vad-model "$vad" "${lang[@]}" --report "$work/$case.report.txt" \
        --output "$work/$case.synced.srt" >"$work/$case.json" 2>/dev/null || true
    echo "$(( $(date +%s) - started ))" >"$work/$case.seconds"
done

python3 -I - "$work" <<'PY'
import json, os, re, statistics, sys
work = sys.argv[1]
stamp = re.compile(r"(\d+):(\d+):(\d+),(\d+) --> ")
def starts(path):
    out = []
    for block in open(path, encoding="utf-8").read().strip().split("\n\n"):
        rows = block.split("\n")
        h, m, s, f = stamp.match(rows[1]).groups()
        out.append(((int(h) * 60 + int(m)) * 60 + int(s)) * 1000 + int(f))
    return out
truth = starts(f"{work}/truth.srt")
print(f"\n{'case':<7}{'lines':>6}{'median':>9}{'p95':>8}{'worst':>8}{'<=500ms':>9}{'not found':>11}{'flagged':>9}{'time':>7}")
for case in ("late", "drift", "half", "extra", "control"):
    if case == "control" and not os.path.exists(f"{work}/control.synced.srt"):
        continue
    try:
        before = starts(f"{work}/{case}.srt")
        after = starts(f"{work}/{case}.synced.srt")
        record = json.loads(open(f"{work}/{case}.json").read().splitlines()[-1])
    except (OSError, ValueError, IndexError):
        print(f"{case:<7}  the run did not produce a file")
        continue
    skip = len(truth) // 2 if case == "extra" else None
    pairs = [(a, truth[i - (1 if skip is not None and i > skip else 0)])
             for i, a in enumerate(after) if i != skip]
    errors = sorted(abs(a - t) for a, t in pairs)
    counts = record.get("counts", {})
    flagged = len(record.get("flagged", []))
    extra_kept = ""
    if skip is not None:
        extra_kept = "  (the line nobody says " + ("kept its time)" if after[skip] == before[skip] else "MOVED)")
    seconds = int(open(f"{work}/{case}.seconds").read())
    p95 = errors[int(len(errors) * 0.95) - 1] if errors else 0
    within = sum(1 for e in errors if e <= 500)
    print(f"{case:<7}{len(errors):>6}{statistics.median(errors):>8.0f}ms{p95:>6}ms{errors[-1]:>6}ms"
          f"{within:>9}{counts.get('not_found', 0):>11}{flagged:>9}{seconds:>6}s{extra_kept}")
print("\nThe files are kept only with --keep.")
PY
