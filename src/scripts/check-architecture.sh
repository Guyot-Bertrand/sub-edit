#!/usr/bin/env bash
# Checks the project invariants that review alone would not hold.
#
# They are stated in docs/specs/00-fondations.md. Writing them into an
# automatic check rather than a review is what keeps them true six months from
# now.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MAIN_MAX_LINES=40

# shellcheck source=src/scripts/lib/project.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/project.sh"
# shellcheck source=src/scripts/lib/output.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/output.sh"

failures=0

# Invariant 1 — subedit_core knows neither Qt nor any graphical interface.
check_core_has_no_ui() {
    local core="${REPO_ROOT}/src/lib/subedit/core"
    [[ -d "${core}" ]] || return 0

    local offenders
    offenders="$(grep -rlE '#include[[:space:]]*[<"](Q[A-Z]|QtCore|QtWidgets|QtGui|gtk|gtkmm)' \
        "${core}" 2>/dev/null || true)"

    if [[ -n "${offenders}" ]]; then
        report_failure "subedit_core includes a graphical interface dependency:
$(printf '    %s\n' ${offenders})
    the core must stay usable by the CLI and the tests, without a toolkit."
    else
        report_success "subedit_core is free of any interface dependency"
    fi
}

# Invariant 2 — a main contains wiring only.
check_executables_are_thin() {
    local main_file
    while IFS= read -r -d '' main_file; do
        local relative="${main_file#"${REPO_ROOT}"/}"

        local lines
        lines="$(grep -cve '^[[:space:]]*$' -e '^[[:space:]]*//' "${main_file}" || true)"
        if (( lines > MAIN_MAX_LINES )); then
            report_failure "${relative} has ${lines} meaningful lines, at most ${MAIN_MAX_LINES}
    move the logic into a library of src/lib."
            continue
        fi

        if grep -qE '^[[:space:]]*(class|struct)[[:space:]]+[A-Z]' "${main_file}"; then
            report_failure "${relative} defines a type
    an executable wires, it does not model."
            continue
        fi

        report_success "${relative} stays wiring (${lines} meaningful lines)"
    done < <(find "${REPO_ROOT}/src/exe" -name 'main.cpp' -print0 2>/dev/null)
}

# Invariant 3 — the scripts are executable in the git index.
#
# A script committed as 100644 runs fine locally, where the execute bit exists
# on disk, and fails in CI with "Permission denied" after a fresh clone. The
# mode recorded in git is therefore the only source of truth that counts.
check_scripts_are_executable() {
    local offenders
    offenders="$(git -C "${REPO_ROOT}" ls-files -s src/scripts \
        | awk '$1 != "100755" { print $4 }')"

    if [[ -n "${offenders}" ]]; then
        report_failure "scripts not executable in the git index:
$(printf '    %s\n' ${offenders})
    fix with: git update-index --chmod=+x <file>"
    else
        report_success "the scripts of src/scripts are executable"
    fi
}

# Invariant — no test case title is in French.
#
# A test title is what `--list-tests` and every failure print, so it is written
# in English like everything else the repository holds — ADR 0043. This check
# predates the repository-wide one and stays because it is stricter: the
# repository-wide check needs two French function words to call a sentence
# French, while here **a non-ASCII character is enough**, since no title of
# this repository needs one (#273).
#
# **The check is a net, and says so.** Recognising a language is a heuristic:
# this one looks for a non-ASCII character, or one of the function words that
# exist only in French. Replayed on the tree before #273, it found 102 titles,
# two of which a careful review had let through.
#
# **The exemption is named, one only**: an English title that quotes a keyword
# of the fixture format, which is French. A list of titles rather than a
# pattern, so that adding an exemption is a gesture a diff shows.
check_test_titles_are_english() {
    local tests="${REPO_ROOT}/src/test"
    [[ -d "${tests}" ]] || return 0

    local french_words='le|les|un|une|des|du|et|qui|que|ne|pas|dans|pour|avec'
    french_words+='|sans|est|sont|aux|cette|ses|leur|elle|sur|quand|deux|rien'
    french_words+='|tel|telle|autre|autres|chaque|toujours|jamais|donc'

    # The English titles that carry a French datum, and why.
    local -a allowed=(
        # The keyword of the .cas format, which is French because the fixtures
        # are: the title quotes it as its neighbour quotes "=".
        '"an expected of supprimé means the subtitle does not survive"'
    )

    local found
    found="$(grep -rhoP 'TEST_CASE\(\s*"(?:[^"\\]|\\.)*"' "${tests}" 2>/dev/null \
        | sed 's/^TEST_CASE(\s*//' \
        | grep -P "[^\x00-\x7F]|(^|[^[:alnum:]])(${french_words})([^[:alnum:]]|$)" \
        | sort -u || true)"

    local one
    for one in "${allowed[@]}"; do
        found="$(grep -Fxv "${one}" <<< "${found}" || true)"
    done

    if [[ -n "${found}" ]]; then
        report_failure "test case titles that are not in English:
$(printf '%s\n' "${found}" | sed 's/^/    /')
    the repository is written in English (ADR 0043). A title sometimes quotes a
    French datum: then add it to the allowed list of this check, with its reason."
    else
        report_success "no test case title is in French"
    fi
}

# Invariant 5 — no test case name starts with a dash.
#
# CTest passes the case name as an argument to the test binary, and Catch2 then
# reads a name starting with a dash as one of its own options: "-v alone is the
# default" becomes a request for verbosity, and the case fails although nothing
# about its title looks wrong.
#
# What makes the defect costly is that it shows **only** through CTest, hence in
# the gate and in CI. Running the binary by hand runs every case at once,
# without ever naming one of them: the test passes. It was paid for twice while
# the CLI was being written before being recorded here.
check_test_names_are_not_options() {
    local tests="${REPO_ROOT}/src/test"
    [[ -d "${tests}" ]] || return 0

    local offenders
    offenders="$(grep -rn 'TEST_CASE("-' "${tests}" 2>/dev/null || true)"

    if [[ -n "${offenders}" ]]; then
        report_failure "a test case name starts with a dash:
$(printf '    %s\n' "${offenders}")
    CTest passes it as an argument, and Catch2 reads an option in it; rename it."
    else
        report_success "no test case name can pass for an option"
    fi
}

# The expected outputs of `--format json` are JSON, carry the envelope and have
# no decimal number — ADR 0038, issue #556.
#
# **A second look, independent of the writer**: the C++ has no JSON reader, and
# its tests compare the output with these files. If the writer and an expected
# file agreed on a mistake, only a parser that is not ours would see it. The
# script lives apart (`check-json-fixtures.py`); `verify-gates.sh` proves that
# it refuses a deliberately wrong expected file.
check_json_fixtures_are_valid() {
    local output
    if output="$("${REPO_ROOT}/src/scripts/check-json-fixtures.py" 2>&1)"; then
        report_success "the JSON expected outputs are valid, with no decimal number"
    else
        report_failure "the JSON expected outputs are not valid:
$(printf '%s\n' "${output}" | sed 's/^/    /')"
    fi
}

# Invariant 6 — nothing under src/ reads the reference repository.
#
# reference/gaupol is a clone of Gaupol, present to be read by a human. It is
# not tracked by git, is not part of the project, and its tree is kept
# read-only. It is absent from CI, absent from any freshly cloned machine, and
# absent from an archive of the repository.
#
# A test that reads it therefore passes for whoever has it and fails for
# everyone else — or, worse, declares itself skipped and proves nothing while
# staying green. The temptation is real: phase 4 may take over Gaupol's pattern
# format, and its files are right there, a relative path away.
#
# Tracked test data lives in src/test/data/. src/data/ is not an option: git
# ignores it, it is each machine's private corpus.
#
# The check covers the code and the tests, not src/scripts: the scripts drive
# the clone's lock and forbid committing it, and naming it is their job. A
# per-file exception list would have grown with every added script; targeting
# the three directories that have no business there never grows.
check_nothing_reads_the_reference() {
    local offenders=""
    local directory
    for directory in lib exe test; do
        [[ -d "${REPO_ROOT}/src/${directory}" ]] || continue
        offenders+="$(grep -rn 'reference/' "${REPO_ROOT}/src/${directory}" 2>/dev/null || true)"
    done

    if [[ -n "${offenders}" ]]; then
        report_failure "code under src/ points at the reference repository:
$(printf '    %s\n' "${offenders}")
    It is absent from CI and from any machine that has not cloned it.
    Tracked test data lives in src/test/data/."
    else
        report_success "nothing under src/ reads the reference repository"
    fi
}

# Invariant 7 — nothing under src/ is generated by a tool.
#
# Four gates filter on `src/`: the format, static analysis, the coverage
# ratchet and the computation of clang-tidy's scope. None can tell a written
# line from a produced one; all of them are nevertheless correct, and **for one
# reason only**: Qt generates into `build/`, and `src/` contains only what a
# human typed.
#
# That reason was written nowhere, so nothing held it. A `.ui` compiled in the
# wrong place, a file copied over from a build directory, and the four gates
# start reading code that nobody wrote — without reporting anything.
#
# **The coverage ratchet is the most insidious of the four.** A format check
# that complains is seen; a skewed line count is not seen, it is believed.
#
# What the check looks for: the names moc, uic and rcc give to what they
# produce, and the warning they write at the top of every file. If one day a
# generated file really must live under src/, this check will name it — and
# that is when the question has to be reopened, not afterwards.
check_sources_are_handwritten() {
    local by_name by_marker offenders

    by_name="$(find "${REPO_ROOT}/src" \
        \( -name 'moc_*.cpp' -o -name 'ui_*.h' -o -name 'qrc_*.cpp' -o -name '*.moc' \
        -o -name 'mocs_compilation*.cpp' \) -printf '%P\n' 2>/dev/null || true)"

    # The warning the three Qt tools write at the top of what they produce, and
    # that no hand-written file carries.
    #
    # **It is assembled rather than written out**, otherwise this script would
    # report itself: it lives under src/, and the pattern it looks for would
    # appear in it. The defect was seen on the first run.
    local marker="All changes made in this file"
    by_marker="$(grep -rl "${marker} will be lost" \
        "${REPO_ROOT}/src" 2>/dev/null | sed "s|^${REPO_ROOT}/src/||" || true)"

    offenders="$(printf '%s\n%s\n' "${by_name}" "${by_marker}" | sed '/^$/d' | sort -u)"

    if [[ -n "${offenders}" ]]; then
        report_failure "generated code lives under src/:
$(printf '    %s\n' ${offenders})
    The format, static analysis, coverage and clang-tidy's scope all filter
    on src/, assuming it is written by hand."
    else
        report_success "nothing under src/ is generated by a tool"
    fi
}

# Invariant 4 — a version tag on HEAD matches the declared version.
#
# The tag and project(VERSION) are two writings of the same number, and nothing
# held them together: a tag set without bumping CMake gives a binary that
# announces a stale version. The check is inert as long as no tag points at
# HEAD, so it does not get in the way of ongoing work.
check_version_matches_tag() {
    local tag
    # "|| true" is not decorative: without a tag on HEAD, grep exits with 1, and
    # set -o pipefail together with set -e would stop the script instead of
    # letting it conclude that there is nothing to compare. That is the usual
    # case in continuous integration, where the clone does not fetch tags.
    tag="$(git -C "${REPO_ROOT}" tag --points-at HEAD 2>/dev/null \
        | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+$' | head -1 || true)"

    if [[ -z "${tag}" ]]; then
        report_success "no version tag on HEAD, nothing to compare"
        return 0
    fi

    # Read from the commit, not from the working tree. A tag designates a
    # commit; comparing it with files modified since compares two different
    # things. The case arises at the start of every phase: the branch starts
    # from the merge commit, which carries the previous phase's tag, and the
    # first bump of the new phase would raise a false alarm.
    local declared
    declared="$(git -C "${REPO_ROOT}" show HEAD:CMakeLists.txt 2>/dev/null \
        | project_version)"

    if [[ "v${declared}" != "${tag}" ]]; then
        report_failure "the tag ${tag} and CMakeLists.txt (${declared:-absent}) do not agree
    bump project(VERSION) before setting the tag, otherwise the binary lies."
    else
        report_success "the tag ${tag} matches the declared version"
    fi
}

# Invariant 8 — the model depends on no operation.
#
# ADR 0018 settles the direction: `format/` translates bytes into the model, so
# it needs the model; the model never needs a reader. What makes the rule
# tenable is that it is mechanical — the boundary has already faded once, when
# the types specific to formats were put under `model/` without anything
# reporting it.
check_model_depends_on_no_operation() {
    local model="${REPO_ROOT}/src/lib/subedit/core/model"
    [[ -d "${model}" ]] || return 0

    local offenders
    offenders="$(grep -rlE '#include[[:space:]]*<subedit/core/(format|io|edit|command)/' \
        "${model}" 2>/dev/null || true)"

    if [[ -n "${offenders}" ]]; then
        report_failure "the model depends on an operation:
$(printf '    %s\n' ${offenders})
    format/, io/, edit/ and command/ depend on the model; the reverse closes a cycle."
    else
        report_success "the model depends on no operation"
    fi
}

# Invariant — the repository is written in English; French is a locale.
#
# ADR 0043 and #660: French is refused in every tracked file except the two
# locale trees (`src/po/`, `docs/i18n/fr/`) and the files of `.french-allowlist`,
# which only ever loses entries as the translation proceeds. It replaces the
# split of #312, where only the C++ was held to English.
#
# The check lives in a script of its own: it classifies lines by a vote of
# function words, which reads better in Python than in awk. See
# `check-comment-language.py` for what it reads, what it leaves out, and how
# the allowlist is kept from growing.
check_repository_is_english() {
    if "${REPO_ROOT}/src/scripts/check-comment-language.py"; then
        return 0
    fi

    failures=$((failures + 1))
}

check_core_has_no_ui
check_repository_is_english
check_sources_are_handwritten
check_executables_are_thin
check_scripts_are_executable
check_version_matches_tag
check_test_names_are_not_options
check_test_titles_are_english
check_nothing_reads_the_reference
check_model_depends_on_no_operation
check_json_fixtures_are_valid

if (( failures > 0 )); then
    printf '\n%s%d architecture invariant(s) violated%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
