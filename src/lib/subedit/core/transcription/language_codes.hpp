#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// The two-letter code (ISO 639-1) of the language a container declares, which is how an
/// engine names it. Containers say `fra` or `eng` (ISO 639-2), or sometimes already `fr`.
///
/// Nothing for a tag that is not a language of the table — `und`, `mul`, a language no
/// engine is likely to take — so that the window offers nothing rather than a guess.
/// Case does not matter.
[[nodiscard]] std::optional<std::string> twoLetterCodeOf(std::string_view tag);

/// The code an engine takes for a language a person typed: `fr`, or `fra` or `fre` for French.
/// Case and surrounding blanks do not matter.
///
/// **Two letters that are not in the table are kept**: an engine knows about a hundred languages
/// and this table about thirty, and refusing `af` for want of a row would be wrong. Nothing for
/// anything else — a word, a longer tag — so that the surface says it is no language code.
[[nodiscard]] std::optional<std::string> readLanguageCode(std::string_view typed);

} // namespace subedit::core
