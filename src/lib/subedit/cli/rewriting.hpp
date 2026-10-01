#pragma once

// Reading a file, changing what it holds, writing it back as it was found.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class FileSystem;
class Session;
} // namespace subedit::core

namespace subedit::cli {

class Destination;
class Reporter;

/// What an operation did to one file: its counts, and the sentence they make.
///
/// **One object for the two readers.** The sentence is the narration of the
/// file — « 4000 subtitles shifted by 2.999 s » — and the counts are what the
/// record of `--format json` carries. A fixed sentence, decided once for every
/// file of the batch, could not say what a removal does, because those numbers
/// differ from one file to the next and only the operation knows them; and a
/// count that has to be dug out of a sentence is the thing a script must never
/// do. Both come from the same values, here, so that text and JSON cannot say
/// two things.
struct OperationResult {
    std::string sentence;
    std::vector<Count> counts;
};

/// What an operation comes to on one file: a result, or why it cannot.
using OperationOutcome = std::expected<OperationResult, Failure>;

/// What an operation does to one file, or why it cannot.
///
/// It receives a session rather than a bare list of subtitles, **so that the
/// command line drives the very commands the window will drive**. Applying the
/// arithmetic here instead would leave the core's operations exercised by
/// nothing but their own unit tests — and this phase exists to exercise them.
///
/// **It words its own result**: only the operation knows how many subtitles a
/// removal rewrote and how many it took away.
using Operation = std::function<OperationOutcome(subedit::core::Session&)>;

/// Applies `operation` to every path and writes each result back.
///
/// The format, the line endings and the byte order mark are those of the file
/// read: changing format is `convert`'s business, and a subcommand doing both
/// would make the failures of one indistinguishable from the failures of the
/// other.
///
/// `verb` names the operation in the summary of the batch — « 3 files
/// shifted ». What each file gets is the phrase the operation itself returned.
[[nodiscard]] ExitCode rewriteAll(subedit::core::FileSystem& files,
                                  const std::vector<std::string>& paths,
                                  const std::optional<subedit::core::Encoding>& reading,
                                  const Destination& destination,
                                  const Reporter& reporter,
                                  std::string_view verb,
                                  const Operation& operation);

} // namespace subedit::cli
