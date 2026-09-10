#pragma once

#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <string_view>

namespace subedit::core {

/// Reads the braced tags of MicroDVD.
///
/// `{y:i}`, `{c:$BBGGRR}`, `{f:NAME}`, `{s:POINTS}` — and the same four in
/// capitals. **The case is the scope**: a lower-case tag runs to the end of its
/// line, a capital one to the end of the subtitle. Neither closes.
///
/// A `{y:…}` value is a bag of letters: `{y:biu}` says three things at once.
[[nodiscard]] DecodedMarkup decodeMicroDvdMarkup(std::string_view text);

/// Writes them back, keeping only what `abilities` allows.
///
/// **A style that does not cover whole lines cannot be written, and is
/// dropped.** The tags of this format have no end: `{y:i}` italicises the rest
/// of its line whatever one wanted. Putting one in the middle of a line would
/// say more than the source did, so a line whose runs disagree is written
/// plain. Gaupol takes the same way out, in `_style`.
[[nodiscard]] EncodedMarkup encodeMicroDvdMarkup(const StyledText& runs,
                                                 const StyleAbilities& abilities);

/// Reads MPL2 — a marker at the head of a line, plus MicroDVD's braces.
///
/// `\` is bold, `/` is italic, `_` is underline, and each runs to the end of
/// its line. They may be piled up: `/_text` is italic and underlined.
[[nodiscard]] DecodedMarkup decodeMpl2Markup(std::string_view text);

/// Writes them back, keeping only what `abilities` allows.
///
/// The same rule about whole lines as MicroDVD, for the same reason, and it
/// bites harder: this format has no way at all to say « from here to the end of
/// the subtitle ».
[[nodiscard]] EncodedMarkup encodeMpl2Markup(const StyledText& runs,
                                             const StyleAbilities& abilities);

} // namespace subedit::core
