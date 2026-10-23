#include <subedit/core/io/json_reader.hpp>

#include <unicode/unistr.h>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>

namespace subedit::core {

namespace {

/// Deeper than any output we read, shallow enough that a hostile document
/// cannot exhaust the stack.
constexpr int kMaxDepth = 64;

/// The ranges of UTF-16 surrogates: a character above U+FFFF is written as a high
/// one followed by a low one.
constexpr std::uint32_t kHighSurrogateFirst = 0xD800;
constexpr std::uint32_t kLowSurrogateFirst = 0xDC00;
constexpr std::uint32_t kSurrogateLast = 0xDFFF;

constexpr std::string_view kWhitespace = " \t\r\n";

class Reader {

public:
    explicit Reader(std::string_view text) : m_text(text) {}

    [[nodiscard]] std::optional<JsonValue> document() {
        std::optional<JsonValue> value = read(0);
        skipWhitespace();
        if (!value || m_position != m_text.size())
            return std::nullopt;
        return value;
    }

private:
    void skipWhitespace() {
        while (m_position < m_text.size() &&
               kWhitespace.find(m_text[m_position]) != std::string_view::npos)
            ++m_position;
    }

    [[nodiscard]] bool consume(std::string_view literal) {
        if (!m_text.substr(m_position).starts_with(literal))
            return false;
        m_position += literal.size();
        return true;
    }

    [[nodiscard]] std::optional<JsonValue> read(int depth) {
        skipWhitespace();
        if (depth > kMaxDepth || m_position >= m_text.size())
            return std::nullopt;

        switch (m_text[m_position]) {
        case '{':
            return readObject(depth);
        case '[':
            return readArray(depth);
        case '"':
            return readString().transform(
                [](std::string text) { return JsonValue{std::move(text)}; });
        case 't':
            return consume("true") ? std::optional{JsonValue{true}} : std::nullopt;
        case 'f':
            return consume("false") ? std::optional{JsonValue{false}} : std::nullopt;
        case 'n':
            return consume("null") ? std::optional{JsonValue{}} : std::nullopt;
        default:
            return readNumber();
        }
    }

    [[nodiscard]] std::optional<JsonValue> readNumber() {
        double number = 0;
        const char* begin = m_text.data() + m_position;
        const char* end = m_text.data() + m_text.size();
        const std::from_chars_result parsed = std::from_chars(begin, end, number);
        // `from_chars` accepts `inf`, `nan` and a leading `+` on some libraries;
        // JSON has none of them, and none starts with a digit or a minus.
        const bool startsLikeANumber = *begin == '-' || (*begin >= '0' && *begin <= '9');
        if (parsed.ec != std::errc{} || !startsLikeANumber)
            return std::nullopt;
        m_position += static_cast<std::size_t>(parsed.ptr - begin);
        return JsonValue{number};
    }

    /// Four hexadecimal digits at the current position, or nothing.
    [[nodiscard]] std::optional<std::uint32_t> readCodeUnit() {
        std::uint32_t unit = 0;
        const char* begin = m_text.data() + m_position;
        if (m_text.size() - m_position < 4)
            return std::nullopt;
        const std::from_chars_result parsed = std::from_chars(begin, begin + 4, unit, 16);
        if (parsed.ec != std::errc{} || parsed.ptr != begin + 4)
            return std::nullopt;
        m_position += 4;
        return unit;
    }

    /// A `\u` escape appended to `out` as UTF-8, with its low surrogate when the
    /// first unit is a high one. False when the escape is not valid.
    [[nodiscard]] bool readUnicodeEscape(std::string& out) {
        const std::optional<std::uint32_t> first = readCodeUnit();
        if (!first)
            return false;
        icu::UnicodeString characters;
        characters.append(static_cast<char16_t>(*first));
        if (*first >= kHighSurrogateFirst && *first <= kSurrogateLast) {
            // A low surrogate on its own, or a high one with nothing to pair with.
            if (*first >= kLowSurrogateFirst || !consume("\\u"))
                return false;
            const std::optional<std::uint32_t> second = readCodeUnit();
            if (!second || *second < kLowSurrogateFirst || *second > kSurrogateLast)
                return false;
            characters.append(static_cast<char16_t>(*second));
        }
        characters.toUTF8String(out);
        return true;
    }

    /// What follows a backslash, appended to `out`. False when it is not an escape.
    [[nodiscard]] bool readEscape(std::string& out) {
        constexpr std::string_view kPlain = "\"\\/";
        constexpr std::string_view kNamed = "bfnrt";
        constexpr std::string_view kNamedValues = "\b\f\n\r\t";
        if (m_position >= m_text.size())
            return false;
        const char letter = m_text[m_position++];
        if (kPlain.find(letter) != std::string_view::npos) {
            out += letter;
            return true;
        }
        if (const std::size_t named = kNamed.find(letter); named != std::string_view::npos) {
            out += kNamedValues[named];
            return true;
        }
        return letter == 'u' && readUnicodeEscape(out);
    }

    [[nodiscard]] std::optional<std::string> readString() {
        ++m_position; // the opening quote
        std::string out;
        while (m_position < m_text.size()) {
            const char letter = m_text[m_position++];
            if (letter == '"')
                return out;
            if (letter != '\\') {
                out += letter;
            } else if (!readEscape(out)) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<JsonValue> readArray(int depth) {
        ++m_position;
        JsonValue::Array items;
        skipWhitespace();
        if (consume("]"))
            return JsonValue{std::move(items)};
        while (true) {
            std::optional<JsonValue> item = read(depth + 1);
            if (!item)
                return std::nullopt;
            items.push_back(std::move(*item));
            skipWhitespace();
            if (consume("]"))
                return JsonValue{std::move(items)};
            if (!consume(","))
                return std::nullopt;
        }
    }

    [[nodiscard]] std::optional<JsonValue> readObject(int depth) {
        ++m_position;
        JsonValue::Object members;
        skipWhitespace();
        if (consume("}"))
            return JsonValue{std::move(members)};
        while (true) {
            skipWhitespace();
            if (m_position >= m_text.size() || m_text[m_position] != '"')
                return std::nullopt;
            std::optional<std::string> key = readString();
            skipWhitespace();
            if (!key || !consume(":"))
                return std::nullopt;
            std::optional<JsonValue> value = read(depth + 1);
            if (!value)
                return std::nullopt;
            members.emplace_back(std::move(*key), std::move(*value));
            skipWhitespace();
            if (consume("}"))
                return JsonValue{std::move(members)};
            if (!consume(","))
                return std::nullopt;
        }
    }

    std::string_view m_text;
    std::size_t m_position = 0;
};

} // namespace

const JsonValue* JsonValue::find(std::string_view key) const {
    const Object* members = std::get_if<Object>(&m_value);
    if (members == nullptr)
        return nullptr;
    const auto found =
        std::ranges::find_if(*members, [key](const auto& member) { return member.first == key; });
    return found == members->end() ? nullptr : &found->second;
}

const std::string* JsonValue::asString() const {
    return std::get_if<std::string>(&m_value);
}

std::optional<double> JsonValue::asNumber() const {
    if (const double* number = std::get_if<double>(&m_value))
        return *number;
    return std::nullopt;
}

std::optional<bool> JsonValue::asBool() const {
    if (const bool* flag = std::get_if<bool>(&m_value))
        return *flag;
    return std::nullopt;
}

const JsonValue::Array* JsonValue::asArray() const {
    return std::get_if<Array>(&m_value);
}

std::optional<JsonValue> parseJson(std::string_view text) {
    return Reader{text}.document();
}

} // namespace subedit::core
