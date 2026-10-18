#!/usr/bin/env bash
# What the scripts read of the project and of the machine, written once.
#
# **This file is sourced, not executed** — executable for the reason given in
# `gate/common.sh`: `check-architecture.sh` wants every file of `src/scripts`
# to be. Run directly, it says what it is and stops.
#
#   project_version [FILE]  the number of `project(VERSION)`
#   current_load            the one-minute load average
#
# **Functions only, no variable.** `gate/timing.sh` sources it, hence every
# step, `gate.sh` and `verify-gates.sh`, each with a root and colours of its
# own: a variable set here, `readonly` or not, would sooner or later meet one.
#
# A script finds it from its own location, never from the current directory:
# `source "$(dirname "${BASH_SOURCE[0]}")/lib/project.sh"`.

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf 'this file is sourced, not executed: source %s\n' "${BASH_SOURCE[0]}" >&2
    exit 2
fi

# The number of `project(VERSION)` in a CMakeLists.txt: the file named, or
# standard input when none is — `git show REV:CMakeLists.txt | project_version`
# reads that of a commit, which is what a tag is compared with. Nothing is
# written when the line is missing; the caller says what that means.
#
# One pattern for every reader. Nine used to carry their own, in three
# spellings: `\+` is a GNU extension where `\{1,\}` is POSIX, `[0-9.]` took a
# lone dot for a version, and `gate/timing.sh` wanted single spaces and none
# after the number. This one keeps the POSIX repetition, requires a leading
# digit, and tolerates any blank around the number — on every tagged
# `CMakeLists.txt`, the three gave the same answer as this one.
project_version() {
    sed -n 's/^[[:space:]]*VERSION[[:space:]]\{1,\}\([0-9][0-9.]*\)[[:space:]]*$/\1/p' "$@" \
        | head -1
}

# The one-minute average, without a newline; nothing, and a failure, where
# `/proc` does not give it. The one-minute average and not the number of
# runnable tasks: that one is instantaneous, so a sample can fall between two
# bursts and declare quiet a machine that is not.
current_load() {
    local load _
    { read -r load _ </proc/loadavg; } 2>/dev/null || return 1
    printf '%s' "${load}"
}
