#pragma once

#include <string>
#include <string_view>

namespace subedit::core {

/// The marker each format writes where a subtitle holds a line break.
///
/// **Most of the nine cannot hold a real break**, because their subtitles are
/// one file line long. Each names a stand-in instead, and the stand-ins are all
/// the same idea written differently: `[br]` in SubViewer 2, `|` in MicroDVD,
/// MPL2 and TMPlayer. Sub Station Alpha is the exception — it reads two markers
/// and writes one — and keeps its own pair of functions for that reason.
namespace break_marker {
constexpr std::string_view kBracketed = "[br]"; ///< SubViewer 2
constexpr std::string_view kPipe = "|";         ///< MicroDVD, MPL2, TMPlayer
} // namespace break_marker

/// Turns one file line into the text of a subtitle, `marker` becoming a break.
[[nodiscard]] std::string textFromMarker(std::string_view line, std::string_view marker);

/// Turns the text of a subtitle into one file line, a break becoming `marker`.
[[nodiscard]] std::string textToMarker(std::string_view text, std::string_view marker);

} // namespace subedit::core
