#pragma once

// Putting the text of a subtitle in italics, and taking it out again.
//
// **Surgical, and it has to be.** The pivot of ADR 0031 translates one
// vocabulary into another and drops what it has no room for — the layout
// overrides, which it counts as lost. An italic toggle routed through it would
// delete a `{\pos(x,y)}` from a file nobody asked to convert. What follows
// touches the italic tags and leaves every other one exactly where it was.

#include <subedit/core/model/subtitle_format.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

/// Tells whether the first visible character of `text` is in italics.
///
/// **The first, and not all of them.** A text half in italics is not italic:
/// the question a single button asks is « would pressing it add or remove », and
/// Gaupol answers it by looking at the head of the text. Anything finer would
/// need a second button to undo it.
///
/// Always false for the two formats that write no style at all.
[[nodiscard]] bool opensInItalics(std::string_view text, SubtitleFormat format);

/// Returns `text` with every italic tag of `format` taken out.
///
/// **A tag that says more than italics keeps the rest.** `{\b1\i1}` comes back
/// as `{\b1}` and `{Y:bi}` as `{Y:b}`, where Gaupol — matching one whole tag at
/// a time — leaves both untouched and hands back a text still in italics after
/// being asked to take them out. That is the one place this differs, and it
/// differs because the user sees it.
[[nodiscard]] std::string withoutItalics(std::string_view text, SubtitleFormat format);

/// Returns `text` in italics, written the way `format` writes them.
///
/// **Idempotent**: the italics already there come out before the new ones go
/// on, so a text put in italics twice is not wrapped twice.
///
/// A text with nothing in it is handed back as it is. An `<i></i>` around
/// nothing is what a selection holding a blank row would otherwise gain, and it
/// is visible in the table.
[[nodiscard]] std::string inItalics(std::string_view text, SubtitleFormat format);

} // namespace subedit::core
