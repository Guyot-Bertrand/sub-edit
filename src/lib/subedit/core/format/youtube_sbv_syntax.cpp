#include <subedit/core/format/youtube_sbv_syntax.hpp>
#include <subedit/core/text/lines.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <optional>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::string_view kDigits = "0123456789";

/// Tells whether `text` opens on `count` digits, and takes them off.
[[nodiscard]] bool takeDigits(std::string_view& text, std::size_t count) {
    if (text.size() < count ||
        text.substr(0, count).find_first_not_of(kDigits) != std::string_view::npos)
        return false;
    text.remove_prefix(count);
    return true;
}

/// Tells whether `text` opens on `mark`, and takes it off.
[[nodiscard]] bool takeMark(std::string_view& text, char mark) {
    if (!text.starts_with(mark))
        return false;
    text.remove_prefix(1);
    return true;
}

/// Tells whether `text` is exactly `H:MM:SS.mmm`, one or two hour digits, sign or not.
///
/// **Stricter than `Timestamp::parse`, on purpose**, as SubViewer 2's check is:
/// the permissive reading of a position would take SubViewer 2's own stamps.
[[nodiscard]] bool hasStampShape(std::string_view text) {
    // A sign, which a position before the video's start is written with; the
    // writer puts it there, so the reader has to take it back.
    if (text.starts_with('-'))
        text.remove_prefix(1);
    const std::size_t hourDigits = text.find(':');
    if (hourDigits != 1 && hourDigits != 2)
        return false;
    return takeDigits(text, hourDigits) && takeMark(text, ':') && takeDigits(text, 2) &&
           takeMark(text, ':') && takeDigits(text, 2) && takeMark(text, '.') &&
           takeDigits(text, 3) && text.empty();
}

} // namespace

std::optional<YouTubeSbvTimeLine> parseYouTubeSbvTimeLine(std::string_view line) {
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

    return YouTubeSbvTimeLine{.start = *start, .end = *end};
}

} // namespace subedit::core
