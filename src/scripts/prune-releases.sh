#!/usr/bin/env bash
# Prunes the patch releases of past milestones — issue #232.
#
# **The rule, in one sentence:** keep every `vX.Y.0` release, keep the patch
# releases of the current milestone, delete those of past milestones — and never
# touch a tag.
#
# "The current milestone" is the highest `X.Y` pair among the releases present.
# While we are at `0.10.*`, every `0.10.Z` stays; the day `v0.11.0` is
# published, the releases `0.10.1` to `0.10.N` go with their packages, and
# `v0.10.0` stays.
#
# **Only the release goes, never the tag.** A tag costs nothing, and it keeps
# the possibility of rebuilding any version from the history: `gh release
# delete` without `--cleanup-tag` removes the release and its files, and leaves
# the tag where it is.
#
# **A consequence worth knowing.** A fix published on a past milestone — a
# `v0.9.1` pushed after `v0.10.0` — is built, published, then pruned straight
# away. The rule keeps patches of the current milestone only, and it makes no
# exception for the most recent one.
#
# **Except by hand.** Rebuilding an old patch through `workflow_dispatch` is a
# deliberate act — a package to redo — and pruning it straight away would undo
# what was just asked. The script notes the event (`GITHUB_EVENT_NAME`, which
# Actions sets by itself), says that it does not prune, and exits with 0 without
# having fetched or deleted anything. The guard is here rather than in an `if:`
# of the workflow for the reason of the next section: a YAML condition cannot be
# replayed. It does not apply to `--dry-run`, which deletes nothing.
#
# **Two other things the rule did not say.** Drafts are set aside before
# anything else: `gh api …/releases` returns them too, and a `v0.12.0` draft
# written by hand would push milestone 0.11 into the past — its patches would go
# for a version that does not exist. And a refused deletion is not a success:
# the script attempts every deletion, then exits with an error naming each
# release that stayed, so that the publishing job does not stay green over a
# release believed pruned.
#
# Names that are not of the form `vX.Y.Z` are neither counted nor touched: a
# release written by hand under another name is none of this script's business.
#
# ## Why the selection is separate
#
# For the reason of `prune-runs.sh`: a script runs locally, a block of YAML does
# not, and a deletion that no run verifies would be a belief. `--input` gives
# the list instead of fetching it, and `verify-gates.sh` uses it to prove the
# selection on a hand-written list.
#
# ## Usage
#
#   prune-releases.sh [--dry-run] [--input FILE]
#
#   --dry-run          writes the tags whose release would go, deletes nothing
#   --input FILE       reads the releases' tags from a file, one per line,
#                      instead of querying the API
#
# Without `--input`, the repository is that of `GITHUB_REPOSITORY` — which
# Actions sets — or, failing that, that of the current git repository. Reading
# works with a personal token; deleting requires `contents: write`.

set -euo pipefail

readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

usage() {
    cat <<'EOF'
prune-releases.sh [--dry-run] [--input FILE]

Prunes the releases: keeps every vX.Y.0 and the patches of the current
milestone, deletes the patches of past milestones. Tags stay. Drafts are
ignored. A refused deletion makes it exit with an error, after all have been
attempted. Under `workflow_dispatch` (GITHUB_EVENT_NAME), nothing is pruned.

  --dry-run          writes the tags whose release would go, deletes nothing
  --input FILE       reads the releases' tags from a file, one per line
  -h, --help         this help
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

repository="${GITHUB_REPOSITORY:-}"

# Written as an `if` rather than `[[ … ]] && …`, for the reason given in
# `prune-runs.sh`: under `set -e`, the short form kills the script in the most
# common case, the one where the repository is already known.
resolve_repository() {
    if [[ -z "${repository}" ]]; then
        require gh
        repository="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"
    fi
}

# The tags of the published releases, one per line. A draft is neither a
# version nor a release to prune: it is set aside here, before the current
# milestone is inferred and before any deletion is chosen.
fetch_tags() {
    if [[ -n "${input}" ]]; then
        [[ -f "${input}" ]] || { printf 'file not found: %s\n' "${input}" >&2; exit 1; }
        cat "${input}"
        return
    fi

    require gh
    resolve_repository
    gh api "repos/${repository}/releases" --paginate \
        --jq '.[] | select(.draft == false) | .tag_name'
}

# The selection. A function of the list, and nothing else — no clock, no
# network, no state.
#
# Two passes over the same list: the first finds the current milestone, the
# second condemns the patches that are not of it. The numbers are compared as
# numbers, and that is the whole point: compared as text, `0.9` would come after
# `0.10`.
select_doomed() {
    awk '
        match($0, /^v[0-9]+\.[0-9]+\.[0-9]+$/) {
            split(substr($0, 2), part, ".")
            count++
            major[count] = part[1] + 0
            minor[count] = part[2] + 0
            patch[count] = part[3] + 0
            name[count] = $0
            if (count == 1 || major[count] > top_major ||
                (major[count] == top_major && minor[count] > top_minor)) {
                top_major = major[count]
                top_minor = minor[count]
            }
        }
        END {
            for (i = 1; i <= count; i++) {
                current = major[i] == top_major && minor[i] == top_minor
                if (patch[i] > 0 && !current)
                    print name[i]
            }
        }
    '
}

# An on-demand rebuild republishes the package and does not prune. The check
# comes before reading the releases: to delete nothing, there is nothing to
# fetch. `--dry-run` escapes it, since it deletes nothing anyway.
if (( dry_run == 0 )) && [[ "${GITHUB_EVENT_NAME:-}" == "workflow_dispatch" ]]; then
    printf 'on-demand rebuild (workflow_dispatch): no release pruned\n'
    if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
        printf '## Pruning of the releases\n\nOn-demand rebuild: nothing is pruned.\n' \
            >> "${GITHUB_STEP_SUMMARY}"
    fi
    exit 0
fi

tags="$(fetch_tags)"
total="$(printf '%s' "${tags}" | grep -c '' || true)"
doomed="$(printf '%s\n' "${tags}" | select_doomed)"
doomed_count="$(printf '%s' "${doomed}" | grep -c '' || true)"

if (( dry_run == 1 )); then
    [[ -z "${doomed}" ]] || printf '%s\n' "${doomed}"
    printf '%s%d releases, %d to delete, %d kept%s\n' \
        "${BOLD}" "${total}" "${doomed_count}" "$(( total - doomed_count ))" "${RESET}" >&2
    exit 0
fi

require gh
resolve_repository

deleted=0
refused=()
while read -r tag; do
    [[ -n "${tag}" ]] || continue
    # `</dev/null`: without it, gh would read standard input and swallow the
    # rest of the list. No `--cleanup-tag`: that is what keeps the tag.
    if gh release delete "${tag}" --repo "${repository}" --yes </dev/null >/dev/null 2>&1; then
        printf '  deleted: %s (the tag stays)\n' "${tag}"
        deleted=$(( deleted + 1 ))
    else
        printf 'refused: %s\n' "${tag}" >&2
        refused+=("${tag}")
    fi
done <<< "${doomed}"

printf '%s%d deleted, %d refused, %d remaining%s\n' \
    "${BOLD}" "${deleted}" "${#refused[@]}" "$(( total - deleted ))" "${RESET}"

if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    {
        printf '## Pruning of the releases\n\n'
        printf '| | |\n| :--- | ---: |\n'
        printf '| before | %d |\n' "${total}"
        printf '| deleted | %d |\n' "${deleted}"
        printf '| refused | %d |\n' "${#refused[@]}"
        printf '| after | %d |\n' "$(( total - deleted ))"
        printf '\nRule: every `vX.Y.0`, and the patches of the current milestone. '
        printf 'Tags are never deleted.\n'
    } >> "${GITHUB_STEP_SUMMARY}"
fi

# A refused deletion fails the job: exiting with 0 would leave the publication
# green over a release believed pruned. They have all been attempted before
# getting here, and each one is named.
if (( ${#refused[@]} > 0 )); then
    printf 'deletion refused, the release stays: %s\n' "${refused[*]}" >&2
    exit 1
fi
