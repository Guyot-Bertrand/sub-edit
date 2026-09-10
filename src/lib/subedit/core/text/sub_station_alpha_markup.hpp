#pragma once

#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <string_view>

namespace subedit::core {

/// Reads the override blocks of Sub Station Alpha and Advanced SSA.
///
/// `{\i1}…{\i0}`, `{\b1}…{\b0}`, `{\u1}…{\u0}`, `{\c&HBBGGRR&}`, `{\fnNAME}`,
/// `{\fsPOINTS}`, and `{\r}` — the reset that closes everything at once.
///
/// **A block is read as a change of state, not as a pair of brackets.** Colour,
/// font and size have no closing override: `{\c&Hff&}` runs to the end of the
/// subtitle or to the next `{\r}`, and a reading that looked for a matching
/// closer would find none. Carrying the style forward is both simpler and what
/// the format means.
///
/// The rest — `{\pos(x,y)}`, `{\an8}`, `{\t(...)}`, the numbered colours — is
/// dropped and counted. ADR 0031 leaves layout outside the pivot.
[[nodiscard]] DecodedMarkup decodeSubStationAlphaMarkup(std::string_view text);

/// Writes them back, keeping only what `abilities` allows.
[[nodiscard]] EncodedMarkup encodeSubStationAlphaMarkup(const StyledText& runs,
                                                        const StyleAbilities& abilities);

} // namespace subedit::core
