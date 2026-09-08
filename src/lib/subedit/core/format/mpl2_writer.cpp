#include <subedit/core/format/mpl2_syntax.hpp>
#include <subedit/core/format/mpl2_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/break_marker.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

std::string Mpl2Writer::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);

    std::string out;
    for (const Subtitle& subtitle : request.subtitles) {
        out += '[';
        out += tenthsOf(subtitle.start);
        out += "][";
        out += tenthsOf(subtitle.end);
        out += ']';
        out += textToMarker(subtitle.text(request.document), break_marker::kPipe);
        out += ending;
    }
    return out;
}

} // namespace subedit::core
