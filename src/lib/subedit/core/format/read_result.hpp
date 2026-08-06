#pragma once

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <string>
#include <vector>

namespace subedit::core {

/// Everything a reader got out of a file: the subtitles, what the file looked
/// like, and what was wrong with it.
struct ReadResult {
    std::vector<Subtitle> subtitles{};

    /// The WebVTT header, empty for the formats that have none.
    std::string header{};

    /// Detected, and kept so that writing can put it back.
    Newline newline = Newline::Lf;
    bool hadUtf8Bom = false;

    /// What the reader recovered from, in the order it was met.
    std::vector<Diagnostic> diagnostics{};
};

} // namespace subedit::core
