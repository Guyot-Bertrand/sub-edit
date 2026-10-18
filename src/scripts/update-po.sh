#!/usr/bin/env bash
# Merge the template into every language catalogue — issues #659, #667 and #670.
#
# The template is rewritten first: merging a stale template would teach nothing. Then every
# `src/po/<lang>.po` is brought up to date with `msgmerge` against `subedit.pot` — new messages
# added empty, vanished ones commented out as obsolete:
#
# - **a language of src/po/LINGUAS**, installed and reviewed, is merged with fuzzy matching: a
#   changed sentence comes back as its old translation flagged "fuzzy", which a translator reviews;
# - **a seed** — any other `.po`, imported from Gaupol by `seed-catalogues.py` and not installed
#   (spec 15, D5) — is merged without fuzzy matching: nobody reviews a seed, so a guess would only
#   add noise that the next re-seed throws away. Its translations are kept, and a fresh seed is
#   left byte for byte as it is.
#
# **`--into DIR` is the same merge written elsewhere** (#670): the template is read as it is, not
# rewritten, every catalogue of the catalogue directory is merged into `DIR/<lang>.po`, and nothing
# of the catalogue directory is written. It is what `check-translations.py` runs, so that the gate
# judges a catalogue by exactly what `make po` would make of it — one function, `merge_catalogue`
# below, holds the flags of both. `--po-dir DIR` names another catalogue directory (default
# `src/po`), for the gate's fixtures and proofs. A language of LINGUAS without its file is left to
# the caller in that mode, which reports it with the rest.
#
#   update-po.sh                               rewrites the template, then merges in place
#   update-po.sh --into DIR [--po-dir PODIR]   merges PODIR's template into copies under DIR

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# shellcheck source=src/scripts/lib/output.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/output.sh"

po_dir="${REPO_ROOT}/src/po"
into=""
while (($# > 0)); do
    case "$1" in
    --into) [[ $# -ge 2 ]] || { printf 'usage: update-po.sh [--into DIR] [--po-dir DIR]\n' >&2; exit 2; }
        into="$2"; shift 2 ;;
    --po-dir) [[ $# -ge 2 ]] || { printf 'usage: update-po.sh [--into DIR] [--po-dir DIR]\n' >&2; exit 2; }
        po_dir="$2"; shift 2 ;;
    *)
        printf 'usage: update-po.sh [--into DIR] [--po-dir DIR]\n' >&2
        exit 2
        ;;
    esac
done

if [[ -z "${into}" && "${po_dir}" != "${REPO_ROOT}/src/po" ]]; then
    printf 'update-po.sh: --po-dir is for --into; make po rewrites src/po only\n' >&2
    exit 2
fi

if ! command -v msgmerge >/dev/null 2>&1; then
    printf '%s✗ msgmerge cannot be found%s\n' "${RED}" "${RESET}" >&2
    printf '  install it with: ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
fi

# The one place the merge flags are written: `--update` in place, or `--output-file` elsewhere.
# The messages of msgmerge are asked for in English, whatever the developer's locale.
merge_catalogue() {
    local catalogue="$1" output="$2" installed="$3"
    local -a flags=(--quiet)
    [[ "${installed}" == yes ]] || flags+=(--no-fuzzy-matching)
    if [[ "${output}" == "${catalogue}" ]]; then
        flags+=(--update --backup=none)
    else
        flags+=("--output-file=${output}")
    fi
    LANGUAGE=en msgmerge "${flags[@]}" "${catalogue}" "${po_dir}/subedit.pot"
}

if [[ -z "${into}" ]]; then
    "${REPO_ROOT}/src/scripts/update-pot.sh"
fi

installed="$(grep -v '^[[:space:]]*\(#\|$\)' "${po_dir}/LINGUAS" || true)"

for catalogue in "${po_dir}"/*.po; do
    [[ -e "${catalogue}" ]] || continue
    language="$(basename "${catalogue}" .po)"
    output="${catalogue}"
    [[ -z "${into}" ]] || output="${into}/${language}.po"
    if grep -qxF "${language}" <<<"${installed}"; then
        merge_catalogue "${catalogue}" "${output}" yes
        [[ -n "${into}" ]] || printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "src/po/${language}.po"
    else
        merge_catalogue "${catalogue}" "${output}" no
        [[ -n "${into}" ]] || printf '%s✓%s %s (seed)\n' "${GREEN}" "${RESET}" "src/po/${language}.po"
    fi
done

[[ -z "${into}" ]] || exit 0

# A language of LINGUAS without its file is an error the build reports too; say it here first.
while IFS= read -r language; do
    [[ -n "${language}" ]] || continue
    if [[ ! -f "${po_dir}/${language}.po" ]]; then
        printf '%s✗%s src/po/LINGUAS names "%s", and src/po/%s.po does not exist\n' \
            "${RED}" "${RESET}" "${language}" "${language}" >&2
        exit 1
    fi
done <<<"${installed}"
