#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads YouTube SBV, the caption format of YouTube's editor.
///
/// Blocks separated by a blank line: a time line `0:00:01.000,0:00:03.000`,
/// then the text, on as many lines as it takes. No header, no numbering, no
/// markup — a `<i>` in the text is text, and it stays as it was written.
///
/// **No specification is published**; the platform's help page describes the
/// format, and the fixtures are written against it (spec 17, D7). What the help
/// page does not say, the reader decides the way SubViewer 2's does: a line
/// where a time line was expected is reported and dropped (ADR 0008).
class YouTubeSbvReader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
