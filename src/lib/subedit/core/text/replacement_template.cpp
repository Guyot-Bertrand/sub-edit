#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/replacement_template.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

constexpr unsigned kOctalBase = 8;
constexpr int kDecimalBase = 10;

[[nodiscard]] bool isDigit(char letter) {
    return letter >= '0' && letter <= '9';
}

[[nodiscard]] bool isOctal(char letter) {
    return letter >= '0' && letter <= '7';
}

[[nodiscard]] bool isAsciiLetter(char letter) {
    return (letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z');
}

/// Python's `chr(code)`, written as UTF-8. The escape reaches 0xFF at most.
[[nodiscard]] std::string characterOf(unsigned code) {
    constexpr unsigned kAsciiEnd = 0x80;
    constexpr unsigned kTwoBytes = 0xC0;
    constexpr unsigned kContinuation = 0x80;
    constexpr unsigned kSixBits = 0x3F;
    constexpr unsigned kShift = 6;
    std::string out;
    if (code < kAsciiEnd) {
        // `+=` of a char, and not a braced list: `{1, c}` would be two characters.
        out += static_cast<char>(code);
        return out;
    }
    out += static_cast<char>(kTwoBytes | (code >> kShift));
    out += static_cast<char>(kContinuation | (code & kSixBits));
    return out;
}

/// The single-character escapes of `sre_parse.ESCAPES`.
[[nodiscard]] std::optional<char> simpleEscape(char letter) {
    switch (letter) {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '\\':
        return '\\';
    default:
        return std::nullopt;
    }
}

} // namespace

/// One reading of a template, with the pieces it has made so far.
class TemplateReading {

public:
    TemplateReading(std::string_view text,
                    std::size_t groupCount,
                    const std::function<std::optional<int>(std::string_view)>& groupNamed)
        : m_text(text), m_groupCount(groupCount), m_groupNamed(groupNamed) {}

    [[nodiscard]] std::expected<ReplacementTemplate, TemplateError> read() {
        while (m_at < m_text.size()) {
            const char here = m_text[m_at++];
            if (here != '\\') {
                literal(std::string(1, here));
            } else if (const std::optional<TemplateError> refused = escape()) {
                return std::unexpected{*refused};
            }
        }
        return ReplacementTemplate{std::move(m_result)};
    }

private:
    [[nodiscard]] static std::optional<TemplateError> fail(std::string reason) {
        return TemplateError{.reason = std::move(reason)};
    }

    void literal(const std::string& text) {
        if (!m_result.m_pieces.empty() && m_result.m_pieces.back().group < 0) {
            m_result.m_pieces.back().literal += text;
        } else {
            m_result.m_pieces.push_back({.group = -1, .literal = text});
        }
    }

    [[nodiscard]] std::optional<TemplateError> group(int number) {
        if (number < 0 || std::cmp_greater(number, m_groupCount))
            return fail("invalid group reference " + std::to_string(number));
        m_result.m_pieces.push_back({.group = number, .literal = {}});
        return std::nullopt;
    }

    /// The escape after a backslash, which `m_at` stands on.
    [[nodiscard]] std::optional<TemplateError> escape() {
        if (m_at == m_text.size())
            return fail("a backslash ends the replacement");
        const char letter = m_text[m_at++];

        if (const std::optional<char> simple = simpleEscape(letter)) {
            literal(std::string(1, *simple));
            return std::nullopt;
        }
        if (letter == 'g')
            return namedGroup();
        if (letter == '0')
            return octalAfterZero();
        if (isDigit(letter))
            return digits(letter);
        if (isAsciiLetter(letter))
            return fail(std::string{"bad escape \\"} + letter);
        // Anything else stays as written, backslash included.
        literal(std::string{'\\'} + letter);
        return std::nullopt;
    }

    /// `\0`, and up to two more octal digits: a character.
    [[nodiscard]] std::optional<TemplateError> octalAfterZero() {
        unsigned code = 0;
        for (int taken = 0; taken < 2 && m_at < m_text.size() && isOctal(m_text[m_at]); ++taken)
            code = (code * kOctalBase) + static_cast<unsigned>(m_text[m_at++] - '0');
        literal(characterOf(code));
        return std::nullopt;
    }

    /// `\1` to `\9`: a group of one or two digits, or three octal digits — the
    /// order `sre_parse.parse_template` tries them in.
    [[nodiscard]] std::optional<TemplateError> digits(char first) {
        int number = first - '0';
        if (m_at < m_text.size() && isDigit(m_text[m_at])) {
            const char second = m_text[m_at++];
            const bool octal =
                isOctal(first) && isOctal(second) && m_at < m_text.size() && isOctal(m_text[m_at]);
            if (octal) {
                const unsigned code =
                    (static_cast<unsigned>(first - '0') * kOctalBase * kOctalBase) +
                    (static_cast<unsigned>(second - '0') * kOctalBase) +
                    static_cast<unsigned>(m_text[m_at++] - '0');
                constexpr unsigned kLargest = 0377;
                if (code > kLargest)
                    return fail("octal escape value out of range");
                literal(characterOf(code));
                return std::nullopt;
            }
            number = (number * kDecimalBase) + (second - '0');
        }
        return group(number);
    }

    /// `\g<name>` or `\g<number>`.
    [[nodiscard]] std::optional<TemplateError> namedGroup() {
        if (m_at == m_text.size() || m_text[m_at] != '<')
            return fail("missing < after \\g");
        const std::size_t close = m_text.find('>', m_at);
        if (close == std::string_view::npos)
            return fail("missing > after \\g<");
        const std::string_view name = m_text.substr(m_at + 1, close - m_at - 1);
        m_at = close + 1;

        if (name.empty())
            return fail("missing group name");
        if (std::ranges::all_of(name, isDigit))
            return group(std::stoi(std::string{name}));

        const std::optional<int> found = m_groupNamed(name);
        if (!found)
            return fail("unknown group name '" + std::string{name} + "'");
        return group(*found);
    }

    std::string_view m_text;
    std::size_t m_groupCount;
    const std::function<std::optional<int>(std::string_view)>& m_groupNamed;
    std::size_t m_at = 0;
    ReplacementTemplate m_result;
};

std::expected<ReplacementTemplate, TemplateError>
ReplacementTemplate::parse(std::string_view text,
                           std::size_t groupCount,
                           const std::function<std::optional<int>(std::string_view)>& groupNamed) {
    return TemplateReading{text, groupCount, groupNamed}.read();
}

std::string ReplacementTemplate::expandedFor(std::string_view text, const Match& match) const {
    std::string out;
    for (const Piece& piece : m_pieces) {
        if (piece.group < 0) {
            out += piece.literal;
            continue;
        }
        const auto number = static_cast<std::size_t>(piece.group);
        if (number >= match.groups.size())
            continue;
        if (const std::optional<MatchSpan>& span = match.groups[number])
            out += text.substr(span->start, span->end - span->start);
    }
    return out;
}

} // namespace subedit::core
