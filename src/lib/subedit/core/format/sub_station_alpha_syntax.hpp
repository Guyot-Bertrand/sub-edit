#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The names an `[Events]` section can give its columns.
///
/// **Ten, and the first is the only one the two formats disagree on.** Sub
/// Station Alpha opens on `Marked`, Advanced SSA on `Layer`; everything after
/// is shared, which is why `ass.py` inherits from `ssa.py` in Gaupol.
///
/// A `Format:` line naming anything else is reported and left out: what is
/// written back has to declare the columns it actually fills, and a column we
/// could not read is one we cannot fill.
namespace event_field {
constexpr std::string_view kMarked = "Marked";
constexpr std::string_view kLayer = "Layer";
constexpr std::string_view kStart = "Start";
constexpr std::string_view kEnd = "End";
constexpr std::string_view kStyle = "Style";
constexpr std::string_view kName = "Name";
constexpr std::string_view kMarginLeft = "MarginL";
constexpr std::string_view kMarginRight = "MarginR";
constexpr std::string_view kMarginVertical = "MarginV";
constexpr std::string_view kEffect = "Effect";
constexpr std::string_view kText = "Text";
} // namespace event_field

/// Tells whether that name is a column this library can read and write.
[[nodiscard]] bool isKnownEventField(std::string_view name);

/// The columns a file of `format` declares when it never declared any.
[[nodiscard]] std::vector<std::string> defaultEventFields(SubtitleFormat format);

/// Splits an event line into its columns, `Text` keeping its own commas.
///
/// **The last column is not split.** A line of dialogue holds commas, and the
/// `Format:` line says how many columns precede it — so the split stops there
/// and hands the rest over whole. It is the one rule that makes this format
/// readable at all.
[[nodiscard]] std::vector<std::string_view> splitEventLine(std::string_view values,
                                                           std::size_t columns);

/// Turns the text of an event into the text of a subtitle: `\N` becomes a break.
///
/// `\n` too, which Sub Station Alpha means as a soft break — nothing here tells
/// the two apart, and Gaupol does not either.
[[nodiscard]] std::string textFromEvent(std::string_view value);

/// Turns the text of a subtitle into the text of an event.
[[nodiscard]] std::string textToEvent(std::string_view text);

} // namespace subedit::core
