#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::core {

/// A text carried from one format's vocabulary into another's.
struct ConvertedMarkup {
    std::string text{};

    /// How many tags did not survive the crossing.
    ///
    /// Two things end up here, and a reader has no reason to tell them apart:
    /// what the pivot had no room for — the layout ADR 0031 leaves out — and
    /// what the arriving format cannot write.
    std::size_t dropped = 0;
};

/// Rewrites `text` so that `to` says what `from` said, as far as it can.
///
/// **The pivot lives here and nowhere else** — ADR 0031. A text is decoded into
/// runs of style, and written back in the vocabulary of the format it is going
/// to; what that format cannot say is dropped, and counted.
///
/// **A text going nowhere is not touched.** `from == to` returns the string as
/// it stands: saving a file in its own format is not a conversion, and ADR 0009
/// keeps the raw text raw. That is also what makes `convert --to srt` on a
/// `.srt` give back the same bytes.
[[nodiscard]] ConvertedMarkup
convertMarkup(std::string_view text, SubtitleFormat from, SubtitleFormat to);

} // namespace subedit::core
