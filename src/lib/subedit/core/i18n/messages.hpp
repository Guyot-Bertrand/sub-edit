#pragma once

// What the rest of the program calls to speak — ADR 0042.
//
// `translate("Open")` returns the installed catalogue's word for it, and the text itself when
// there is no catalogue, which is how English works: nothing is installed. A message is the
// English text (D2), so a missing translation degrades to something readable.
//
// **The catalogue is installed once, at start-up, before any thread runs**: the functions
// return views into it, and replacing it while one is held would leave that view dangling.
// A test that wants another language installs one and puts English back.

#include <subedit/core/i18n/catalogue.hpp>

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

namespace subedit::core {

/// Makes `catalogue` the one that `translate` consults; null puts English back.
void installCatalogue(std::shared_ptr<const Catalogue> catalogue);

/// The translation of `text`, or `text`.
[[nodiscard]] std::string_view translate(std::string_view text);

/// The translation of `text` in `context`, or `text`. A context separates two identical
/// English words that translate differently; it is not a way of grouping messages.
[[nodiscard]] std::string_view translateIn(std::string_view context, std::string_view text);

/// The form that `n` selects, or — with no translation — `singular` for one and `plural` for
/// anything else. A sentence with a count never concatenates an `s`.
[[nodiscard]] std::string_view
translatePlural(std::string_view singular, std::string_view plural, std::uint64_t n);

/// `%1`..`%9` replaced by the arguments, in one pass: an argument that itself contains `%2`
/// is not expanded again. A `%` not followed by a digit from 1 to 9, and a number beyond the
/// arguments, stay as written. Positional so that a translation may reorder them.
[[nodiscard]] std::string substitute(std::string_view text,
                                     std::initializer_list<std::string_view> arguments);

} // namespace subedit::core
