#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes the SubRip format.
///
/// The numbering is **regenerated** from one, whatever the file said: this is
/// what makes a file whose numbers do not follow heal itself by being opened
/// and saved.
///
/// The layout is Gaupol's, down to the blank line closing every block,
/// including the last — a file read and written back comes out byte for byte
/// identical, save for that numbering.
class SubRipWriter final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
