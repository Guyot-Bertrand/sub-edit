#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/html_markup.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_conversion.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/micro_dvd_markup.hpp>
#include <subedit/core/text/sub_station_alpha_markup.hpp>

#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

[[nodiscard]] DecodedMarkup decodeAs(std::string_view text, MarkupVocabulary vocabulary) {
    switch (vocabulary) {
    case MarkupVocabulary::None: {
        // Nothing here is a tag, so nothing here is read as one: a `/` at the
        // head of an LRC line is a slash.
        DecodedMarkup read;
        appendRun(read.runs, text, Style{});
        return read;
    }
    case MarkupVocabulary::Html:
        return decodeHtmlMarkup(text);
    case MarkupVocabulary::SubStationAlpha:
        return decodeSubStationAlphaMarkup(text);
    case MarkupVocabulary::MicroDvd:
        return decodeMicroDvdMarkup(text);
    case MarkupVocabulary::Mpl2:
        return decodeMpl2Markup(text);
    }
    std::unreachable();
}

[[nodiscard]] EncodedMarkup
encodeAs(const StyledText& runs, MarkupVocabulary vocabulary, const StyleAbilities& abilities) {
    switch (vocabulary) {
    case MarkupVocabulary::None: {
        // `keepWritable` does the counting: a format that can say nothing drops
        // everything, and the tally is the same one every other vocabulary
        // produces.
        const WritableMarkup writable = keepWritable(runs, abilities);
        return EncodedMarkup{.text = plainTextOf(writable.runs), .dropped = writable.dropped};
    }
    case MarkupVocabulary::Html:
        return encodeHtmlMarkup(runs, abilities);
    case MarkupVocabulary::SubStationAlpha:
        return encodeSubStationAlphaMarkup(runs, abilities);
    case MarkupVocabulary::MicroDvd:
        return encodeMicroDvdMarkup(runs, abilities);
    case MarkupVocabulary::Mpl2:
        return encodeMpl2Markup(runs, abilities);
    }
    std::unreachable();
}

} // namespace

ConvertedMarkup convertMarkup(std::string_view text, SubtitleFormat from, SubtitleFormat to) {
    if (from == to)
        return ConvertedMarkup{.text = std::string{text}, .dropped = 0};

    const DecodedMarkup read = decodeAs(text, vocabularyOf(from));
    const EncodedMarkup written = encodeAs(read.runs, vocabularyOf(to), abilitiesOf(to));

    return ConvertedMarkup{.text = written.text, .dropped = read.unknown + written.dropped};
}

} // namespace subedit::core
