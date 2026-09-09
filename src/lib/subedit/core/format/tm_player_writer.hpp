#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes TMPlayer.
///
/// One line per subtitle, no header and no separator: `00:00:12:text`, the
/// start and nothing else. **The ends are not written, because the format has
/// nowhere to put them** — which is also why a file read and written back
/// comes out identical whatever the reading made of them.
///
/// The hour is written on two digits or on one, as the document's file had it.
class TMPlayerWriter final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
