#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_station_alpha_reader.hpp>
#include <subedit/core/format/sub_station_alpha_syntax.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/lines.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

using namespace event_field;

constexpr std::string_view kEventsSection = "[Events]";
constexpr std::string_view kFormatKeyword = "Format:";
constexpr std::string_view kDialogueKeyword = "Dialogue:";
constexpr int kDecimalBase = 10;

/// Wide enough for any margin or layer a real file carries, narrow enough that
/// the accumulation below cannot overflow an `int`.
constexpr std::size_t kMaxNumberDigits = 9;

/// Reads a whole number, or nothing when the column holds something else.
///
/// A margin that is not a number is a file saying something we cannot use, and
/// answering zero would be inventing. The caller reports it.
[[nodiscard]] std::optional<int> parseWholeNumber(std::string_view text) {
    const std::string_view digits = trimmedBlanks(text);
    if (digits.empty())
        return std::nullopt;

    const bool negative = digits.starts_with('-');
    const std::string_view rest = negative ? digits.substr(1) : digits;
    if (rest.empty() || rest.size() > kMaxNumberDigits)
        return std::nullopt;

    int value = 0;
    for (const char character : rest) {
        if (character < '0' || character > '9')
            return std::nullopt;
        value = (value * kDecimalBase) + (character - '0');
    }
    return negative ? -value : value;
}

/// Reads `Marked=1`, whose value is what follows the sign.
[[nodiscard]] std::optional<int> parseMarked(std::string_view text) {
    const std::size_t equals = text.rfind('=');
    return parseWholeNumber(equals == std::string_view::npos ? text : text.substr(equals + 1));
}

/// What one `Dialogue:` line builds, before it is known to be complete.
struct Event {
    Subtitle subtitle{};
    SubStationAlphaExtras extras{};
    bool hasStart = false;
    bool hasEnd = false;
};

/// The state carried while walking the file.
class Parser {

public:
    explicit Parser(SubtitleFormat format) : m_result{.format = format} {}

    void feed(std::string_view line, int lineNumber) {
        const std::string_view text = trimmedBlanks(line);

        if (!m_inEvents) {
            if (text == kEventsSection) {
                m_inEvents = true;
                return;
            }
            appendToHeader(line);
            return;
        }

        if (text.starts_with(kFormatKeyword)) {
            readFormat(text.substr(kFormatKeyword.size()), lineNumber);
            return;
        }
        if (text.starts_with(kDialogueKeyword)) {
            readDialogue(text.substr(kDialogueKeyword.size()), lineNumber);
            return;
        }
        if (!text.empty())
            report(lineNumber, DiagnosticKind::IgnoredLine, std::string{text});
    }

    [[nodiscard]] ReadResult finish() {
        // A file whose `[Events]` section never declared its columns is read
        // under the ones its format uses by default — which is what Gaupol
        // would have crashed on, and what a truncated file looks like.
        if (m_fields.empty())
            m_fields = defaultEventFields(m_result.format);

        m_result.extras = SubStationAlphaFile{.eventFields = std::move(m_fields)};
        m_result.header = trimmedHeader();
        return std::move(m_result);
    }

private:
    void appendToHeader(std::string_view line) {
        if (!m_header.empty())
            m_header += '\n';
        m_header += line;
    }

    /// Drops what the writing puts back on its own: the blank line before
    /// `[Events]`.
    ///
    /// **Only at the end.** Gaupol strips both ends, and the front one is worse
    /// than useless here: an empty opening line never reaches the header —
    /// nothing is appended for it — so the only thing a leading trim could
    /// remove is a line of spaces someone wrote on purpose, and removing it
    /// would break the round trip it was meant to protect.
    [[nodiscard]] std::string trimmedHeader() const {
        std::size_t last = m_header.size();
        while (last > 0 && (m_header[last - 1] == '\n' || m_header[last - 1] == ' '))
            --last;
        return m_header.substr(0, last);
    }

    void readFormat(std::string_view names, int lineNumber) {
        m_fields.clear();
        std::string_view rest = names;
        while (!rest.empty()) {
            const std::size_t comma = rest.find(',');
            const std::string_view name = trimmedBlanks(rest.substr(0, comma));
            if (isKnownEventField(name))
                m_fields.emplace_back(name);
            else if (!name.empty())
                // Declared and dropped: what is written back has to name the
                // columns it fills, and a column nothing could read is one
                // nothing can fill.
                report(lineNumber, DiagnosticKind::UnknownEventField, std::string{name});

            if (comma == std::string_view::npos)
                break;
            rest = rest.substr(comma + 1);
        }
    }

    void readDialogue(std::string_view values, int lineNumber) {
        if (m_fields.empty())
            m_fields = defaultEventFields(m_result.format);

        const std::vector<std::string_view> columns = splitEventLine(values, m_fields.size());
        Event event;
        for (std::size_t index = 0; index < columns.size() && index < m_fields.size(); ++index)
            decode(m_fields[index], columns[index], event, lineNumber);

        // **Without both positions there is no subtitle**, only a line that
        // looked like one. Reporting it beats placing it at the origin, where
        // it would sit at the top of the table for no reason anyone could see.
        if (!event.hasStart || !event.hasEnd) {
            report(lineNumber, DiagnosticKind::MalformedTimestamp, std::string{values});
            return;
        }

        event.subtitle.extras = event.extras;
        m_result.subtitles.push_back(std::move(event.subtitle));
    }

    void decode(std::string_view field, std::string_view value, Event& event, int lineNumber) {
        if (field == kStart || field == kEnd) {
            const std::optional<Timestamp> position = Timestamp::parse(value);
            if (!position.has_value())
                return;
            (field == kStart ? event.subtitle.start : event.subtitle.end) = *position;
            (field == kStart ? event.hasStart : event.hasEnd) = true;
            return;
        }
        if (field == kText) {
            event.subtitle.mainText = textFromEvent(value);
            return;
        }
        if (field == kStyle) {
            event.extras.style = std::string{value};
            return;
        }
        if (field == kName) {
            event.extras.name = std::string{value};
            return;
        }
        if (field == kEffect) {
            event.extras.effect = std::string{value};
            return;
        }

        const std::optional<int> number =
            field == kMarked ? parseMarked(value) : parseWholeNumber(value);
        if (!number.has_value()) {
            report(lineNumber, DiagnosticKind::UnknownEventField, std::string{field});
            return;
        }
        if (field == kMarked)
            event.extras.marked = *number;
        else if (field == kLayer)
            event.extras.layer = *number;
        else if (field == kMarginLeft)
            event.extras.marginLeft = *number;
        else if (field == kMarginRight)
            event.extras.marginRight = *number;
        else
            event.extras.marginVertical = *number;
    }

    void report(int line, DiagnosticKind kind, std::string detail) {
        m_result.diagnostics.push_back(Diagnostic{
            .severity = Severity::Warning,
            .line = line,
            .kind = kind,
            .detail = std::move(detail),
        });
    }

    ReadResult m_result;
    std::string m_header;
    std::vector<std::string> m_fields;
    bool m_inEvents = false;
};

} // namespace

std::expected<ReadResult, ReadError> SubStationAlphaReader::read(std::string_view content) const {
    Parser parser{m_format};

    int lineNumber = 0;
    for (const std::string_view line : splitLines(content)) {
        ++lineNumber;
        parser.feed(line, lineNumber);
    }

    ReadResult result = parser.finish();
    if (result.subtitles.empty())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::NoSubtitleFound,
            .detail = "no dialogue line",
        });

    return result;
}

} // namespace subedit::core
