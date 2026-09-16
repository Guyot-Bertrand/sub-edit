#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// Removes the hearing-impaired mentions of `text` — sounds described between
/// square brackets or between parentheses.
///
/// Returns the cleaned text, or nothing when the subtitle does not survive the
/// removal. An empty string would not say that: a subtitle with no text is not
/// the same thing as a subtitle that is gone, and the corpus of cases spells
/// the two apart for exactly that reason.
///
/// **What shows is judged in the tags of `format`**: a subtitle left holding
/// `{\i1}{\i0}` in Advanced SSA shows nothing and goes, as one left holding
/// `<i></i>` in SubRip does. Issue #403.
///
/// The rule this implements is written case by case in
/// `src/test/data/textes/mentions.cas`, which came before the code. Where the
/// two disagree, the corpus is right.
[[nodiscard]] std::optional<std::string> withoutHearingImpaired(std::string_view text,
                                                                SubtitleFormat format);

} // namespace subedit::core
