#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/micro_dvd_reader.hpp>
#include <subedit/core/format/micro_dvd_syntax.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/break_marker.hpp>
#include <subedit/core/text/lines.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

std::expected<ReadResult, ReadError> MicroDvdReader::read(std::string_view content) const {
    ReadResult result{.format = SubtitleFormat::MicroDvd};
    result.extras = MicroDvdFile{.rate = m_rate};

    int lineNumber = 0;
    for (const std::string_view line : splitLines(content)) {
        ++lineNumber;
        if (isBlank(line))
            continue;

        // The header is one line and it comes first, so it is recognised before
        // anything else: `{DEFAULT}` opens on a brace like every other line.
        if (isMicroDvdHeader(line)) {
            result.header = std::string{line};
            continue;
        }

        const std::optional<MicroDvdFrameLine> frames = parseMicroDvdFrameLine(line);
        if (!frames.has_value()) {
            result.diagnostics.push_back(Diagnostic{
                .severity = Severity::Warning,
                .line = lineNumber,
                .kind = DiagnosticKind::IgnoredLine,
                .detail = std::string{trimmedBlanks(line)},
            });
            continue;
        }

        result.subtitles.push_back(Subtitle{
            .start = Timestamp::fromFrame(frames->start, m_rate),
            .end = Timestamp::fromFrame(frames->end, m_rate),
            .mainText = textFromMarker(line.substr(frames->textAt), break_marker::kPipe),
        });
    }

    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "no braced line",
        });

    // **Said only when nobody said it.** A rate the caller chose is not news;
    // one the tool picked is the single place where every position on screen
    // rests on an assumption, and the file cannot confirm it.
    if (!m_rateWasChosen)
        result.diagnostics.insert(result.diagnostics.begin(),
                                  Diagnostic{
                                      .severity = Severity::Recovered,
                                      .line = kWholeFile,
                                      .kind = DiagnosticKind::AssumedFrameRate,
                                      .detail = nameOf(m_rate),
                                  });

    return result;
}

} // namespace subedit::core
