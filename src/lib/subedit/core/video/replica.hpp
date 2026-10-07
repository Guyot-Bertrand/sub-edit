#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

/// The text the overlay of the video draws, for a subtitle written in `format` — issue #408.
///
/// **The tags of the format, understood rather than drawn.** A subtitle is held as its file wrote
/// it, tags included (ADR 0009), and the picture showed them as they stood: a `<i>` was drawn as a
/// `<i>`, a `{\an8}` as a `{\an8}`. The pivot of ADR 0031 reads the text in the vocabulary of its
/// format and writes it back in the Sub Station Alpha one, which is what libmpv's overlay speaks:
/// italics, bold, underline and colour become override blocks the picture applies.
///
/// **A tag with no equivalent on screen is removed, never drawn as it stands** — a position, an
/// alignment, a speaker, a ruby. Font and size are in that number too: a size of `<font size=3>`
/// is not one of the overlay's, and honouring it would draw a replica nobody can read. There is
/// no second reader here: the one the conversion uses is the one that reads.
///
/// **What comes out is in the Sub Station Alpha vocabulary, braces of the visible text escaped**
/// (`\{`), so that a subtitle saying « {laughs} » says it rather than disappearing into a block
/// libass would take for a tag. Line breaks stay what they were; the player turns them into the
/// breaks of its overlay.
[[nodiscard]] std::string replicaOf(std::string_view text, SubtitleFormat format);

} // namespace subedit::core
