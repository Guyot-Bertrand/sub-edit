#pragma once

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
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

    /// Detected, and kept so that writing can put them back.
    Newline newline = Newline::Lf;
    Encoding encoding = Encoding::utf8(ByteOrderMark::Absent);

    /// What the reader recovered from, in the order it was met.
    std::vector<Diagnostic> diagnostics{};
};

/// Builds what a project has to remember of the file it came from.
///
/// The counterpart of reading: everything a writer needs to put the file back
/// as it was — its format, its header, its line endings, its encoding and the
/// mark that encoding carries — plus where it lives. An empty `path` is a
/// document that was never on disk.
///
/// A free function and not a member: `ReadResult` is what a reader produces,
/// `SourceFile` is what the model keeps, and neither has to know the other's
/// reasons.
[[nodiscard]] inline SourceFile sourceFileOf(const ReadResult& result, std::filesystem::path path) {
    return SourceFile{
        .path = path.empty() ? std::optional<std::filesystem::path>{}
                             : std::optional<std::filesystem::path>{std::move(path)},
        .format = result.format,
        .newline = result.newline,
        .encoding = result.encoding,
        .header = result.header,
    };
}

} // namespace subedit::core
