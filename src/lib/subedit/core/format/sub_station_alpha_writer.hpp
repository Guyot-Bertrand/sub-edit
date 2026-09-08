#pragma once

#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <string>

namespace subedit::core {

/// Writes Sub Station Alpha and Advanced SSA — one writer for the two.
///
/// The layout is Gaupol's: the header, a blank line, `[Events]`, the `Format:`
/// line, then one `Dialogue:` line per subtitle. A file read and written back
/// comes out byte for byte identical, column order included.
///
/// **A document that never carried a header gets the template one.** A file
/// without `[Script Info]` and a `ScriptType` is not recognised as this format
/// at all — the detection reads exactly that line — so it would not come back.
class SubStationAlphaWriter final : public SubtitleWriter {

public:
    explicit SubStationAlphaWriter(SubtitleFormat format) : m_format(format) {}

    [[nodiscard]] std::string write(const WriteRequest& request) const override;

private:
    SubtitleFormat m_format;
};

} // namespace subedit::core
