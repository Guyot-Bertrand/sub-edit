#pragma once

#include <string>
#include <string_view>

namespace subedit::core {

/// The name of a language as a person reads it in a list: `French` for `fr` in English, `français`
/// in French, `Französisch` in German.
///
/// `code` is an ISO 639-1 code as an engine takes it (whisper.cpp's old `jw` for Javanese is read
/// as the `jv` ICU knows). `inLanguage` is the name of the language to write the name in — a
/// catalogue's name, `fr`, `pt_BR` — and English when empty. The names come from ICU, which already
/// holds them in every language: the program keeps no table of a hundred languages × twenty.
///
/// A code ICU does not know is returned as it stands, so that a list never shows a blank.
[[nodiscard]] std::string languageDisplayName(std::string_view code, std::string_view inLanguage);

} // namespace subedit::core
