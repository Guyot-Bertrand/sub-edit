#pragma once

#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace subedit::core {

/// Tells whether a file of `format` carries the end of a subtitle.
///
/// Seven of the nine do. TMPlayer and LRC hold one position per line, and the
/// ends of a document converted into either are not written at all — ADR 0029.
[[nodiscard]] bool carriesEnds(SubtitleFormat format);

/// Tells whether a file of `format` can hold a line break.
///
/// Eight of the nine can, one way or another — a real break, `[br]`, `\N`, `|`.
/// LRC cannot: its subtitles are one file line long, and two lines come back
/// as one with their words joined by a space.
[[nodiscard]] bool holdsLineBreaks(SubtitleFormat format);

/// Where `position` lands once a file of `format` has written it.
///
/// **The grain is a property of the format, and the eight time formats have a
/// whole number of milliseconds for one.** MicroDVD does not: its grain is a
/// frame, so the rate is what says where a position falls — and that is the
/// one case where the answer depends on something the file does not carry.
[[nodiscard]] Timestamp asWrittenBy(Timestamp position, SubtitleFormat format, FrameRate rate);

/// What converting a document into another format cost it.
///
/// **Every field is a post, and a post is only reported when it is not zero.**
/// A conversion that loses nothing says nothing: a report printed on every call
/// is a report nobody reads, which is the phase's own rule for this.
struct ConversionLoss {
    /// The arriving format writes no end.
    bool ends = false;

    /// Subtitles whose lines were joined, the format having no break.
    std::size_t joined = 0;

    /// Tags that did not survive — unwritable, or unknown to the pivot.
    std::size_t tags = 0;

    /// A header the arriving format will not take back.
    bool header = false;

    /// Subtitles that carried data of their own and will not any more.
    std::size_t fields = 0;

    /// The furthest a position moved, in milliseconds. Zero when none moved.
    std::int64_t precision = 0;

    /// Tells whether anything at all was lost.
    [[nodiscard]] bool isAny() const;

    friend bool operator==(const ConversionLoss&, const ConversionLoss&) = default;
};

/// Rewrites `subtitles` for `target`, and says what the crossing cost.
///
/// **The one place a conversion happens, and it happens once.** The markup is
/// carried across by the pivot of ADR 0031, and the same walk measures what the
/// arriving format will not be able to hold. Doing it twice — once to convert,
/// once to report — would be two chances to disagree.
///
/// `rate` is only read for a target counted in frames, and a caller converting
/// into one has had to settle it beforehand anyway.
[[nodiscard]] ConversionLoss convertFor(std::vector<Subtitle>& subtitles,
                                        const SourceFile& source,
                                        SubtitleFormat target,
                                        FrameRate rate);

} // namespace subedit::core
