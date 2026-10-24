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

} // namespace subedit::core
