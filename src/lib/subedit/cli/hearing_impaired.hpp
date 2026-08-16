#pragma once

// Taking the hearing-impaired mentions out of a file.

#include <subedit/cli/exit_code.hpp>

#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Removes the mentions of every path, and says how it went.
///
/// **The main text of the whole file.** No selection, because no subcommand has
/// one; and no `--document`, because nothing in this library has a notion of a
/// translation — the core takes one, and the window of phase 5 will pass it.
///
/// A file where nothing bites is written all the same, as `shift` writes one it
/// moved by zero: a subcommand given a destination writes to it, and making
/// this the exception would force a script to know which subcommands sometimes
/// produce a file and sometimes nothing.
[[nodiscard]] ExitCode removeHearingImpairedIn(subedit::core::FileSystem& files,
                                               const std::vector<std::string>& paths,
                                               const Destination& destination,
                                               const Reporter& reporter);

} // namespace subedit::cli
