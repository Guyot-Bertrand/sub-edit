#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads TMPlayer — the poorest of the nine, and the other one behind a `.txt`.
///
/// **One position per line, and it is the start.** `00:00:12:text` says when a
/// subtitle begins and never when it ends: the ends are worked out here, the
/// next start for each one and five seconds for the last, and the reading says
/// so — ADR 0029.
///
/// It counts in whole seconds, has no vocabulary for italics, and writes its
/// line breaks as `|`. What it does declare is the shape of its own hour, on
/// one digit or on two, which the document keeps so that writing puts the same
/// one back.
class TMPlayerReader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
