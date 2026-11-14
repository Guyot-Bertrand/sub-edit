#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes YouTube SBV.
///
/// The layout is the one YouTube's editor exports: a block per subtitle, the
/// time line `0:00:01.000,0:00:03.000` with the hour unpadded, the text, and a
/// blank line between two blocks — so a file read and written back comes out
/// byte for byte identical.
///
/// **The text goes out as it is held.** The format has no markup, so a text
/// converted from a format that has some arrives without it: the markup pivot
/// took it off, and the conversion said so (spec 09, D5).
class YouTubeSbvWriter final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
