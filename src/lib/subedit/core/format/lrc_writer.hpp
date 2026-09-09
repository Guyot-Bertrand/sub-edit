#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes LRC.
///
/// The header, a blank line, then one line per subtitle: `[00:12.34]text`, the
/// start and nothing else. **The ends are not written, because the format has
/// nowhere to put them** — which is also why a file read and written back
/// comes out identical whatever the reading made of them.
///
/// **A line break is written as a space**, this format having no way to hold
/// one. That is a real loss and it does not come back: it is what the
/// conversion journal declares, and what the scene fixture shows.
class LrcWriter final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
