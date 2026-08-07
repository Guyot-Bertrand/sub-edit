#include <subedit/core/format/lines.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kSignature = "WEBVTT";
constexpr std::string_view kArrow = "-->";
constexpr std::string_view kBlanks = " \t";

/// Tells whether `line` is a SubRip timestamp line.
///
/// The comma is what settles it. A file whose timestamps use a period and
/// which carries no signature is claimed by neither format — not WebVTT for
/// want of the signature, not SubRip for want of the comma — and refusing it
/// beats picking one at random.
[[nodiscard]] bool isSubRipTimeLine(std::string_view line) {
    const std::size_t arrow = line.find(kArrow);
    if (arrow == std::string_view::npos)
        return false;

    const std::string_view left = line.substr(0, arrow);
    if (!left.contains(','))
        return false;

    if (!Timestamp::parse(left).has_value())
        return false;

    // Whatever follows the end timestamp is the extended coordinates, which
    // detection has no need to check.
    const std::string_view right = trimmedBlanks(line.substr(arrow + kArrow.size()));
    const std::string_view stamp = right.substr(0, right.find_first_of(kBlanks));
    return stamp.contains(',') && Timestamp::parse(stamp).has_value();
}

} // namespace

std::optional<SubtitleFormat> detectFormat(std::string_view content) {
    const std::vector<std::string_view> lines = splitLines(content);

    const auto firstText = std::ranges::find_if_not(lines, isBlank);
    if (firstText != lines.end() && trimmedBlanks(*firstText).starts_with(kSignature))
        return SubtitleFormat::WebVtt;

    if (std::ranges::any_of(lines, isSubRipTimeLine))
        return SubtitleFormat::SubRip;

    return std::nullopt;
}

} // namespace subedit::core
