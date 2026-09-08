#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/mpl2_reader.hpp>
#include <subedit/core/format/mpl2_syntax.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/break_marker.hpp>
#include <subedit/core/text/lines.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

std::expected<ReadResult, ReadError> Mpl2Reader::read(std::string_view content) const {
    ReadResult result{.format = SubtitleFormat::Mpl2};

    int lineNumber = 0;
    for (const std::string_view line : splitLines(content)) {
        ++lineNumber;
        if (isBlank(line))
            continue;

        const std::optional<Mpl2TimeLine> times = parseMpl2TimeLine(line);
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

        result.subtitles.push_back(Subtitle{
            .start = times->start,
            .end = times->end,
            .mainText = textFromMarker(line.substr(times->textAt), break_marker::kPipe),
        });
    }

    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "no bracketed line",
        });

    return result;
}

} // namespace subedit::core
