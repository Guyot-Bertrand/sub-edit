#include <subedit/core/text/python_regex_syntax.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

/// What Python's `\w` is, written as the contents of an ICU set.
constexpr std::string_view kWordSet = R"(\p{L}\p{N}_)";

[[nodiscard]] bool isAsciiLetter(char letter) {
    return (letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z');
}

[[nodiscard]] bool isAsciiDigit(char letter) {
    return letter >= '0' && letter <= '9';
}

/// A group name ICU accepts: an ASCII letter, then letters and digits.
[[nodiscard]] bool isIcuGroupName(std::string_view name) {
    return !name.empty() && isAsciiLetter(name.front()) &&
           std::ranges::all_of(
               name, [](char letter) { return isAsciiLetter(letter) || isAsciiDigit(letter); });
}

/// The boundary between a word character and what is not one, in either order.
[[nodiscard]] std::string wordBoundary(bool negated) {
    const std::string word = "[" + std::string{kWordSet} + "]";
    const std::string inside = "(?<=" + word + ")";
    const std::string outside = "(?<!" + word + ")";
    if (negated) {
        return "(?:" + inside + "(?=" + word + ")|" + outside + "(?!" + word + "))";
    }
    return "(?:" + inside + "(?!" + word + ")|" + outside + "(?=" + word + "))";
}

/// An escape outside a class.
[[nodiscard]] std::string escapeOutsideClass(char letter) {
    switch (letter) {
    case 'Z':
        return R"(\z)";
    case 'w':
        return "[" + std::string{kWordSet} + "]";
    case 'W':
        return "[^" + std::string{kWordSet} + "]";
    case 'b':
        return wordBoundary(false);
    case 'B':
        return wordBoundary(true);
    default:
        return std::string{'\\'} + letter;
    }
}

/// An escape inside a class: `\w` is the contents of the set, `\W` a nested
/// set that is the complement of it, and `\b` is a backspace there, which ICU
/// reads the same way.
[[nodiscard]] std::string escapeInClass(char letter) {
    switch (letter) {
    case 'w':
        return std::string{kWordSet};
    case 'W':
        return "[^" + std::string{kWordSet} + "]";
    default:
        return std::string{'\\'} + letter;
    }
}

/// `(?P<name>` or `(?P=name)` at `at`: how far it reaches, and what it becomes.
struct NamedGroup {
    std::size_t length;
    std::string icu;
};

[[nodiscard]] std::expected<NamedGroup, SyntaxError> namedGroupAt(std::string_view text,
                                                                  std::size_t at) {
    const std::string_view rest = text.substr(at);
    const bool opens = rest.starts_with("(?P<");
    const std::size_t nameStart = 4;
    const std::size_t close = rest.find(opens ? '>' : ')', nameStart);
    if (close == std::string_view::npos)
        return std::unexpected{SyntaxError{.reason = "an unterminated group name"}};

    const std::string_view name = rest.substr(nameStart, close - nameStart);
    if (!isIcuGroupName(name)) {
        return std::unexpected{
            SyntaxError{.reason = "the group name '" + std::string{name} +
                                  "' is not letters and digits, which is all ICU accepts"}};
    }
    if (opens)
        return NamedGroup{.length = close + 1, .icu = "(?<" + std::string{name} + ">"};
    return NamedGroup{.length = close + 1, .icu = "\\k<" + std::string{name} + ">"};
}

} // namespace

namespace {

/// A character inside a class, which `at` stands on: `]` closes it, `[` and
/// `&&` are literal here and ICU would read them as sets.
void appendInClass(std::string_view python, std::size_t at, std::string& icu, bool& inClass) {
    const char here = python[at];
    if (here == ']') {
        inClass = false;
        icu += here;
    } else if (here == '[') {
        icu += R"(\[)";
    } else if (here == '&' && at + 1 < python.size() && python[at + 1] == '&') {
        icu += R"(\&)";
    } else {
        icu += here;
    }
}

/// A `[` outside a class, which `at` stands on, and what follows it that
/// belongs to its opening: a `^` opens a negated class, and a `]` first in a
/// class is a literal one — in Python, and here it is written so ICU agrees.
/// Leaves `at` on the last character it took.
void openClass(std::string_view python, std::size_t& at, std::string& icu) {
    icu += '[';
    if (at + 1 < python.size() && python[at + 1] == '^') {
        icu += '^';
        ++at;
    }
    if (at + 1 < python.size() && python[at + 1] == ']') {
        icu += R"(\])";
        ++at;
    }
}

[[nodiscard]] bool startsNamedGroup(std::string_view python, std::size_t at) {
    return python.substr(at).starts_with("(?P<") || python.substr(at).starts_with("(?P=");
}

} // namespace

std::expected<std::string, SyntaxError> icuExpressionOf(std::string_view python) {
    std::string icu;
    bool inClass = false;

    for (std::size_t at = 0; at < python.size(); ++at) {
        if (python[at] == '\\') {
            if (at + 1 == python.size())
                return std::unexpected{SyntaxError{.reason = "a backslash ends the expression"}};
            ++at;
            icu += inClass ? escapeInClass(python[at]) : escapeOutsideClass(python[at]);
        } else if (inClass) {
            appendInClass(python, at, icu, inClass);
        } else if (python[at] == '[') {
            inClass = true;
            openClass(python, at, icu);
        } else if (startsNamedGroup(python, at)) {
            const std::expected<NamedGroup, SyntaxError> group = namedGroupAt(python, at);
            if (!group)
                return std::unexpected{group.error()};
            icu += group->icu;
            at += group->length - 1;
        } else {
            icu += python[at];
        }
    }
    return icu;
}

} // namespace subedit::core
