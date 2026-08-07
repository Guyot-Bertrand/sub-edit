#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads the WebVTT format — the `.vtt` files of the web.
///
/// The file is a series of blocks separated by blank lines. The first is the
/// header, and the rest are cues, `STYLE` blocks and `NOTE` blocks. **The
/// `STYLE` and `NOTE` blocks belong to the cue that follows them**, which is
/// what the format says and what makes them survive a round trip.
///
/// **The signature is required.** A file that does not begin with `WEBVTT` is
/// refused rather than read at a guess: the detection of phase 8 needs readers
/// that say no, so that no format is ever assumed.
///
/// The tags the format allows in a text — `<v Locuteur>`, `<c.classe>`,
/// `<ruby>` — are **not** interpreted. The text stays a raw string, as ADR
/// 0009 has it; a reader that normalised them would make the file come back
/// changed.
class WebVttReader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
