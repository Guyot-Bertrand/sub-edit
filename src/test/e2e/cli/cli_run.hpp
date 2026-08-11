#pragma once

// Running the command-line tool for real.
//
// A unit test links the library and calls into it. These tests do neither:
// they start the binary a user would start, and read what a user would see.
// Nothing here knows what subedit-cli does — only how to run it.

#include <string>
#include <vector>

namespace subedit::e2e {

/// What one run of subedit-cli produced.
struct CliRun {
    /// The process exit code.
    ///
    /// A process killed by a signal reports `128 + signal` rather than the
    /// exit status it never returned. Without that, a crash would be
    /// indistinguishable from a clean zero, which is the one value most
    /// assertions expect.
    int exitCode = 0;

    /// Standard output, verbatim, newlines included.
    std::string output;

    /// Standard error, kept apart from the above.
    std::string errors;
};

/// Runs subedit-cli with `args` and waits for it to finish.
///
/// No shell is involved: arguments reach the program exactly as written, with
/// no splitting, globbing or quoting to undo. The binary path comes from the
/// build (`SUBEDIT_CLI_BINARY`), never from the current directory.
///
/// Throws `std::system_error` when the process cannot be started or its output
/// cannot be read. A test that cannot run the binary has nothing to assert.
CliRun invoke(const std::vector<std::string>& args);

} // namespace subedit::e2e
