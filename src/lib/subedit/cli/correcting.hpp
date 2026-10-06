#pragma once

// Correcting the texts of a file with the patterns of the assistant.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_dictionary.hpp>

#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// What `correct` was given, as it was written.
struct CorrectionOptions {
    /// The tasks, comma separated: `mentions`, `common-errors`, `capitalization`,
    /// `line-break`.
    std::string tasks;

    /// The cascade of patterns every task of the run reads: `Zyyy`, `Latn`,
    /// `Latn-en`, `Latn-en-US`.
    std::string code;

    /// `human`, `ocr` or `human,ocr`; empty means both, as the window opens.
    std::string classes;

    /// Patterns, by their English name or `type:name`, switched on or off.
    std::vector<std::string> enable;
    std::vector<std::string> disable;

    /// Leave the subtitles the correction empties, empty, instead of removing them.
    bool keepBlankSubtitles = false;

    /// The dictionary the tasks that check words read: `fr_FR`, `en`… **Required
    /// with `join-words` or `split-words`**, and nothing else's.
    std::string language;

    /// What `line-break` breaks to, as written, empty when not given. **In
    /// characters**: the 24 of Gaupol is a width in ems, a unit this program has
    /// no font for, and its number means nothing in letters — so the length has no
    /// default. The lines default to Gaupol's own, 3, which the unit does not change.
    std::string maxLength;
    std::string maxLines;

    /// The skip gate, as written: a number, or `off`. Empty means the same limits
    /// as `maxLength` and `maxLines`, as Gaupol's own defaults do.
    std::string skipLength;
    std::string skipLines;
};

/// The settings of the assistant these options come to, or why they cannot be
/// honoured — **read before any file is**: a mistake about the command line is
/// not a mistake about a file.
///
/// **Nothing is read from the user's settings** (decision D2 of the spec of the
/// phase): not their activations, not their last choices. The run starts from
/// what the shipped `.conf` files say and is adjusted by `enable` and `disable`
/// alone, so that the result depends on the arguments, the files, and what the
/// installation provides. The three refusals that matter, each a usage error:
///
/// - `code` is required, since no task here reads without one;
/// - a name that designates no pattern of the tasks given — or two, of two
///   types, without a `type:` in front — is refused, because a misspelling must
///   be loud: an invocation has no stale setting to forgive;
/// - a task with no active pattern is refused: « nothing to do » is an answer
///   that is said, not a silent success.
[[nodiscard]] std::expected<subedit::core::CorrectionSettings, std::string>
correctionSettingsOf(const CorrectionOptions& options,
                     const subedit::core::PatternCatalogue& catalogue);

/// What a run of `correct` reads **before any file**: the patterns, the settings its
/// options come to, and — when words are to be joined or split — the dictionary.
struct CorrectionRun {
    subedit::core::PatternCatalogue catalogue;
    subedit::core::CorrectionSettings settings;

    /// Null unless the run joins or splits words. **Kept beside the checker**, which
    /// looks words up in a dictionary this provider opened.
    std::unique_ptr<subedit::core::SpellProvider> provider{};
    std::optional<subedit::core::SpellChecker> spellChecker{};
};

/// Makes the provider of dictionaries, when — and only when — a run needs one.
using SpellProviderFactory = std::function<std::unique_ptr<subedit::core::SpellProvider>()>;

/// Everything `correct` settles before it touches a file, or why it cannot go on.
///
/// The patterns are read once, from the two places the executable and the environment
/// give (ADR 0037): the shipped ones, and those a user dropped. An installation without
/// its patterns says so, at level one, and the run that follows cannot find a pattern to
/// play and says that as well. Then the settings (`correctionSettingsOf`), and last the
/// dictionary: **opened once, and before anything is read**, since a language nobody has
/// is a mistake about the command line, said in the words of the window, which greys the
/// function out and says so.
///
/// **The replacement list the window keeps is neither read nor written** (decision D2): no
/// configuration directory is given, so `spellReplacementFile` names none.
[[nodiscard]] std::expected<CorrectionRun, std::string>
prepareCorrection(subedit::core::FileSystem& files,
                  const Reporter& reporter,
                  const CorrectionOptions& options,
                  const std::filesystem::path& installedPatterns,
                  const std::filesystem::path& userPatterns,
                  const SpellProviderFactory& makeProvider);

/// Corrects the texts of every path under `settings`, and says how it went.
///
/// The count is the window's, by `noticeOfCorrection`: texts changed and
/// subtitles removed, never matches. **A pattern that cannot be applied is
/// named, with the subtitle it gave up on, and is not a failure of the file**:
/// the file is written, and the code is that of a file that was. `range` limits
/// the subtitles looked at; with a `pairing` it is the translation that is
/// corrected and written (`rewriteAll`). `spellChecker` is what joining and
/// splitting words ask, opened once by the caller for the whole run; the
/// dictionary was checked to exist before anything was read.
[[nodiscard]] ExitCode correctIn(subedit::core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 const subedit::core::PatternCatalogue& catalogue,
                                 const subedit::core::CorrectionSettings& settings,
                                 const std::optional<Range>& range,
                                 const Destination& destination,
                                 const Reporter& reporter,
                                 const std::optional<Pairing>& pairing = std::nullopt,
                                 const subedit::core::SpellChecker* spellChecker = nullptr);

} // namespace subedit::cli
