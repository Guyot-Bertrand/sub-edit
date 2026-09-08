#include <subedit/core/format/sub_viewer2_syntax.hpp>
#include <subedit/core/text/lines.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <optional>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::size_t kStampLength = 11; // `HH:MM:SS.cc`

/// The character `HH:MM:SS.cc` holds at that place, or a null for a digit.
[[nodiscard]] constexpr char punctuationAt(std::size_t index) {
    constexpr std::size_t kFirstColon = 2;
    constexpr std::size_t kSecondColon = 5;
    constexpr std::size_t kMark = 8;

    if (index == kFirstColon || index == kSecondColon)
        return ':';
    if (index == kMark)
        return '.';
    return '\0';
}

/// Tells whether `text` is exactly `HH:MM:SS.cc`, sign included or not.
///
/// **Stricter than `Timestamp::parse`, on purpose.** The reading of a position
/// is deliberately permissive — one or two digits, one to three decimals, hours
/// optional — and that permissiveness is what tells a SubViewer 2 line from a
/// line of another format. `00:01,500` would parse; it is not this format.
[[nodiscard]] bool hasStampShape(std::string_view text) {
    if (text.starts_with('-'))
        text.remove_prefix(1);
    if (text.size() != kStampLength)
        return false;

    for (std::size_t index = 0; index < text.size(); ++index) {
        const char punctuation = punctuationAt(index);
        const char character = text[index];
        if (punctuation != '\0') {
            if (character != punctuation)
                return false;
        } else if (character < '0' || character > '9') {
            return false;
        }
    }
    return true;
}

} // namespace

std::optional<SubViewer2TimeLine> parseSubViewer2TimeLine(std::string_view line) {
    const std::string_view text = trimmedBlanks(line);
    const std::size_t comma = text.find(',');
    if (comma == std::string_view::npos)
        return std::nullopt;

    const std::string_view left = text.substr(0, comma);
    const std::string_view right = text.substr(comma + 1);
    if (!hasStampShape(left) || !hasStampShape(right))
        return std::nullopt;

    const std::optional<Timestamp> start = Timestamp::parse(left);
    const std::optional<Timestamp> end = Timestamp::parse(right);
    if (!start.has_value() || !end.has_value())
        return std::nullopt;

    return SubViewer2TimeLine{.start = *start, .end = *end};
}

} // namespace subedit::core
