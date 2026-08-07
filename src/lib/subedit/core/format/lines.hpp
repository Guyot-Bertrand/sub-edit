#pragma once

#include <string_view>
#include <vector>

namespace subedit::core {

/// Cuts `content` into lines, on any of the three endings, keeping none of
/// them.
///
/// The views point into `content`, which must outlive them. A file of several
/// thousand subtitles is cut without copying a single line.
///
/// A trailing ending does not produce an empty last line: `"un\n"` is one line,
/// not two. Endings may be mixed within one file — they should not be, and the
/// reader reports it, but reporting it means having read the file first.
[[nodiscard]] std::vector<std::string_view> splitLines(std::string_view content);

/// Strips the spaces and tabulations around `text`.
///
/// Shared because every reader needs the same answer to « is this line
/// blank ? », and three copies of it would be three chances to disagree.
[[nodiscard]] std::string_view trimmedBlanks(std::string_view text);

/// Tells whether `text` holds nothing but blanks.
[[nodiscard]] inline bool isBlank(std::string_view text) {
    return trimmedBlanks(text).empty();
}

} // namespace subedit::core
