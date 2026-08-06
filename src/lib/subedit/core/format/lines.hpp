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

} // namespace subedit::core
