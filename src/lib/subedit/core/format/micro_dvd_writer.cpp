#include <subedit/core/format/micro_dvd_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/break_marker.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <string>
#include <string_view>
#include <variant>

namespace subedit::core {

namespace {

/// The rate the document was read at, or the declared default.
[[nodiscard]] FrameRate rateOf(const WriteRequest& request) {
    const MicroDvdFile* file = std::get_if<MicroDvdFile>(&request.extras);
    return file != nullptr ? file->rate : MicroDvdFile{}.rate;
}

} // namespace

std::string MicroDvdWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);
    const FrameRate rate = rateOf(request);

    std::string out;
    if (!request.header.empty()) {
        appendWithEnding(out, request.header, ending);
        out += ending;
    }

    for (const Subtitle& subtitle : request.subtitles) {
        out += '{';
        out += std::to_string(subtitle.start.toFrame(rate).number());
        out += "}{";
        out += std::to_string(subtitle.end.toFrame(rate).number());
        out += '}';
        out += textToMarker(subtitle.text(request.document), break_marker::kPipe);
        out += ending;
    }
    return out;
}

} // namespace subedit::core
