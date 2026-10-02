#pragma once

// What a file of a batch comes to, as a record a script can read.
//
// One object per input, **never zero and never two**, whether the input
// succeeded or not (ADR 0038). This file is where that object is shaped; the
// loops of the batch only say what happened.

#include <subedit/cli/changes.hpp>
#include <subedit/cli/json.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/file_system.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::cli {

class Reporter;

/// The version of the shape of a record. On every one of them: a stream is cut,
/// concatenated and filtered, and each line must stay readable alone.
inline constexpr int kSchema = 1;

/// Why one file did not come to anything: a stable identifier, and the words.
///
/// **The identifier is promised, the words are not.** A script tests the kind;
/// the message is English for a human and is reworded when it should be.
struct Failure {
    std::string kind;
    std::string message;

    /// A failure nobody gave a kind: `refused`, the general one.
    Failure(std::string words) : kind{"refused"}, message{std::move(words)} {}

    Failure(std::string_view identifier, std::string words)
        : kind{identifier}, message{std::move(words)} {}
};

/// One count of an operation, by name.
///
/// **The sentence of narration and the counts of the record come from the same
/// object**, so that the text and the JSON cannot say two things.
struct Count {
    Count(std::string name, std::int64_t number) : key{std::move(name)}, value{number} {}

    std::string key;
    std::int64_t value;
};

/// The identifiers of the failures and warnings, which are promised.
[[nodiscard]] std::string_view idOf(core::DiagnosticKind kind);
[[nodiscard]] std::string_view idOf(core::FileErrorKind kind);
[[nodiscard]] std::string_view idOf(core::ReadErrorKind kind);
[[nodiscard]] std::string_view idOf(core::WriteErrorKind kind);
[[nodiscard]] std::string_view idOf(const core::OpenError& error);

/// An object of integers, in the order given.
[[nodiscard]] Json countsOf(const std::vector<Count>& counts);

/// What a reading had to decide, as an array of `{kind, line?, detail?, settled?}`.
///
/// **All of them, at every level of narration**: the text keeps them for level
/// three because it is made to be read, and a script wants them whatever was
/// asked of the narration.
[[nodiscard]] Json warningsOf(std::span<const core::Diagnostic> diagnostics);

/// The record of a file that came to nothing.
[[nodiscard]] Json failureRecord(std::string_view command,
                                 std::string_view file,
                                 const std::string& message,
                                 std::string_view kind);

/// The record of a file that was written.
///
/// `changes` is there when the operation lists what it changed (the subcommands
/// of text); the others have none, and the key is not written for them.
[[nodiscard]] Json writtenRecord(std::string_view command,
                                 std::string_view file,
                                 const std::filesystem::path& destination,
                                 const std::vector<Count>& counts,
                                 Json warnings,
                                 const std::optional<std::vector<TextChange>>& changes = {});

/// The record of a file that was worked out and not written (`--dry-run`).
///
/// **The same object as a written file's**, `dry_run` and `destination` aside:
/// what was computed is what the next run, without the option, will write.
[[nodiscard]] Json dryRunRecord(std::string_view command,
                                std::string_view file,
                                const std::vector<Count>& counts,
                                Json warnings,
                                const std::optional<std::vector<TextChange>>& changes = {});

/// The start of the record of a file that was read: the envelope, up to `ok`.
///
/// `warnings` receives `path-not-utf8` when the path could not be written as
/// JSON as it is — the fidelity of such a path is not promised, and it is said.
[[nodiscard]] Json
recordOf(std::string_view command, std::string_view file, bool ok, Json& warnings);

/// Says that `file` failed with `failure`: the line on standard error, whatever
/// the level, and the record on standard output when there is one.
void reportFailure(const Reporter& reporter, std::string_view file, const Failure& failure);

} // namespace subedit::cli
