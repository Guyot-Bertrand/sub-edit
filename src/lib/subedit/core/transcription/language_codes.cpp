#include <subedit/core/transcription/language_codes.hpp>

#include <algorithm>
#include <array>

namespace subedit::core {

namespace {

/// One language: its two-letter code, and the three-letter codes a container uses — the
/// bibliographic and the terminological one differ for some (`fre` and `fra`).
struct Language {
    std::string_view two;
    std::string_view bibliographic;
    std::string_view terminological;
};

constexpr std::array kLanguages{
    Language{"ar", "ara", "ara"}, Language{"ca", "cat", "cat"}, Language{"cs", "cze", "ces"},
    Language{"da", "dan", "dan"}, Language{"de", "ger", "deu"}, Language{"el", "gre", "ell"},
    Language{"en", "eng", "eng"}, Language{"es", "spa", "spa"}, Language{"fi", "fin", "fin"},
    Language{"fr", "fre", "fra"}, Language{"he", "heb", "heb"}, Language{"hu", "hun", "hun"},
    Language{"is", "ice", "isl"}, Language{"it", "ita", "ita"}, Language{"ja", "jpn", "jpn"},
    Language{"ko", "kor", "kor"}, Language{"nl", "dut", "nld"}, Language{"no", "nor", "nor"},
    Language{"pl", "pol", "pol"}, Language{"pt", "por", "por"}, Language{"ro", "rum", "ron"},
    Language{"ru", "rus", "rus"}, Language{"sr", "srp", "srp"}, Language{"sv", "swe", "swe"},
    Language{"tr", "tur", "tur"}, Language{"uk", "ukr", "ukr"}, Language{"zh", "chi", "zho"},
};

[[nodiscard]] std::string lowered(std::string_view text) {
    std::string out{text};
    std::ranges::transform(out, out.begin(), [](unsigned char letter) {
        return static_cast<char>(letter >= 'A' && letter <= 'Z' ? letter - 'A' + 'a' : letter);
    });
    return out;
}

} // namespace

std::optional<std::string> twoLetterCodeOf(std::string_view tag) {
    const std::string wanted = lowered(tag);
    const auto found = std::ranges::find_if(kLanguages, [&wanted](const Language& language) {
        return wanted == language.two || wanted == language.bibliographic ||
               wanted == language.terminological;
    });
    if (found == kLanguages.end())
        return std::nullopt;
    return std::string{found->two};
}

} // namespace subedit::core
