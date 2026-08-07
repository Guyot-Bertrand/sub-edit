#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes the WebVTT format.
///
/// The layout is Gaupol's: the signature, then each cue preceded by its
/// `STYLE` and `NOTE` blocks, its identifier, and its timestamps carrying the
/// settings. A file read and written back comes out byte for byte identical.
///
/// **The hours are left out when no cue reaches one hour**, as Gaupol does.
/// The decision looks at every cue and not only at the last: Gaupol's shortcut
/// assumes a sorted file, and an unsorted one would come out with minutes past
/// fifty-nine, which no reader accepts. All the timestamps of a file are
/// written the same way — mixing the two shapes would be legal and unreadable.
class WebVttWriter final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
