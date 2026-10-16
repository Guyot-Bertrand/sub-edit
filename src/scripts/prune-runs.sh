#!/usr/bin/env bash
# Prunes the Actions runs accumulated on the repository.
#
# **The rule, in one sentence:** of each workflow, keep the thirty most recent
# runs, plus the most recent one whose branch is `main` if it is not already
# among them; delete the rest; never touch a run that has not finished.
#
# Each of the three parts answers something precise.
#
# **Thirty per workflow, not a duration.** GitHub can set a duration — the
# *Artifact and log retention* setting — but it only erases logs and artifacts:
# the run's row stays in the list, with its steps empty. And a duration cleans
# nothing when the activity is concentrated on the past week, which was the case
# when this was written: seven days would have left 103 of the 146 runs, thirty
# per workflow leave 60.
#
# **The most recent on `main`.** That is the full Monday analysis, the weekly
# net of `ci.yml`. Its period is seven days and a dozen runs arrive each day:
# without an exception it would fall out of the thirty within three days, and
# the repository would lose track of the one run whose absence means something.
# "The most recent" is singular, and deliberately so — in the plural, the clause
# would resurrect the forty-seven runs of the `push` trigger on `main` removed
# in #106, which this pruning exists precisely to erase.
#
# **Unfinished runs.** GitHub refuses to delete them; asking would make the pass
# fail over a pointless race.
#
# ## Why a workflow, and not this script run by hand
#
# A personal token cannot delete a run: the API answers
# `403 Resource not accessible by personal access token`. Only a workflow's
# `GITHUB_TOKEN` can, with `permissions: actions: write`. The deletion therefore
# lives in `.github/workflows/elagage.yml`, which calls this script.
#
# *Reading*, on the other hand, works with a personal token. That is what makes
# `--dry-run` useful from a development machine, and what makes the selection
# provable: `--input` gives it a hand-written list instead of having it fetch
# one, and `verify-gates.sh` uses it to prove that the five cases that decide
# this file are handled as announced.
#
# ## Usage
#
#   prune-runs.sh [--dry-run] [--input FILE]
#
#   --dry-run          writes the identifiers to delete, deletes nothing
#   --input FILE       reads the list from a JSONL file — one object per
#                      line, as produced by
#                      `gh api … --paginate --jq '.workflow_runs[]'` — instead
#                      of querying the API
#
# Without `--input`, the repository is that of `GITHUB_REPOSITORY` — which
# Actions sets — or, failing that, that of the current git repository.

set -euo pipefail

# The number of runs kept per workflow. A single number, written once: making it
# settable by an option would invite setting it differently here and there, and
# the rule would no longer fit in one sentence.
readonly KEEP=30

readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

usage() {
    cat <<'EOF'
prune-runs.sh [--dry-run] [--input FILE]

Prunes the Actions runs: of each workflow, keeps the thirty most recent, plus
the most recent on `main`, and deletes the rest. Unfinished runs are never
touched.

  --dry-run          writes the identifiers to delete, deletes nothing
  --input FILE       reads the list from a JSONL file instead of the API
  -h, --help         this help

Deleting requires a workflow's GITHUB_TOKEN: a personal token gets 403 on
Actions. From a development machine, only --dry-run works.
EOF
}

dry_run=0
input=""

while (( $# > 0 )); do
    case "$1" in
        --dry-run) dry_run=1; shift ;;
        --input)
            [[ $# -ge 2 ]] || { printf '--input expects a file\n' >&2; exit 2; }
            input="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'unknown option: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done

require() {
    command -v "$1" >/dev/null 2>&1 || {
        printf '%s is missing: ./src/scripts/setup-toolchain.sh installs it\n' "$1" >&2
        exit 1
    }
}

require jq

repository="${GITHUB_REPOSITORY:-}"

# The target repository. Actions sets GITHUB_REPOSITORY; locally it has to be
# asked of git. Resolved apart from the reading, because the deletion needs it
# even when the list comes from a file.
#
# Written as an `if` rather than `[[ … ]] && return`: under `set -e`, the latter
# makes the function return 1 when the test fails, and kills the script in the
# most common case — the one where the repository is already known.
resolve_repository() {
    if [[ -z "${repository}" ]]; then
        require gh
        repository="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"
    fi
}

# The list, as one JSON object per line.
fetch_runs() {
    if [[ -n "${input}" ]]; then
        [[ -f "${input}" ]] || { printf 'file not found: %s\n' "${input}" >&2; exit 1; }
        cat "${input}"
        return
    fi

    resolve_repository
    gh api "repos/${repository}/actions/runs" --paginate \
        --jq '.workflow_runs[]
              | {id, workflow_id, name, head_branch, status, created_at}'
}

# The selection. A function of the list, and nothing else: no clock, no network,
# no state — that is what makes it provable on a hand-written set.
#
# `index(…)` returns `null` when the identifier is absent, and an integer
# otherwise — including zero, which is true in jq. The negation therefore
# behaves for the first element, which a `> 0` would have missed.
select_doomed() {
    jq -rs --argjson keep "${KEEP}" '
        group_by(.workflow_id)
        | map(
            (sort_by(.created_at) | reverse) as $runs
            | ($runs[0:$keep] | map(.id))          as $recent
            | ([$runs[] | select(.head_branch == "main")][0].id) as $latest_main
            | $runs
            | map(. as $run
                  | select($run.status == "completed"
                           and ($recent | index($run.id) | not)
                           and ($run.id != $latest_main)))
          )
        | flatten
        | .[].id
    '
}

runs="$(fetch_runs)"
total="$(printf '%s' "${runs}" | grep -c '' || true)"
doomed="$(printf '%s\n' "${runs}" | select_doomed)"
doomed_count="$(printf '%s' "${doomed}" | grep -c '' || true)"
kept=$(( total - doomed_count ))

if (( dry_run == 1 )); then
    [[ -z "${doomed}" ]] || printf '%s\n' "${doomed}"
    printf '%s%d runs, %d to delete, %d kept%s\n' \
        "${BOLD}" "${total}" "${doomed_count}" "${kept}" "${RESET}" >&2
    exit 0
fi

require gh
resolve_repository

deleted=0
refused=0
while read -r id; do
    [[ -n "${id}" ]] || continue
    # `</dev/null`: without it, a call that read standard input would swallow
    # the rest of the list and the loop would stop after one turn.
    if gh api -X DELETE "repos/${repository}/actions/runs/${id}" --silent \
        </dev/null 2>/dev/null; then
        deleted=$(( deleted + 1 ))
    else
        # A run can disappear between the reading and the deletion, or have
        # become undeletable. That does not justify giving up on the following
        # ones: the next pass will pick them up anyway.
        printf 'refused: %s\n' "${id}" >&2
        refused=$(( refused + 1 ))
    fi
done <<< "${doomed}"

# What really remains, not what was meant to be kept: a refused deletion leaves
# its run in place.
printf '%s%d deleted, %d refused, %d remaining%s\n' \
    "${BOLD}" "${deleted}" "${refused}" "$(( total - deleted ))" "${RESET}"

# The measure asked for by #123, redone on each pass rather than taken once: a
# figure frozen in a document goes stale, this one does not.
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    {
        printf '## Pruning of the runs\n\n'
        printf '| | |\n| :--- | ---: |\n'
        printf '| before | %d |\n' "${total}"
        printf '| deleted | %d |\n' "${deleted}"
        printf '| refused | %d |\n' "${refused}"
        printf '| after | %d |\n' "$(( total - deleted ))"
        printf '\nRule: the %d most recent of each workflow, ' "${KEEP}"
        printf 'plus the most recent on `main`.\n'
    } >> "${GITHUB_STEP_SUMMARY}"
fi
