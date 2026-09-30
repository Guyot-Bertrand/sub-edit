#include <subedit/core/text/line_lengths.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_reader.hpp>

#include <cmath>
#include <string>

namespace subedit::core {

std::vector<int>
lineLengths(const LineMeasure& measure, std::string_view text, MarkupVocabulary vocabulary) {
    // Most subtitles carry no tag at all, and reading one to find that out is
    // an allocation per repaint of a cell.
    const std::string visible = mayHoldMarkup(text, vocabulary)
                                    ? plainTextOf(decodeAs(text, vocabulary).runs)
                                    : std::string{text};

    std::vector<int> lengths;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = visible.find('\n', start);
        const std::string_view line = std::string_view{visible}.substr(
            start, end == std::string::npos ? std::string::npos : end - start);
        lengths.push_back(static_cast<int>(std::floor(measure.lengthOf(line))));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return lengths;
}

} // namespace subedit::core
