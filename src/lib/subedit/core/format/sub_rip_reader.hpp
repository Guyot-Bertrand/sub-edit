#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads the SubRip format — the `.srt` files everyone has.
///
/// Deliberately tolerant. What starts a subtitle is a **timestamp line**, not
/// the number above it and not the blank line before it: files in the wild are
/// missing numbers, have numbers that do not follow, and separate their blocks
/// however they please.
///
/// This is where the pitfall found in Gaupol is avoided. Its reader appends
/// every non-timestamp line to the previous subtitle, and raises an
/// undocumented exception when there is no previous subtitle — a file
/// beginning with a translator's note is enough. Ours reports the lines and
/// looks for the next block.
class SubRipReader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
