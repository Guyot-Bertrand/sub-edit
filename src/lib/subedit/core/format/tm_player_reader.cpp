#include <subedit/core/format/deduced_ends.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/tm_player_reader.hpp>
#include <subedit/core/format/tm_player_syntax.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/break_marker.hpp>
#include <subedit/core/text/lines.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

std::expected<ReadResult, ReadError> TMPlayerReader::read(std::string_view content) const {
    ReadResult result{.format = SubtitleFormat::TMPlayer};
    TMPlayerFile shape{};
    bool shapeSeen = false;

    int lineNumber = 0;
    for (const std::string_view line : splitLines(content)) {
        ++lineNumber;
        if (isBlank(line))
            continue;

        const std::optional<TMPlayerTimeLine> times = parseTMPlayerTimeLine(line);
        if (!times.has_value()) {
            // One line is one subtitle, so a line that is not one belongs to no
            // subtitle. Gaupol drops it without a word; we say so first.
            result.diagnostics.push_back(Diagnostic{
                .severity = Severity::Warning,
                .line = lineNumber,
                .kind = DiagnosticKind::IgnoredLine,
                .detail = std::string{trimmedBlanks(line)},
            });
            continue;
        }

        // **The first line settles the shape, not the last.** A file uses one
        // form throughout, which is what makes the question answerable at all;
        // taking the first is what a reader does with everything else a file
        // declares about itself.
        if (!shapeSeen) {
            shape.twoDigitHour = times->twoDigitHour;
            shapeSeen = true;
        }

        result.subtitles.push_back(Subtitle{
            .start = times->start,
            .mainText = textFromMarker(line.substr(times->textAt), break_marker::kPipe),
        });
    }

    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "no timed line",
        });

    result.extras = shape;
    deduceEnds(result);
    return result;
}

} // namespace subedit::core
