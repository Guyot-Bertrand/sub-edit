#pragma once

// Walking a UTF-8 text one code point at a time.
//
// **The texts are valid UTF-8**, always: every one comes out of a decoder that
// produced it. Nothing here checks the bytes, and an offset passed in is an
// offset at the start of a code point — which is what every caller holds, a
// tag or a match never landing inside a character.

#include <cstddef>
#include <string_view>

namespace subedit::core {

/// The offset where the code point ending at `at` starts.
///
/// `at` is past the start of the text: there is a code point before it.
[[nodiscard]] std::size_t previousCodePoint(std::string_view text, std::size_t at);

/// The offset past the code point starting at `at`, or the end of the text.
[[nodiscard]] std::size_t nextCodePoint(std::string_view text, std::size_t at);

/// The code point starting at `at`, which is before the end of the text.
[[nodiscard]] char32_t codePointAt(std::string_view text, std::size_t at);

} // namespace subedit::core
