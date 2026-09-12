#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <array>
#include <string>
#include <string_view>

namespace subedit::core {

/// The four ways a text can be re-cased.
///
/// **Gaupol's four, and its words** — `Text ▸ Case` offers exactly these, and
/// nothing decides between them but what a user wants to read.
enum class LetterCase {
    Title,    ///< Every Word Takes A Capital
    Sentence, ///< Only the first letter of the text
    Upper,    ///< EVERYTHING IN CAPITALS
    Lower,    ///< everything in small letters
};

/// The four, in the order the enumeration declares them.
///
/// For walking them, never for deciding anything — as `kSubtitleFormats` is for
/// the formats.
inline constexpr std::array<LetterCase, 4> kLetterCases = {
    LetterCase::Title,
    LetterCase::Sentence,
    LetterCase::Upper,
    LetterCase::Lower,
};

/// Returns `text` re-cased, its tags left exactly where they were.
///
/// **ICU does the work, and there was no second candidate.** `std::toupper`
/// reads one byte at a time and would cut an accented letter in two; a table
/// written by hand would be wrong for every language nobody thought of. ICU is
/// a dependency of this project since ADR 0027, and it knows all four.
///
/// **What precedes the first letter or digit is left alone**, which is Gaupol's
/// rule: a dialogue dash, an opening quote, a bracket survive a text put in
/// sentence case rather than becoming the thing that gets capitalised.
/// **The tags are not text and do not change case**: putting a subtitle in
/// small letters must not turn its `<I>` into an `<i>`. The tag-aware parser of
/// ADR 0009 holds them aside, which is also why this takes a format.
[[nodiscard]] std::string recased(std::string_view text, LetterCase wanted, SubtitleFormat format);

} // namespace subedit::core
