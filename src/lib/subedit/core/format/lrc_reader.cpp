#include <subedit/core/format/deduced_ends.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/lrc_reader.hpp>
#include <subedit/core/format/lrc_syntax.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
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

/// Joins the header lines, dropping the blank ones that trail it.
///
/// **The blank line between the tags and the first lyric belongs to neither**,
/// and writing puts one back of its own — so keeping it here would add one
/// more at every round trip. What sits *between* two header lines is kept, on
/// the other hand: nothing says it is not meant to be there.
[[nodiscard]] std::string headerOf(const std::vector<std::string_view>& lines) {
    std::size_t kept = lines.size();
    while (kept > 0 && isBlank(lines[kept - 1]))
        --kept;

    std::string header;
    for (std::size_t index = 0; index < kept; ++index) {
        if (index > 0)
            header += '\n';
        header += lines[index];
    }
    return header;
}

} // namespace

std::expected<ReadResult, ReadError> LrcReader::read(std::string_view content) const {
    ReadResult result{.format = SubtitleFormat::Lrc};
    std::vector<std::string_view> headerLines;

    int lineNumber = 0;
    for (const std::string_view line : splitLines(content)) {
        ++lineNumber;

        const std::optional<LrcTimeLine> times = parseLrcTimeLine(line);
        if (times.has_value()) {
            result.subtitles.push_back(Subtitle{
                .start = times->start,
                // **No marker to read back**: this format writes one file line
                // per subtitle and has nothing standing for a break, so what
                // comes back is what is there.
                .mainText = std::string{line.substr(times->textAt)},
            });
            continue;
        }

        // **Before the first timed line, anything is the header.** That is
        // where a player's `[ar:…]` and `[ti:…]` tags live, and they are not
        // timed lines however much they look like one.
        if (result.subtitles.empty()) {
            if (!headerLines.empty() || !isBlank(line))
                headerLines.push_back(line);
            continue;
        }

        if (isBlank(line))
            continue;

        // Past that point one line is one subtitle, so a line that is not one
        // belongs to nothing. Gaupol drops it without a word; we say so first.
        result.diagnostics.push_back(Diagnostic{
            .severity = Severity::Warning,
            .line = lineNumber,
            .kind = DiagnosticKind::IgnoredLine,
            .detail = std::string{trimmedBlanks(line)},
        });
    }

    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "no timed line",
        });

    result.header = headerOf(headerLines);
    deduceEnds(result);
    return result;
}

} // namespace subedit::core
