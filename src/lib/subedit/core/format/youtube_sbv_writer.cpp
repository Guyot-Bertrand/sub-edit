#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/youtube_sbv_writer.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

std::string YouTubeSbvWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);

    std::string out;
    bool first = true;
    for (const Subtitle& subtitle : request.subtitles) {
        // The blank line separates two blocks and does not close the last one.
        if (!first)
            out += ending;
        first = false;

        out +=
            subtitle.start.format(DecimalMark::Period, HourField::Unpadded, Decimals::Milliseconds);
        out += ',';
        out +=
            subtitle.end.format(DecimalMark::Period, HourField::Unpadded, Decimals::Milliseconds);
        out += ending;

        appendWithEnding(out, subtitle.text(request.document), ending);
        out += ending;
    }
    return out;
}

} // namespace subedit::core
