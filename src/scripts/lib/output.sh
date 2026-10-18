#!/usr/bin/env bash
# The colours and the two report lines the scripts write, written once.
#
# **This file is sourced, not executed** — executable for the reason given in
# `gate/common.sh`. Run directly, it says what it is and stops.
#
#   BOLD GREEN RED YELLOW RESET   the escape sequences, `readonly`
#   report_success MESSAGE…       « ✓ MESSAGE » on standard output
#   report_failure MESSAGE…       « ✗ MESSAGE » on standard error, and one more
#                                 in `failures`, which the script sets to zero
#                                 and reads at the end
#
# The colours are `readonly`: a script that sources this file defines none of
# its own. A copy remains where this file cannot be reached: in the script that
# `check-rpm.sh` hands to a container, which does not see the repository.

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf 'this file is sourced, not executed: source %s\n' "${BASH_SOURCE[0]}" >&2
    exit 2
fi

readonly BOLD=$'\033[1m'
readonly GREEN=$'\033[32m'
readonly RED=$'\033[31m'
# Yellow came with #270: a step that succeeds without having been able to do
# its work is neither green nor red. Benchmarks on a busy machine are the only
# case to date.
readonly YELLOW=$'\033[33m'
readonly RESET=$'\033[0m'

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}
