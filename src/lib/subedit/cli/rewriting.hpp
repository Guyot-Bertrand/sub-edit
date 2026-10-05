#pragma once

// Reading a file, changing what it holds, writing it back as it was found.

#include <subedit/cli/changes.hpp>
#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/selection.hpp>

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

/// One thing an operation went on without: a stable identifier, and the words.
struct Warning {
    std::string kind;
    std::string detail;
};

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

    /// What it changed in the texts, subtitle by subtitle — for the operations
    /// that change texts, and only when it was asked (`Request`). Nothing means
    /// this operation does not list its changes, which an empty list does not:
    /// that one says there were none.
    std::optional<std::vector<TextChange>> changes{};

    /// What the operation employed, for the record only.
    Fields fields{};

    /// What it could not do in full and went on without — a pattern that gave up
    /// on a text. **Not a failure of the file**: it is written, and the exit code
    /// is that of a file that was. Said at level 1, and a `warnings` entry of the
    /// record (`kind` is promised, `detail` is the words).
    std::vector<Warning> warnings{};
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

/// What the loop of the batch asks of an operation on one file.
struct Request {
    /// Whether anyone reads the list of changes — `--dry-run` or `--format json`.
    ///
    /// **Asked, because the list costs**: a copy of each text that changes, for a
    /// file of thousands of subtitles. The operation builds it only then. How it
    /// builds it is its own business: the core knows which texts, and the
    /// operation is the one that holds the command.
    bool changes = false;

    /// The subtitles to act on: the whole file, or what `--range` names in it.
    ///
    /// **Resolved against this file, before the operation runs** — the bounds a
    /// range is judged by are the file's own, so a range that fits the first
    /// file of a batch and not the third fails the third, and only it.
    subedit::core::Selection selection;

    /// The text the operation acts on: the translation when the file is
    /// paired (`--document translation`), the main one otherwise.
    subedit::core::Document document = subedit::core::Document::Main;
};

/// An operation that can list what it changes, and acts on a selection.
///
/// It receives the selection **rather than choosing one**: the command line
/// drives the commands the window drives, and the window passes what is
/// selected. No subcommand takes `--range` yet — the ones that will (`adjust`,
/// `replace`, `case`, `italics`, `dialogue-dashes`, `correct`) say so in their
/// manual page when they do; until then every operation is handed the whole file.
using ChangingOperation = std::function<OperationOutcome(subedit::core::Session&, const Request&)>;

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

/// The same, for an operation that lists its changes and takes a selection.
///
/// `range` limits the operation to those subtitles, **for every file of the
/// batch**: it is read once and resolved for each file against that file's own
/// count. A file it does not fit fails with `range-out-of-bounds`, before the
/// operation touches it and without writing it; the others go on.
///
/// **`--dry-run` is the one thing this loop does that the operations do not**:
/// the file is read, the operation runs, the bytes are made — so that a
/// character the encoding cannot carry fails here as it fails there — and the
/// write is skipped. No operation has a line about it (ADR 0040).
///
/// **With a `pairing`, `paths` is the one main file, and what is written is the
/// translation** (ADR 0032): the translation is read, laid over the main file by
/// the method asked for, and the operation acts on it. It is written to its own
/// path and in its own format, and the main file is not touched — `--output`
/// names the translation, `--output-dir` takes its base name, `--in-place`
/// rewrites it. A paired run is **one file**: the caller refuses a batch.
[[nodiscard]] ExitCode rewriteAll(subedit::core::FileSystem& files,
                                  const std::vector<std::string>& paths,
                                  const std::optional<subedit::core::Encoding>& reading,
                                  const Destination& destination,
                                  const Reporter& reporter,
                                  std::string_view verb,
                                  const ChangingOperation& operation,
                                  const std::optional<Range>& range = std::nullopt,
                                  const std::optional<Pairing>& pairing = std::nullopt);

} // namespace subedit::cli
