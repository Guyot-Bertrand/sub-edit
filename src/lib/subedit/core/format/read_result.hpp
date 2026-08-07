#pragma once

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <string>
#include <vector>

namespace subedit::core {

/// Everything a reader got out of a file: the subtitles, what the file looked
/// like, and what was wrong with it.
struct ReadResult {
    std::vector<Subtitle> subtitles{};

    /// The format of the reader that produced this result. Always set by that
    /// reader, so that a caller who did not choose it knows what it read.
    SubtitleFormat format = SubtitleFormat::SubRip;

    /// The WebVTT header, empty for the formats that have none.
    std::string header{};

    /// Detected, and kept so that writing can put it back.
    Newline newline = Newline::Lf;
    bool hadUtf8Bom = false;

    /// What the reader recovered from, in the order it was met.
    std::vector<Diagnostic> diagnostics{};
};

} // namespace subedit::core
