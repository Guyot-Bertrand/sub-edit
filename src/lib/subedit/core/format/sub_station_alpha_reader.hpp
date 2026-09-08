#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads Sub Station Alpha and Advanced SSA — one reader for the two.
///
/// **They differ by one column and a version string**, which is why `ass.py`
/// inherits from `ssa.py` in Gaupol and why one class serves both here. The
/// format is carried rather than guessed: the detection settled it on
/// `ScriptType`, and the reader has no better evidence than that.
///
/// The layout is a header of sections, then `[Events]`, then a `Format:` line
/// naming the columns and one `Dialogue:` line per subtitle. **The column order
/// is read, never assumed**: it varies from one producer to the next, and
/// writing has to put back the one the file had.
class SubStationAlphaReader final : public SubtitleReader {

public:
    explicit SubStationAlphaReader(SubtitleFormat format) : m_format(format) {}

    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;

private:
    SubtitleFormat m_format;
};

} // namespace subedit::core
