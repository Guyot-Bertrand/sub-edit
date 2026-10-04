#pragma once

// The three operations that rewrite the texts of a file without changing what it
// says: its case, its italics, its dialogue dashes.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/text/letter_case.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Puts the texts of every path in `wanted` case, tags left where they were, and
/// says how it went.
///
/// `range` limits the subtitles changed, in every file. A file where nothing
/// changes — every text already in that case — is written all the same and says
/// `nothing to change`: a destination given is a destination written.
[[nodiscard]] ExitCode recaseIn(subedit::core::FileSystem& files,
                                const std::vector<std::string>& paths,
                                const std::optional<subedit::core::Encoding>& reading,
                                subedit::core::LetterCase wanted,
                                const std::optional<Range>& range,
                                const Destination& destination,
                                const Reporter& reporter);

/// Puts the texts in italics (`italic`), or takes their italics out.
///
/// **Told which way**: the window has one button and asks the core which way it
/// means; a command line has two words and asks nothing. **A format that writes no
/// style refuses, file by file, with the reason** — as the window greys the entry
/// out rather than hide it — and the other files go on.
[[nodiscard]] ExitCode italicsIn(subedit::core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 bool italic,
                                 const std::optional<Range>& range,
                                 const Destination& destination,
                                 const Reporter& reporter);

/// Puts a dialogue dash at the head of the lines (`dashed`), or takes them off.
[[nodiscard]] ExitCode dialogueDashesIn(subedit::core::FileSystem& files,
                                        const std::vector<std::string>& paths,
                                        const std::optional<subedit::core::Encoding>& reading,
                                        bool dashed,
                                        const std::optional<Range>& range,
                                        const Destination& destination,
                                        const Reporter& reporter);

} // namespace subedit::cli
