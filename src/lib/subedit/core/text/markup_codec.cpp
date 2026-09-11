#include <subedit/core/text/html_markup.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/micro_dvd_markup.hpp>
#include <subedit/core/text/sub_station_alpha_markup.hpp>

#include <string_view>
#include <utility>

namespace subedit::core {

DecodedMarkup decodeAs(std::string_view text, MarkupVocabulary vocabulary) {
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

EncodedMarkup
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

WritableMarkup keepWritable(const StyledText& runs, const StyleAbilities& abilities) {
    WritableMarkup kept;

    for (const StyledRun& run : runs) {
        Style writable = run.style;
        for (const StyleAttribute attribute : kStyleAttributes) {
            if (writable.carries(attribute) && !abilities.can(attribute)) {
                writable = writable.without(attribute);
                ++kept.dropped;
            }
        }
        appendRun(kept.runs, run.text, writable);
    }

    return kept;
}

} // namespace subedit::core
