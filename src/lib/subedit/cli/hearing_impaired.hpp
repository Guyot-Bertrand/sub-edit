#pragma once

// Taking the hearing-impaired mentions out of a file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/text/hearing_impaired.hpp>

#include <optional>
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
/// **The whole of the main text, or of the translation** when it is `pairing`:
/// see `rewriteAll`. No selection, because no subcommand has one. A mention that
/// would empty a translation leaves it empty and keeps the subtitle, as the
/// window does.
///
/// **Square brackets only, unless `brackets` says otherwise**: parentheses are often a whispered
/// line (issue #691), and `--parentheses` is how a user whose files describe sounds with them asks.
///
/// A file where nothing bites is written all the same, as `shift` writes one it
/// moved by zero: a subcommand given a destination writes to it, and making
/// this the exception would force a script to know which subcommands sometimes
/// produce a file and sometimes nothing.
[[nodiscard]] ExitCode
removeHearingImpairedIn(subedit::core::FileSystem& files,
                        const std::vector<std::string>& paths,
                        const std::optional<subedit::core::Encoding>& reading,
                        const Destination& destination,
                        const Reporter& reporter,
                        const std::optional<Pairing>& pairing = std::nullopt,
                        subedit::core::MentionBrackets brackets = {});

} // namespace subedit::cli
