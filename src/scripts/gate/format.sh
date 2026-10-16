#!/usr/bin/env bash
# The format of the sources, applied or checked.
#
# `--check` modifies nothing and fails on the first difference; without it, the
# script rewrites. It is the same list of files in both cases, and that is what
# makes `make format` repair exactly what `make format-check` refuses.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

check=0

usage() {
    cat >&2 <<'USAGE'
usage: format.sh [--check]

  --check  checks without modifying, and fails on the first difference
USAGE
    exit 2
}

while (($# > 0)); do
    case "$1" in
    --check) check=1; shift ;;
    -h | --help) usage ;;
    *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

require clang-format

# `find` and not a Makefile variable: the list is computed where it is used,
# and a file added between two calls enters it without anything else knowing.
mapfile -t sources < <(find "${REPO_ROOT}/src" \( -name '*.cpp' -o -name '*.hpp' \) | sort)

if ((check)); then
    step "format"
    clang-format --dry-run --Werror "${sources[@]}"
else
    step "formatting"
    clang-format -i "${sources[@]}"
fi
