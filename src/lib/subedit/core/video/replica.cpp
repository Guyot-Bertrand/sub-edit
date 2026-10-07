#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/video/replica.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

namespace {

/// What the overlay draws of a style: the four the spec names, and not the font or the size.
constexpr StyleAbilities kDrawn{
    .bold = true, .italic = true, .underline = true, .colour = true, .font = false, .size = false};

/// `text` with the braces that would open an override block escaped, so that it is drawn.
[[nodiscard]] std::string withBracesEscaped(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (const char character : text) {
        if (character == '{' || character == '}')
            escaped += '\\';
        escaped += character;
    }
    return escaped;
}

} // namespace

std::string replicaOf(std::string_view text, SubtitleFormat format) {
    DecodedMarkup decoded = decodeAs(text, vocabularyOf(format));
    for (StyledRun& run : decoded.runs)
        run.text = withBracesEscaped(run.text);

    return encodeAs(decoded.runs, MarkupVocabulary::SubStationAlpha, kDrawn).text;
}

} // namespace subedit::core
