#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_conversion.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

ConvertedMarkup convertMarkup(std::string_view text, SubtitleFormat from, SubtitleFormat to) {
    if (from == to)
        return ConvertedMarkup{.text = std::string{text}, .dropped = 0};

    const DecodedMarkup read = decodeAs(text, vocabularyOf(from));
    const EncodedMarkup written = encodeAs(read.runs, vocabularyOf(to), abilitiesOf(to));

    return ConvertedMarkup{.text = written.text, .dropped = read.unknown + written.dropped};
}

} // namespace subedit::core
