#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_viewer2_reader.hpp>
#include <subedit/core/format/sub_viewer2_syntax.hpp>
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

namespace {

/// Takes the bracketed lines that open the file, and returns where they end.
///
/// **A text line could start with a bracket** — a hearing-impaired mention does
/// — and that is not a risk here: the header only ever runs until the first
/// line that is not bracketed, and what follows a header is a blank line or a
/// timestamp line. Neither starts with one.
[[nodiscard]] std::size_t takeHeader(const std::vector<std::string_view>& lines,
                                     std::string& header) {
    std::size_t index = 0;
    while (index < lines.size() && lines[index].starts_with('[')) {
        if (index > 0)
            header += '\n';
        header += lines[index];
        ++index;
    }
    return index;
}

} // namespace

std::expected<ReadResult, ReadError> SubViewer2Reader::read(std::string_view content) const {
    const std::vector<std::string_view> lines = splitLines(content);

    ReadResult result{.format = SubtitleFormat::SubViewer2};
    std::size_t index = takeHeader(lines, result.header);

    for (; index < lines.size(); ++index) {
        const std::string_view line = lines[index];
        if (isBlank(line))
            continue;

        const std::optional<SubViewer2TimeLine> times = parseSubViewer2TimeLine(line);
        if (!times.has_value()) {
            // A line that is neither a timestamp nor the text of one. Gaupol
            // drops it; we say so, and drop it too — keeping it would mean
            // guessing which subtitle it belongs to, and there is no answer.
            result.diagnostics.push_back(Diagnostic{
                .severity = Severity::Warning,
                .line = static_cast<int>(index) + 1,
                .kind = DiagnosticKind::IgnoredLine,
                .detail = std::string{trimmedBlanks(line)},
            });
            continue;
        }

        // The text is the next line, whatever it holds — a blank one included,
        // which is how a subtitle with no text is written. A timestamp line
        // closing the file leaves an empty text rather than an exception, which
        // is what Gaupol raises there.
        std::string text;
        if (index + 1 < lines.size()) {
            ++index;
            text = textFromSubViewer2(lines[index]);
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
            .detail = "no timestamp line",
        });

    return result;
}

} // namespace subedit::core
