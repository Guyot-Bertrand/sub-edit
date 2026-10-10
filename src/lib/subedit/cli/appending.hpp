#pragma once

// Putting files one after another into a single file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/model/encoding.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Appends every path after the first to the first, and writes the one result.
///
/// **The one subcommand whose arity is not a batch's**: N inputs, one output. The
/// first path is the base and gives the result its format, its encoding and its
/// line endings; each of the others crosses into that format through
/// `appendFile` — the very command the window drives — and is shifted from the
/// end of what precedes it, so that the files follow one another in the order
/// they were given. What crossing cost is said file by file, in the words of the
/// window.
///
/// **All or nothing**: the first input that cannot be read stops the run, and
/// nothing is written. A film in two parts with one part missing is not a film
/// in one part.
///
/// A destination that is one of the inputs is refused as a usage error, before
/// any file is read. `paths` holds at least two entries; the caller checks.
[[nodiscard]] ExitCode appendAll(subedit::core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 const Destination& destination,
                                 const Reporter& reporter,
                                 bool sort = false);

} // namespace subedit::cli
