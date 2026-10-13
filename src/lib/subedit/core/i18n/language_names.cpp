// The languages by their own names — issue #669.
//
// **Every literal here is a language's name in that language**, which is never translated: a
// reader looks down a list of languages for the one they read, written as they write it. The
// file is exempted as a whole in `.translation-exemptions`, under `[language-name]`.

#include <subedit/core/i18n/language.hpp>

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

/// English, the source, and every language `src/po/` holds a catalogue of, installed or seeded.
constexpr std::array<std::pair<std::string_view, std::string_view>, 22> kEndonyms = {{
    {"ca", "Català"},
    {"cs", "Čeština"},
    {"de", "Deutsch"},
    {"en", "English"},
    {"es", "Español"},
    {"fi", "Suomi"},
    {"fr", "Français"},
    {"gl", "Galego"},
    {"he", "עברית"},
    {"hu", "Magyar"},
    {"ie", "Interlingue"},
    {"is", "Íslenska"},
    {"it", "Italiano"},
    {"nl", "Nederlands"},
    {"pl", "Polski"},
    {"pt", "Português"},
    {"pt_BR", "Português do Brasil"},
    {"ru", "Русский"},
    {"sr", "Српски"},
    {"tr", "Türkçe"},
    {"zh", "中文"},
    {"zh_CN", "简体中文"},
}};

[[nodiscard]] std::optional<std::string_view> lookUp(std::string_view language) {
    const auto* const found = std::ranges::find(
        kEndonyms, language, &std::pair<std::string_view, std::string_view>::first);
    if (found == kEndonyms.end())
        return std::nullopt;
    return found->second;
}

} // namespace

std::optional<std::string_view> endonymOf(std::string_view language) {
    if (std::optional<std::string_view> name = lookUp(language))
        return name;
    return lookUp(language.substr(0, language.find('_')));
}

} // namespace subedit::core
