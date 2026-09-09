#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/tm_player_syntax.hpp>
#include <subedit/core/format/tm_player_writer.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/break_marker.hpp>

#include <string>
#include <string_view>
#include <variant>

namespace subedit::core {

namespace {

/// The hour shape the document's file had, or the commoner of the two.
[[nodiscard]] bool twoDigitHourOf(const WriteRequest& request) {
    const TMPlayerFile* file = std::get_if<TMPlayerFile>(&request.extras);
    return file != nullptr ? file->twoDigitHour : TMPlayerFile{}.twoDigitHour;
}

} // namespace

std::string TMPlayerWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);
    const bool twoDigitHour = twoDigitHourOf(request);

    std::string out;
    for (const Subtitle& subtitle : request.subtitles) {
        out += tmPlayerTimeOf(subtitle.start, twoDigitHour);
        out += ':';
        out += textToMarker(subtitle.text(request.document), break_marker::kPipe);
        out += ending;
    }
    return out;
}

} // namespace subedit::core
