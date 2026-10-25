#include <subedit/core/transcription/language_display.hpp>

#include <unicode/locid.h>
#include <unicode/unistr.h>

namespace subedit::core {

namespace {

/// whisper.cpp's code for Javanese is the one ISO 639-1 replaced.
[[nodiscard]] std::string modernCode(std::string_view code) {
    return code == "jw" ? "jv" : std::string{code};
}

} // namespace

std::string languageDisplayName(std::string_view code, std::string_view inLanguage) {
    const std::string wanted = modernCode(code);
    const icu::Locale language{wanted.c_str()};
    const icu::Locale reader{inLanguage.empty() ? "en" : std::string{inLanguage}.c_str()};

    icu::UnicodeString name;
    language.getDisplayLanguage(reader, name);

    std::string utf8;
    name.toUTF8String(utf8);
    // ICU answers with the code itself for a language it has no name for.
    return utf8.empty() ? std::string{code} : utf8;
}

} // namespace subedit::core
