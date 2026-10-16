#!/usr/bin/env bash
# Checks the three obligations of a pull request that nothing else catches.
#
# The CI already checks the quality gate, the grammar of the commit messages and
# the title of the pull request. Three obligations escaped it, all mechanically
# decidable, all done by hand until now: the "Closes #N" line, the patch bump
# and the regeneration of the changelog.
#
# Called by .github/workflows/pull-request.yml, which hands it the values of the
# GitHub event through the environment. Kept here rather than written in the
# YAML: a script can be tested locally, a workflow step cannot — and
# src/scripts/verify-gates.sh proves that each of the three checks closes.
#
# Inputs:
#
#   PR_BODY   body of the pull request
#   BASE_SHA  commit of the base branch, to read the starting version from
#
# Arguments: the checks to run among "closes", "version" and "changelog".
# Without arguments, all three. Naming them one by one serves the proof: an
# injection demonstrates its check only if it alone can make the run fail.
#
# The requested checks are all evaluated, without stopping at the first
# failure: a pull request that lacks two things learns both in one round.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

PR_BODY="${PR_BODY:-}"
BASE_SHA="${BASE_SHA:-}"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

failures=0

pass() {
    printf '  %s✓%s %s\n' "${GREEN}" "${RESET}" "$1"
}

# First argument: the finding. The following ones: the lines of explanation.
fail() {
    printf '  %s✗%s %s\n' "${RED}" "${RESET}" "$1"
    shift
    local line
    for line in "$@"; do
        printf '      %s\n' "${line}"
    done
    failures=$((failures + 1))
}

# --- 1. The body carries "Closes #N" ----------------------------------------
#
# The only one of the three checks with a measured proof: during phase 2, the
# pull requests said "Ferme #22", applying the project's language rule of the
# time. GitHub recognises only its own keywords, all English — five issues
# stayed open after their pull request was merged.
#
# Only "Closes" is accepted here, although GitHub recognises eight ("fixes",
# "resolves"…). That is deliberate: CLAUDE.md names "Closes #N", and a check
# more permissive than the rule would let the rule drift — which is precisely
# what it exists to prevent.
check_closes() {
    # GitHub returns the body of a pull request with CRLF line endings. The
    # trailing \r would make the end-of-line anchor fail on a correct body.
    local body="${PR_BODY//$'\r'/}"

    # The line may carry something else after the number — GitHub closes the
    # issue in that case, and refusing "Closes #42, the first of three" would
    # refuse a correct pull request. What is required is that the line starts
    # with the keyword and its number.
    if grep -Eq '^[[:space:]]*Closes #[0-9]+([^0-9].*)?$' <<<"${body}"; then
        pass "the body carries a \"Closes #N\" line"
        return
    fi

    fail "the body carries no \"Closes #N\" line" \
        "GitHub closes an issue only on its own keywords, all English:" \
        "\"Ferme #42\" is not one of them, and the issue stays open after the merge." \
        "Add a \"Closes #42\" line to the body of the pull request."
}

# --- 2. The patch has been incremented --------------------------------------
#
# The check is about "different", not "patch + 1": a pull request that closes a
# phase bumps the minor and resets the patch to zero, which also makes the
# number differ without incrementing it.
declared_version() {
    sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' | head -1
}

check_version() {
    if [[ -z "${BASE_SHA}" ]]; then
        fail "BASE_SHA is not set" \
            "without a base commit, the starting version cannot be read."
        return
    fi

    local base=""
    base="$(git -C "${REPO_ROOT}" show "${BASE_SHA}:CMakeLists.txt" 2>/dev/null \
        | declared_version || true)"
    if [[ -z "${base}" ]]; then
        fail "unreadable version in CMakeLists.txt at ${BASE_SHA:0:8}" \
            "is the history complete? the workflow requires fetch-depth: 0."
        return
    fi

    local head=""
    head="$(declared_version <"${REPO_ROOT}/CMakeLists.txt" || true)"
    if [[ -z "${head}" ]]; then
        fail "unreadable version in CMakeLists.txt" \
            "project(VERSION) is the source of the current number."
        return
    fi

    if [[ "${head}" != "${base}" ]]; then
        pass "the version goes from ${base} to ${head}"
        return
    fi

    fail "the version has not moved (${head})" \
        "every pull request increments the patch in its own diff." \
        "Bump project(VERSION) in CMakeLists.txt."
}

# --- 3. The CHANGELOG has been regenerated ----------------------------------
#
# The comparison is between the file as it is and the file as git-cliff produces
# it — not with "git diff", which would compare the working copy with the index
# and let through a change that the regeneration has just erased.
#
# No false positive on maintenance commits: a `chore` commit is left out by
# cliff.toml, so a pull request that contains only those does not move the
# changelog — and the check passes, correctly.
check_changelog() {
    local changelog="${REPO_ROOT}/CHANGELOG.md"
    local backup
    backup="$(mktemp)"
    cp "${changelog}" "${backup}"

    if ! make -C "${REPO_ROOT}" --no-print-directory changelog >/dev/null 2>&1; then
        cp "${backup}" "${changelog}"
        rm -f "${backup}"
        fail "regenerating the changelog failed" \
            "is git-cliff installed? ./src/scripts/setup-toolchain.sh installs it."
        return
    fi

    local difference=""
    difference="$(diff -u --label 'CHANGELOG.md (committed)' --label 'CHANGELOG.md (regenerated)' \
        "${backup}" "${changelog}" || true)"

    cp "${backup}" "${changelog}"
    rm -f "${backup}"

    if [[ -z "${difference}" ]]; then
        pass "CHANGELOG.md is up to date"
        return
    fi

    fail "CHANGELOG.md has not been regenerated" \
        "run \"make changelog\" and commit the result."
    printf '%s\n' "${difference}" | sed 's/^/      /'
}

# --- Run --------------------------------------------------------------------

controls=("$@")
if (( ${#controls[@]} == 0 )); then
    controls=(closes version changelog)
fi

printf '%spull request checks%s\n\n' "${BOLD}" "${RESET}"

for control in "${controls[@]}"; do
    case "${control}" in
        closes) check_closes ;;
        version) check_version ;;
        changelog) check_changelog ;;
        *)
            printf '%sunknown check: %s%s\n' "${RED}" "${control}" "${RESET}" >&2
            printf 'expected: closes, version, changelog\n' >&2
            exit 2
            ;;
    esac
done

printf '\n'
if (( failures > 0 )); then
    printf '%s%d check(s) failed%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sthe checks pass%s\n' "${GREEN}" "${RESET}"
