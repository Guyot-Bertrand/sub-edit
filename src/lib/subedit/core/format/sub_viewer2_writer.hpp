#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes the SubViewer 2.0 format.
///
/// The layout is Gaupol's: the header, then one blank line before each block —
/// so a file read and written back comes out byte for byte identical.
///
/// **A document that never carried a header gets the template one**, which is
/// what `SubtitleWriter` promises for the formats that need one. A SubViewer 2
/// file without its `[INFORMATION]` block is not a SubViewer 2 file, and would
/// not be recognised as one on the way back in.
class SubViewer2Writer final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
