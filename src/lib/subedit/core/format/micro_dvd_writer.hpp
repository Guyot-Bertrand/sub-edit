#pragma once

#include <subedit/core/format/subtitle_writer.hpp>

#include <string>

namespace subedit::core {

/// Writes MicroDVD.
///
/// **The rate comes from the document**, where the reading put it — ADR 0030.
/// Read at a rate and written at the same one, a file gives back the frame
/// numbers it arrived with, whatever that rate was: the two conversions cancel.
///
/// A document that came from a time format carries no rate, and is written at
/// the declared default. That is a conversion, and it moves every position; the
/// command line refuses it without a rate rather than choose one, and only the
/// library's own default stands here.
class MicroDvdWriter final : public SubtitleWriter {

public:
    [[nodiscard]] std::string write(const WriteRequest& request) const override;
};

} // namespace subedit::core
