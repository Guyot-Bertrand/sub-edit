#pragma once

#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <string_view>

namespace subedit::core {

/// Reads the HTML-style tags of SubRip, WebVTT and SubViewer 2.
///
/// `<b>`, `<i>`, `<u>` and `<font color="#RRGGBB">`, in either case. Everything
/// else a file of these formats may carry — `<c.loud>`, `<v Marie>`, `<ruby>`,
/// the internal timestamps of WebVTT — is dropped and counted: ADR 0031 puts
/// layout outside the pivot on purpose.
///
/// **The three formats are read alike, and written apart.** A `<font color>` in
/// a WebVTT file is not WebVTT, but reading it costs nothing and refusing it
/// would lose a colour someone put there; whether it can be written back is
/// `abilitiesOf`'s question, one step later.
[[nodiscard]] DecodedMarkup decodeHtmlMarkup(std::string_view text);

/// Writes them back, keeping only what `abilities` allows.
[[nodiscard]] EncodedMarkup encodeHtmlMarkup(const StyledText& runs,
                                             const StyleAbilities& abilities);

} // namespace subedit::core
