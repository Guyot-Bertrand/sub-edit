#include <subedit/core/format/format_detection.hpp>
#include <subedit/core/format/lrc_syntax.hpp>
#include <subedit/core/format/micro_dvd_syntax.hpp>
#include <subedit/core/format/mpl2_syntax.hpp>
#include <subedit/core/format/sub_viewer2_syntax.hpp>
#include <subedit/core/format/tm_player_syntax.hpp>
#include <subedit/core/text/lines.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kSignature = "WEBVTT";
constexpr std::string_view kScriptType = "ScriptType:";
constexpr std::string_view kAdvancedVersion = "4.00+";
constexpr std::string_view kVersion = "4.00";
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

/// Reads `ScriptType: v4.00+`, which is the one line the two formats disagree on.
///
/// **The `+` is the whole difference**, and Gaupol settles it the same way. It
/// is checked before the version without it, since one is a prefix of the other.
[[nodiscard]] std::optional<SubtitleFormat> scriptTypeOf(std::string_view line) {
    const std::string_view text = trimmedBlanks(line);
    if (!text.starts_with(kScriptType))
        return std::nullopt;

    std::string_view version = trimmedBlanks(text.substr(kScriptType.size()));

    // The `v` comes in both cases in the wild, and Gaupol's pattern accepts
    // both. Nothing else about the line varies.
    if (version.starts_with('v') || version.starts_with('V'))
        version.remove_prefix(1);

    if (version == kAdvancedVersion)
        return SubtitleFormat::AdvancedSubStationAlpha;
    if (version == kVersion)
        return SubtitleFormat::SubStationAlpha;
    return std::nullopt;
}

} // namespace

std::optional<SubtitleFormat> detectFormat(std::string_view content) {
    const std::vector<std::string_view> lines = splitLines(content);

    const auto firstText = std::ranges::find_if_not(lines, isBlank);
    if (firstText != lines.end() && trimmedBlanks(*firstText).starts_with(kSignature))
        return SubtitleFormat::WebVtt;

    if (std::ranges::any_of(lines, isSubRipTimeLine))
        return SubtitleFormat::SubRip;

    // **A declaration, and the only one of the nine formats that carries one.**
    // It comes before the timestamp lines below because it is stronger than
    // them: a file saying what it is settles the question, where a line shaped
    // a certain way only suggests it.
    for (const std::string_view line : lines) {
        if (const std::optional<SubtitleFormat> declared = scriptTypeOf(line))
            return declared;
    }

    // **The `[INFORMATION]` header is not what settles it, the timestamp line
    // is.** A header is a promise a file makes about itself; a timestamp line
    // is the format actually being spoken. Gaupol recognises this format the
    // same way, and it is what lets a file whose header was trimmed still open.
    if (std::ranges::any_of(
            lines, [](std::string_view line) { return parseSubViewer2TimeLine(line).has_value(); }))
        return SubtitleFormat::SubViewer2;

    // Two bracketed whole numbers opening a line, and nothing else among the
    // nine is written that way: LRC opens on one bracket holding a time,
    // MicroDVD on two braces.
    if (std::ranges::any_of(
            lines, [](std::string_view line) { return parseMpl2TimeLine(line).has_value(); }))
        return SubtitleFormat::Mpl2;

    // The same grammar in braces, and the only pair of the nine that had to be
    // told apart with care.
    if (std::ranges::any_of(
            lines, [](std::string_view line) { return parseMicroDvdFrameLine(line).has_value(); }))
        return SubtitleFormat::MicroDvd;

    // One bracket holding a time, where MPL2 puts two holding whole numbers.
    // The two are told apart by the colon inside, which is why this comes after
    // MPL2 rather than instead of it.
    if (std::ranges::any_of(
            lines, [](std::string_view line) { return parseLrcTimeLine(line).has_value(); }))
        return SubtitleFormat::Lrc;

    // **Last of the nine, and it is the one that claims the least.** Three
    // fields and a closing colon at the head of a line is a shape SubViewer 2
    // comes close to — `00:00:01.00,` — and the two are separated by what
    // follows the seconds. SubViewer 2 is looked for first all the same: a
    // header and a range say more than a bare start does.
    if (std::ranges::any_of(
            lines, [](std::string_view line) { return parseTMPlayerTimeLine(line).has_value(); }))
        return SubtitleFormat::TMPlayer;

    return std::nullopt;
}

} // namespace subedit::core
