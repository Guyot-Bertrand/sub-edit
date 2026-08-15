#pragma once

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
/// The rule this implements is written case by case in
/// `src/test/data/textes/mentions.cas`, which came before the code. Where the
/// two disagree, the corpus is right.
[[nodiscard]] std::optional<std::string> withoutHearingImpaired(std::string_view text);

} // namespace subedit::core
