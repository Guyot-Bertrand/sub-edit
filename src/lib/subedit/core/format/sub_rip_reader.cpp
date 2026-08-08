#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/lines.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_rip_reader.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kArrow = "-->";
constexpr std::string_view kBlanks = " \t";
constexpr std::string_view kDigits = "0123456789";
constexpr int kDecimalBase = 10;

/// Wide enough for any numbering a real file carries, narrow enough that the
/// accumulation below cannot overflow an `int`.
constexpr std::size_t kMaxNumberingDigits = 9;

/// What a timestamp line says.
struct TimeLine {
    Timestamp start;
    Timestamp end;
    std::optional<Rectangle> coordinates;
};

/// Reads `X1:040 X2:600 Y1:020 Y2:460`, or nothing if that is not what follows.
[[nodiscard]] std::optional<Rectangle> parseCoordinates(std::string_view text) {
    static constexpr std::array<std::string_view, 4> kLabels = {"X1:", "X2:", "Y1:", "Y2:"};
    std::array<int, 4> values{};

    std::string_view rest = trimmedBlanks(text);
    for (std::size_t field = 0; field < kLabels.size(); ++field) {
        if (!rest.starts_with(kLabels[field]))
            return std::nullopt;
        rest.remove_prefix(kLabels[field].size());

        const std::size_t digits = rest.find_first_not_of(kDigits);
        const std::string_view number = rest.substr(0, digits);
        if (number.empty())
            return std::nullopt;

        int value = 0;
        for (const char digit : number)
            value = (value * kDecimalBase) + (digit - '0');
        values[field] = value;

        rest = trimmedBlanks(rest.substr(number.size()));
    }

    if (!rest.empty())
        return std::nullopt;

    return Rectangle{
        .x1 = values[0],
        .x2 = values[1],
        .y1 = values[2],
        .y2 = values[3],
    };
}

/// Reads `00:00:01,000 --> 00:00:02,000` and whatever coordinates follow.
///
/// Answers nothing when the line is not a timestamp line at all — which is
/// what tells text apart from structure.
[[nodiscard]] std::optional<TimeLine> parseTimeLine(std::string_view line) {
    const std::size_t arrow = line.find(kArrow);
    if (arrow == std::string_view::npos)
        return std::nullopt;

    const std::optional<Timestamp> start = Timestamp::parse(line.substr(0, arrow));
    if (!start.has_value())
        return std::nullopt;

    const std::string_view rest = trimmedBlanks(line.substr(arrow + kArrow.size()));

    // The end timestamp runs up to the first blank; anything after it is the
    // extended coordinates, which not every file carries.
    const std::size_t endOfStamp = rest.find_first_of(kBlanks);
    const std::optional<Timestamp> end = Timestamp::parse(rest.substr(0, endOfStamp));
    if (!end.has_value())
        return std::nullopt;

    std::optional<Rectangle> coordinates;
    if (endOfStamp != std::string_view::npos) {
        const std::string_view tail = trimmedBlanks(rest.substr(endOfStamp));
        if (!tail.empty()) {
            coordinates = parseCoordinates(tail);
            if (!coordinates.has_value())
                return std::nullopt;
        }
    }

    return TimeLine{.start = *start, .end = *end, .coordinates = coordinates};
}

/// Reads a line holding nothing but a number, the SubRip numbering.
[[nodiscard]] std::optional<int> parseNumbering(std::string_view line) {
    const std::string_view text = trimmedBlanks(line);
    if (text.empty() || text.size() > kMaxNumberingDigits)
        return std::nullopt;

    int value = 0;
    for (const char character : text) {
        if (character < '0' || character > '9')
            return std::nullopt;
        value = (value * kDecimalBase) + (character - '0');
    }
    return value;
}

/// One line of the file, with the number an editor would show for it.
struct NumberedLine {
    std::string_view text;
    int number = 0;
};

/// Joins the pending lines into the text of a subtitle, dropping the blank
/// lines that only separated it from the next block.
[[nodiscard]] std::string joinText(const std::vector<NumberedLine>& pending) {
    std::size_t last = pending.size();
    while (last > 0 && trimmedBlanks(pending[last - 1].text).empty())
        --last;

    std::string text;
    for (std::size_t index = 0; index < last; ++index) {
        if (index > 0)
            text += '\n';
        text += pending[index].text;
    }
    return text;
}

/// The state carried while walking the file.
class Parser {

public:
    void feed(std::string_view line, int lineNumber) {
        if (const std::optional<TimeLine> timeLine = parseTimeLine(line)) {
            openSubtitle(*timeLine, lineNumber);
            return;
        }

        if (line.find(kArrow) != std::string_view::npos) {
            // It wanted to be a timestamp line and failed. Report it, and keep
            // it: destroying a line we did not understand would be worse than
            // showing it to the user as it was.
            m_result.diagnostics.push_back(Diagnostic{
                .severity = Severity::Warning,
                .line = lineNumber,
                .kind = DiagnosticKind::MalformedTimestamp,
                .detail = std::string{trimmedBlanks(line)},
            });
        }

        m_pending.push_back(NumberedLine{.text = line, .number = lineNumber});
    }

    /// Closes the subtitle still open, if any, and hands the result over.
    [[nodiscard]] ReadResult finish() {
        closeSubtitle();
        return std::move(m_result);
    }

private:
    void openSubtitle(const TimeLine& timeLine, int lineNumber) {
        // The numbering is the last non-blank line before the timestamp, which
        // is why it is taken from the pending lines rather than looked for
        // ahead: it belongs to the subtitle that starts, not to the one that
        // ends.
        const std::optional<int> numbering = takeTrailingNumbering();

        closeSubtitle();
        reportBefore();

        if (!numbering.has_value())
            report(Severity::Recovered, lineNumber, DiagnosticKind::MissingNumbering);
        else if (*numbering != m_expectedNumbering)
            report(Severity::Recovered,
                   lineNumber,
                   DiagnosticKind::InconsistentNumbering,
                   std::to_string(*numbering));
        ++m_expectedNumbering;

        if (timeLine.end < timeLine.start)
            report(Severity::Warning, lineNumber, DiagnosticKind::EndBeforeStart);

        if (m_previousEnd.has_value() && timeLine.start < *m_previousEnd)
            report(Severity::Warning, lineNumber, DiagnosticKind::OverlappingSubtitles);
        m_previousEnd = timeLine.end;

        // Reported on top of the overlap, not instead of it: a subtitle that
        // starts before the previous one started also starts before it ended,
        // and the two say different things. The order is what a sort would
        // change; the overlap is what a duration would.
        if (m_previousStart.has_value() && timeLine.start < *m_previousStart)
            report(Severity::Warning, lineNumber, DiagnosticKind::OutOfOrder);
        m_previousStart = timeLine.start;

        m_current = Subtitle{
            .start = timeLine.start,
            .end = timeLine.end,
            .extras = SubRipExtras{.coordinates = timeLine.coordinates},
        };
        m_pending.clear();
    }

    /// Removes the trailing numbering from the pending lines, and the blank
    /// lines that separated it from the previous block.
    [[nodiscard]] std::optional<int> takeTrailingNumbering() {
        std::size_t last = m_pending.size();
        while (last > 0 && trimmedBlanks(m_pending[last - 1].text).empty())
            --last;

        if (last == 0)
            return std::nullopt;

        const std::optional<int> numbering = parseNumbering(m_pending[last - 1].text);
        if (!numbering.has_value())
            return std::nullopt;

        m_pending.resize(last - 1);
        return numbering;
    }

    void closeSubtitle() {
        if (!m_current.has_value())
            return;

        m_current->mainText = joinText(m_pending);
        m_result.subtitles.push_back(*std::move(m_current));
        m_current.reset();
        m_pending.clear();
    }

    /// Reports the lines standing before the first timestamp of the file.
    void reportBefore() {
        for (const NumberedLine& line : m_pending) {
            if (trimmedBlanks(line.text).empty())
                continue;
            report(Severity::Warning,
                   line.number,
                   DiagnosticKind::TextBeforeAnyTimestamp,
                   std::string{trimmedBlanks(line.text)});
        }
    }

    void report(Severity severity, int line, DiagnosticKind kind, std::string detail = {}) {
        m_result.diagnostics.push_back(Diagnostic{
            .severity = severity,
            .line = line,
            .kind = kind,
            .detail = std::move(detail),
        });
    }

    ReadResult m_result{.format = SubtitleFormat::SubRip};
    std::vector<NumberedLine> m_pending;
    std::optional<Subtitle> m_current;
    std::optional<Timestamp> m_previousEnd;
    std::optional<Timestamp> m_previousStart;
    int m_expectedNumbering = 1;
};

} // namespace

std::expected<ReadResult, ReadError> SubRipReader::read(std::string_view content) const {
    Parser parser;

    int lineNumber = 0;
    for (const std::string_view line : splitLines(content)) {
        ++lineNumber;
        parser.feed(line, lineNumber);
    }

    // The last subtitle is only closed here, so the count is only known once
    // the walk is over.
    ReadResult result = parser.finish();
    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "aucune ligne d'horodatage",
        });

    return result;
}

} // namespace subedit::core
