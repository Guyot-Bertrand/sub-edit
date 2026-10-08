#pragma once

// What the tool says a conversion, a paste or an append cost. Shared wording
// of `core/wording/`; see `formats.hpp` for why it lives in the core.

#include <subedit/core/model/subtitle_format.hpp>

#include <cstddef>
#include <optional>
#include <string>

namespace subedit::core {

struct ConversionLoss;
struct GridRepair;

/// What a conversion cost, post by post, or nothing when it cost nothing.
///
/// **The silence is half the design.** A conversion that loses nothing returns
/// an empty string and the surfaces say nothing: a report printed on every call
/// is a report nobody reads, and the ones that matter would go by unnoticed
/// among them.
///
/// The posts are the phase's, in the order the scoping lists them: the ends a
/// format does not carry, the line breaks it joined, the tags it could not
/// write, what it left of the header and of a subtitle's own fields, and how
/// far the positions moved. Each appears only when it is not zero.
[[nodiscard]] std::string
noticeOf(const ConversionLoss& loss, SubtitleFormat from, SubtitleFormat to);

/// What the search for the conversion that puts a file back on a grid found, or nothing.
///
/// **Said when there is something to choose or to refuse**: the conversion found, with how well
/// it fits, how well the positions fit as they are and how well the next best conversion
/// fits — the deduction's own measure, three times —, or the two conversions that fit equally
/// well, which is why none is proposed. A file that is on a grid already, or that no
/// conversion of the closed set mends, gets an empty string: nothing is proposed, and nothing
/// is said of it — issue #386.
[[nodiscard]] std::string repairNoticeOf(const GridRepair& repair);

/// What a paste did that the table does not show by itself, or nothing.
///
/// Two posts: the rows laid down past the end to receive the texts — Gaupol's
/// « inserted N subtitles to fit clipboard contents » — and the tags that did
/// not survive a paste from another format, in the words of `noticeOf`, which
/// `Save As…` already uses. `from` is nothing for a text that came from outside
/// this program, which has no tags to translate.
[[nodiscard]] std::string noticeOfPaste(std::size_t inserted,
                                        const ConversionLoss& loss,
                                        std::optional<SubtitleFormat> from,
                                        SubtitleFormat to);

/// What appending a file did: how many subtitles it added, and what crossing
/// into `to` cost the ones that came from `from` — the same words as a paste
/// of another format, through the same `noticeOf(ConversionLoss, …)`.
[[nodiscard]] std::string noticeOfAppend(std::size_t inserted,
                                         const ConversionLoss& loss,
                                         SubtitleFormat from,
                                         SubtitleFormat to);

} // namespace subedit::core
