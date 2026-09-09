#include <subedit/core/format/lrc_syntax.hpp>
#include <subedit/core/format/lrc_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/break_marker.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

namespace {

/// What a line break becomes, having nothing to become.
///
/// **Not a break marker, and named apart from them for that reason.** `[br]`
/// and `|` are read back; a space is not, and a reader meeting one has no way
/// to know a break was ever there. The joining is one way, which is the whole
/// of what this format loses.
constexpr std::string_view kJoin = " ";

} // namespace

std::string LrcWriter::write(const WriteRequest& request) const {
    const std::string_view ending = charactersOf(request.newline);

    std::string out;
    if (!request.header.empty()) {
        appendWithEnding(out, request.header, ending);
        // The header's own last ending, then the blank line that separates it
        // from the lyrics — which is what a reading takes it to end at.
        out += ending;
        out += ending;
    }

    for (const Subtitle& subtitle : request.subtitles) {
        out += '[';
        out += lrcTimeOf(subtitle.start);
        out += ']';
        out += textToMarker(subtitle.text(request.document), kJoin);
        out += ending;
    }
    return out;
}

} // namespace subedit::core
