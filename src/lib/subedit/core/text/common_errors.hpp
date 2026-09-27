#pragma once

// The correction of common errors, as `correct_common_errors` of Gaupol does it.
//
// A function of texts: it takes the patterns to apply and the texts, and gives
// the corrected texts and what it could not do. It touches no project — the
// assistant of the phase decides what to do with the answer (decision D8).

#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_engine.hpp>

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {

/// What kept a pattern from being applied.
enum class FailureKind {
    Untranslatable,     ///< its expression uses what the translation to the engine refuses
    CompileError,       ///< the engine refused its expression
    InvalidReplacement, ///< its replacement cannot be read as Python reads it
    TimedOut,           ///< the engine gave up on a text
    TooManyPasses,      ///< `Repeat` still changed the text after a hundred passes
    TooLong,            ///< the text grew beyond what a subtitle can hold
};

/// A pattern that was not applied, and to what.
///
/// **Two ranges**: a pattern that does not compile is out for every text, and
/// says so once, with no `text`; one that gives up on a text is out for that
/// text only, and names it.
struct PatternFailure {
    FailureKind kind;

    /// The record, as its file names it: code, rank, name.
    std::string code;
    std::size_t rank = 0;
    std::string name;

    /// The index of the text it gave up on. Nothing when it is out for all.
    std::optional<std::size_t> text;

    /// The engine's reason, or what was found.
    std::string detail;

    friend bool operator==(const PatternFailure&, const PatternFailure&) = default;
};

struct CorrectedTexts {
    /// One per text given, in the same order, unchanged where nothing applied.
    std::vector<std::string> texts;
    std::vector<PatternFailure> failures;
};

/// The most passes `Repeat` may take on one text.
///
/// Gaupol repeats until a pass finds nothing, which never ends for a pattern
/// whose replacement is matched again. Here it stops when the text stops
/// changing — the same answer wherever Gaupol terminates — and this bounds what
/// a pattern that never settles could do.
inline constexpr int kMaxRepeatPasses = 100;

/// The most a text may grow to while a pattern works on it.
///
/// A subtitle is a few hundred bytes. A replacement that doubles what it finds,
/// under `Repeat`, would otherwise fill the memory before the passes ran out.
inline constexpr std::size_t kMaxTextBytes = 16384;

/// Applies `patterns` to each of `texts`, in order, each on the text the
/// previous one left — and `Repeat` again until the text stops changing.
///
/// Only the patterns of kind `CommonError` are applied; the others are not this
/// function's. **Which patterns to give is the caller's**: `enabled` is a
/// setting, and the class filter of decision D4 is another.
///
/// **A pattern that cannot be applied leaves the text as it was before it**, a
/// half-done replacement being worse than none, and is reported.
[[nodiscard]] CorrectedTexts correctCommonErrors(const PatternEngine& engine,
                                                 std::span<const CorrectionPattern* const> patterns,
                                                 std::span<const std::string> texts);

} // namespace subedit::core
