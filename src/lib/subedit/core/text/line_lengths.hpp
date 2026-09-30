#pragma once

// The length of each line of a subtitle, as the table and its editor show it —
// issue #526, the rule of Gaupol's `gaupol/ruler.py`.

#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <string_view>
#include <vector>

namespace subedit::core {

/// The length of every line of `text`, tags left out, each rounded down.
///
/// **One entry per line, empty lines included**: a text of `"a\n\nb"` has three
/// lines, and the table shows a length beside each of them — the middle one
/// zero. A text with no line break is one line, and an empty text is one empty
/// line, because that is what an editor opened on it shows.
///
/// **Tags come out the way every other reader of the core takes them out**:
/// `decodeAs` in the `vocabulary` that wrote the text, then the text of the runs
/// it gave — never a pattern of this file's own. Line breaks survive that
/// reading, so the text is cut into lines afterwards.
///
/// **Rounded down, as Gaupol does** (`int(...)` on the length): a line of
/// 12,9 ems reads 12, so the number never claims a line is longer than it is.
/// In characters the length is whole already and the rounding changes nothing.
[[nodiscard]] std::vector<int>
lineLengths(const LineMeasure& measure, std::string_view text, MarkupVocabulary vocabulary);

} // namespace subedit::core
