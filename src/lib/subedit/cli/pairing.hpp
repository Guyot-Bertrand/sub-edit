#pragma once

// Laying a translation file over a main one, for the subcommands that look at
// both or act on the translation.

#include <subedit/cli/json.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/model/document.hpp>

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
class Session;
} // namespace subedit::core

namespace subedit::cli {

/// A translation file to lay over the main one, and how the lines find their
/// subtitles — what `-t` and `--align-method` say.
struct Pairing {
    std::string translation;
    subedit::core::TranslationMethod method = subedit::core::TranslationMethod::Position;
};

/// The document a subcommand acts on: the translation when it is paired, and
/// the main one otherwise. **`-t` and `--document translation` come together**,
/// so that one fact is enough to tell.
[[nodiscard]] inline subedit::core::Document targetOf(const std::optional<Pairing>& pairing) {
    return pairing ? subedit::core::Document::Translation : subedit::core::Document::Main;
}

/// What laying the translation did.
struct Paired {
    subedit::core::TranslationOutcome outcome;

    /// What reading the translation had to decide.
    std::vector<subedit::core::Diagnostic> diagnostics;
};

/// Reads the translation of `pairing` and attaches it to the project of
/// `session`, as the window does: one command, which the session applies.
///
/// `reading` is what the translation is read with, **the same choices as the
/// main file** — a file counted in frames is read at the rate of the project.
/// The failure is the second half of a sentence whose first half is the path the
/// caller is working on — **not the translation's, which it does not repeat**:
/// the caller reports the translation when the translation is what it works on.
[[nodiscard]] std::expected<Paired, Failure> pair(const subedit::core::FileSystem& files,
                                                  subedit::core::Session& session,
                                                  const Pairing& pairing,
                                                  const subedit::core::ReadingChoices& reading);

/// How the lines were matched, as an object: the method, then the four counts
/// of `TranslationOutcome` — the same facts as `noticeOf`, none left to be read
/// out of a sentence.
[[nodiscard]] Json alignmentOf(const Pairing& pairing,
                               const subedit::core::TranslationOutcome& outcome);

} // namespace subedit::cli
