#include <subedit/cli/json.hpp>
#include <subedit/cli/reporter.hpp>

#include <ostream>
#include <utility>

namespace subedit::cli {

namespace {

/// Whether the value of `key` is a path, which stays on one line, rather than a sentence, which
/// is cut at its line ends.
bool isPath(Key key) {
    switch (key) {
    case Key::Source:
    case Key::Translation:
    case Key::Media:
    case Key::Dest:
    case Key::Tail:
    case Key::Settings:
        return true;
    default:
        return false;
    }
}

/// `path` on one line: its line ends written `\n` (D1 of the spec).
std::string oneLine(std::string_view path) {
    std::string line;
    line.reserve(path.size());
    for (const char c : path) {
        if (c == '\n') {
            line += "\\n";
        } else {
            line += c;
        }
    }
    return line;
}

} // namespace

std::string_view nameOf(Key key) {
    switch (key) {
    case Key::Source:
        return "source";
    case Key::Translation:
        return "translation";
    case Key::Media:
        return "media";
    case Key::Format:
        return "format";
    case Key::Encoding:
        return "encoding";
    case Key::Bom:
        return "bom";
    case Key::LineEndings:
        return "line-endings";
    case Key::BytesRead:
        return "bytes-read";
    case Key::Diagnostic:
        return "diagnostic";
    case Key::Alignment:
        return "alignment";
    case Key::Progress:
        return "progress";
    case Key::Warning:
        return "warning";
    case Key::Error:
        return "error";
    case Key::Result:
        return "result";
    case Key::BytesWritten:
        return "bytes-written";
    case Key::Dest:
        return "dest";
    case Key::Tail:
        return "tail";
    case Key::Settings:
        return "settings";
    case Key::Walk:
        return "walk";
    case Key::Summary:
        return "summary";
    }
    std::unreachable();
}

Reporter Reporter::withRecords(std::ostream& out) const {
    Reporter copy = *this;
    copy.m_records = &out;
    return copy;
}

Reporter Reporter::withTextOutput(std::ostream& out) const {
    Reporter copy = *this;
    copy.m_text = &out;
    return copy;
}

void Reporter::result(std::string_view text) const {
    if (m_text != nullptr && m_records == nullptr) {
        *m_text << text;
    }
}

Reporter Reporter::forCommand(std::string command) const {
    Reporter copy = *this;
    copy.m_command = std::move(command);
    return copy;
}

Reporter Reporter::forItem(std::size_t position, std::size_t count, std::string source) const {
    Reporter copy = *this;
    copy.m_item = std::make_shared<Item>(Item{
        .position = position, .count = count, .source = std::move(source), .announced = false});
    return copy;
}

void Reporter::tell(int atLeast, Key key, std::string_view value) const {
    if (m_level >= atLeast) {
        write(key, value);
    }
}

void Reporter::error(std::string_view value) const {
    write(Key::Error, value);
}

void Reporter::write(Key key, std::string_view value) const {
    // The element begins with its input, whatever line comes first and at whatever level.
    if (m_item != nullptr && !m_item->announced) {
        m_item->announced = true;
        writeLine(Key::Source, oneLine(m_item->source));
    }
    if (isPath(key)) {
        writeLine(key, oneLine(value));
        return;
    }
    // One line per physical line; the line end that closes the last one opens none.
    std::string_view rest = value;
    do {
        const std::size_t end = rest.find('\n');
        writeLine(key, rest.substr(0, end));
        rest = end == std::string_view::npos ? std::string_view{} : rest.substr(end + 1);
    } while (!rest.empty());
}

void Reporter::writeLine(Key key, std::string_view value) const {
    *m_errors << "[subedit] [" << (m_command.empty() ? std::string_view{"-"} : m_command) << "] [";
    if (m_item != nullptr) {
        *m_errors << m_item->position << '/' << m_item->count;
    } else {
        *m_errors << '-';
    }
    *m_errors << "] " << nameOf(key) << ": " << value << '\n';
}

void Reporter::record(const Json& record) const {
    if (m_records != nullptr) {
        *m_records << record.dump() << '\n';
    }
}

} // namespace subedit::cli
