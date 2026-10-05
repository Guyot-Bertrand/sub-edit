#pragma once

// Writing the translation of a pair at the positions of its main file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/core/model/encoding.hpp>

#include <optional>
#include <string>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Lays the translation of `pairing` over `mainPath` and writes it back out at the
/// positions of the main file, in its own format.
///
/// **The one thing the pairing does that a file alone does not** — spec D10: every
/// other subcommand that is given `-t` changes the translation it reads, and this
/// one changes nothing and writes what the alignment made. The main file is read
/// and not touched. The sentence of the file **is the alignment**, in the words
/// of the window (`noticeOf(TranslationOutcome)`), and the record carries its
/// counts as `alignment`.
///
/// Destinations are those of any run on one file: `--output` names the
/// translation, `--output-dir` takes its base name, `--in-place` rewrites it.
[[nodiscard]] ExitCode writeTranslationAt(subedit::core::FileSystem& files,
                                          const std::string& mainPath,
                                          const std::optional<subedit::core::Encoding>& reading,
                                          const Destination& destination,
                                          const Reporter& reporter,
                                          const Pairing& pairing);

} // namespace subedit::cli
