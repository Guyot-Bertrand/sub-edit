#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/youtube_sbv_reader.hpp>
#include <subedit/core/format/youtube_sbv_syntax.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/lines.hpp>

#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

std::expected<ReadResult, ReadError> YouTubeSbvReader::read(std::string_view content) const {
    const std::vector<std::string_view> lines = splitLines(content);

    ReadResult result{.format = SubtitleFormat::YouTubeSbv};

    for (std::size_t index = 0; index < lines.size(); ++index) {
        const std::string_view line = lines[index];
        if (isBlank(line))
            continue;

        const std::optional<YouTubeSbvTimeLine> times = parseYouTubeSbvTimeLine(line);
        if (!times.has_value()) {
            // A line outside any block: it belongs to no subtitle, and guessing
            // which one would be inventing. Said, then dropped.
            result.diagnostics.push_back(Diagnostic{
                .severity = Severity::Warning,
                .line = static_cast<int>(index) + 1,
                .kind = DiagnosticKind::IgnoredLine,
                .detail = std::string{trimmedBlanks(line)},
            });
            continue;
        }

        // The text runs to the next blank line, or to the end of the file. A
        // time line followed at once by a blank one is a subtitle with no text.
        std::string text;
        while (index + 1 < lines.size() && !isBlank(lines[index + 1])) {
            ++index;
            if (!text.empty())
                text += '\n';
            text += lines[index];
        }

        result.subtitles.push_back(Subtitle{
            .start = times->start,
            .end = times->end,
            .mainText = std::move(text),
        });
    }

    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = std::string{translate("no timestamp line")},
        });

    return result;
}

} // namespace subedit::core
