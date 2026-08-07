#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/lines.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/format/web_vtt_reader.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kSignature = "WEBVTT";
constexpr std::string_view kArrow = "-->";
constexpr std::string_view kStyleKeyword = "STYLE";
constexpr std::string_view kNoteKeyword = "NOTE";
constexpr std::string_view kBlanks = " \t";

/// What a cue's timestamp line says.
struct TimeLine {
    Timestamp start;
    Timestamp end;
    std::string settings;
};

/// Tells whether `line` opens a block introduced by `keyword`.
///
/// The keyword stands alone or is followed by a blank, so that a cue
/// identifier named `NOTES` is not taken for a comment.
[[nodiscard]] bool opensBlock(std::string_view line, std::string_view keyword) {
    const std::string_view text = trimmedBlanks(line);
    if (!text.starts_with(keyword))
        return false;
    return text.size() == keyword.size() || kBlanks.contains(text[keyword.size()]);
}

/// Reads `00:01.000 --> 00:02.000 align:start`, settings included.
[[nodiscard]] std::optional<TimeLine> parseTimeLine(std::string_view line) {
    const std::size_t arrow = line.find(kArrow);
    if (arrow == std::string_view::npos)
        return std::nullopt;

    const std::optional<Timestamp> start = Timestamp::parse(line.substr(0, arrow));
    if (!start.has_value())
        return std::nullopt;

    const std::string_view rest = trimmedBlanks(line.substr(arrow + kArrow.size()));
    const std::size_t endOfStamp = rest.find_first_of(kBlanks);
    const std::optional<Timestamp> end = Timestamp::parse(rest.substr(0, endOfStamp));
    if (!end.has_value())
        return std::nullopt;

    std::string settings;
    if (endOfStamp != std::string_view::npos)
        settings = std::string{trimmedBlanks(rest.substr(endOfStamp))};

    return TimeLine{.start = *start, .end = *end, .settings = std::move(settings)};
}

/// Appends `line` to `block`, keeping the line feeds of a multi-line block.
void appendLine(std::string& block, std::string_view line) {
    if (!block.empty())
        block += '\n';
    block += line;
}

/// The state carried while walking the file.
///
/// A block-oriented format read line by line: what a line means depends on the
/// block it belongs to, and a blank line closes that block.
class Parser {

public:
    void feed(std::string_view line, int lineNumber) {
        if (isBlank(line)) {
            closeBlock();
            return;
        }

        switch (m_state) {
        case State::Header:
            appendLine(m_result.header, line);
            return;
        case State::Style:
            appendLine(m_pendingStyle, line);
            return;
        case State::Comment:
            appendLine(m_pendingComment, line);
            return;
        // Outside a block, a line is judged on what it is rather than on
        // what came before it.
        case State::Text:
        case State::Idle:
            feedOutsideBlock(line, lineNumber);
            return;
        }
    }

    /// Closes whatever is still open at the end of the file.
    [[nodiscard]] ReadResult finish() {
        closeBlock();
        return std::move(m_result);
    }

private:
    enum class State { Header, Idle, Style, Comment, Text };

    /// Handles a line that is not inside a style, a comment or a header.
    void feedOutsideBlock(std::string_view line, int lineNumber) {
        if (const std::optional<TimeLine> timeLine = parseTimeLine(line)) {
            openCue(*timeLine, lineNumber);
            return;
        }

        if (line.find(kArrow) != std::string_view::npos) {
            // It wanted to be a timestamp line and failed. Reported, then kept
            // as it stands — the same choice as the SubRip reader, for the
            // same reason: destroying a line we did not understand would be
            // worse than showing it to the user.
            report(Severity::Warning,
                   lineNumber,
                   DiagnosticKind::MalformedTimestamp,
                   std::string{trimmedBlanks(line)});
        }

        if (m_state == State::Text) {
            appendLine(m_currentText, line);
            return;
        }

        if (opensBlock(line, kStyleKeyword)) {
            m_state = State::Style;
            appendLine(m_pendingStyle, line);
            return;
        }

        if (opensBlock(line, kNoteKeyword)) {
            m_state = State::Comment;
            appendLine(m_pendingComment, line);
            return;
        }

        // Anything else standing on its own is either the identifier of the
        // cue about to start, or a block we do not know. Which of the two it
        // was is only settled by what comes next.
        if (m_pendingId.has_value())
            reportUnknownBlock();
        m_pendingId = std::string{trimmedBlanks(line)};
        m_pendingIdLine = lineNumber;
    }

    void openCue(const TimeLine& timeLine, int lineNumber) {
        closeCue();

        if (timeLine.end < timeLine.start)
            report(Severity::Warning, lineNumber, DiagnosticKind::EndBeforeStart);

        if (m_previousEnd.has_value() && timeLine.start < *m_previousEnd)
            report(Severity::Warning, lineNumber, DiagnosticKind::OverlappingSubtitles);
        m_previousEnd = timeLine.end;

        m_current = Subtitle{
            .start = timeLine.start,
            .end = timeLine.end,
            .extras =
                WebVttExtras{
                    .id = m_pendingId.value_or(std::string{}),
                    .settings = timeLine.settings,
                    .style = std::move(m_pendingStyle),
                    .comment = std::move(m_pendingComment),
                },
        };
        m_pendingId.reset();
        m_pendingStyle.clear();
        m_pendingComment.clear();
        m_currentText.clear();
        m_state = State::Text;
    }

    /// A blank line ends whatever block was open.
    void closeBlock() {
        if (m_state == State::Idle && m_pendingId.has_value())
            reportUnknownBlock();

        if (m_state == State::Text)
            closeCue();

        m_state = State::Idle;
    }

    void closeCue() {
        if (!m_current.has_value())
            return;

        m_current->mainText = std::move(m_currentText);
        m_result.subtitles.push_back(*std::move(m_current));
        m_current.reset();
        m_currentText.clear();
    }

    /// A line that looked like a cue identifier and was followed by no
    /// timestamps was in fact a block of a kind we do not handle — `REGION`,
    /// or whatever a later revision of the format adds.
    void reportUnknownBlock() {
        report(Severity::Warning,
               m_pendingIdLine,
               DiagnosticKind::UnknownBlock,
               m_pendingId.value_or(std::string{}));
        m_pendingId.reset();
    }

    void report(Severity severity, int line, DiagnosticKind kind, std::string detail = {}) {
        m_result.diagnostics.push_back(Diagnostic{
            .severity = severity,
            .line = line,
            .kind = kind,
            .detail = std::move(detail),
        });
    }

    ReadResult m_result{.format = SubtitleFormat::WebVtt};
    State m_state = State::Header;

    std::optional<Subtitle> m_current;
    std::string m_currentText;
    std::optional<Timestamp> m_previousEnd;

    std::optional<std::string> m_pendingId;
    int m_pendingIdLine = 0;
    std::string m_pendingStyle;
    std::string m_pendingComment;
};

} // namespace

std::expected<ReadResult, ReadError> WebVttReader::read(std::string_view content) const {
    const std::vector<std::string_view> lines = splitLines(content);

    // The signature comes first, blank lines aside. Without it, this is not a
    // WebVTT file, and saying so is more useful than reading it at a guess.
    const auto firstText = std::ranges::find_if_not(lines, isBlank);
    if (firstText == lines.end() || !trimmedBlanks(*firstText).starts_with(kSignature))
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::UnknownFormat,
            .detail = "signature WEBVTT absente",
        });

    Parser parser;
    int lineNumber = 0;
    for (const std::string_view line : lines) {
        ++lineNumber;
        parser.feed(line, lineNumber);
    }

    ReadResult result = parser.finish();
    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "aucune cue",
        });

    return result;
}

} // namespace subedit::core
