#!/usr/bin/env bash
# The tests under ASan and UBSan. The asan preset also registers the end-to-end
# tests, with no label filter: this is where they run.
#
# **`GATE_PROOF_TESTS`, reserved for `verify-gates.sh`** (#686): a regular expression of test
# names, and only those run. The proof of a gate injects ONE faulty test and expects the step to
# fail; replaying the three thousand others under sanitizers for that cost a quarter of an hour
# per proof. The step stays the real one — same tree, same options, same `ctest`. An expression
# that designates no test makes `ctest` return zero: the proof sees it, since it expects a
# failure. `gate.sh` refuses the variable for `check` and `check-local`: a reduced gate is not
# run by mistake.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "tests under sanitizers"
cmake --preset asan
cmake --build --preset asan -j "${JOBS}"
if [[ -n "${GATE_PROOF_TESTS:-}" ]]; then
    printf '%s  only the tests « %s » run (gate proof)%s\n' "${YELLOW}" "${GATE_PROOF_TESTS}" "${RESET}"
    "${REPO_ROOT}/src/scripts/limit-cores.sh" ctest --preset asan -R "${GATE_PROOF_TESTS}"
else
    "${REPO_ROOT}/src/scripts/limit-cores.sh" ctest --preset asan
fi
