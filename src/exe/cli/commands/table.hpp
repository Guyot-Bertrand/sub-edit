#pragma once

// The table of subcommands: the one place that lists them.
//
// **Adding a subcommand is one line in `commands()` plus its own files.** The
// files hold the options, their declaration and the run function, and export a
// single `declareXxx` (see `command.hpp`); `run()` in application.cpp knows
// nothing of any of them, it walks this table. The order of the table is the
// order of `--help`, which lists subcommands as they are declared.

#include <span>

#include "command.hpp"

namespace subedit::cli {

/// One subcommand: its name on the command line, and how to declare it.
struct Command {
    std::string_view name;
    Declare declare;
};

/// Every subcommand, in the order the help lists them.
[[nodiscard]] std::span<const Command> commands();

} // namespace subedit::cli
