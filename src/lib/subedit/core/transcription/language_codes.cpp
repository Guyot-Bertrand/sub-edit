#include <subedit/core/transcription/language_codes.hpp>

#include <algorithm>
#include <array>
#include <cstddef>

namespace subedit::core {

namespace {

/// One language: its two-letter code, and the three-letter codes a container uses — the
/// bibliographic and the terminological one differ for some (`fre` and `fra`).
using Language = std::array<std::string_view, 3>;

constexpr std::size_t kTwoLetters = 0;
constexpr std::size_t kBibliographic = 1;
constexpr std::size_t kTerminological = 2;

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
    const auto* const found = std::ranges::find_if(kLanguages, [&wanted](const Language& language) {
        return wanted == language[kTwoLetters] || wanted == language[kBibliographic] ||
               wanted == language[kTerminological];
    });
    if (found == kLanguages.end())
        return std::nullopt;
    return std::string{(*found)[kTwoLetters]};
}

} // namespace subedit::core
