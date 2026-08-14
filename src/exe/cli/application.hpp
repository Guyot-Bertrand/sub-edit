#pragma once

// The command line itself: what the options are, and how they are read.
//
// Kept apart from main.cpp so that the entry point holds nothing but the
// call and the last-resort catch. Argument parsing is the one thing the
// architecture rule of docs/specs/00-fondations.md leaves in src/exe — it is
// not functional logic, and everything the tool *decides* lives in
// subedit_cli, which knows nothing of CLI11.

#include <subedit/cli/exit_code.hpp>

namespace subedit::cli {

/// Reads the command line and runs what it asks for.
[[nodiscard]] ExitCode run(int argc, char** argv);

} // namespace subedit::cli
