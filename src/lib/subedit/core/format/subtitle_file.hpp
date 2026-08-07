#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/format/subtitle_writer.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::core {

/// Whether a byte order mark precedes the text of a file.
///
/// An enumeration rather than a boolean: `writeSubtitles(format, request,
/// true)` says nothing at the call site about what is true.
enum class Utf8Bom {
    Absent,
    Present,
};

/// Reads a file whose format is not known in advance.
///
/// Ties together what the file is made of before any format sees it: the bytes
/// are checked as UTF-8, the byte order mark is taken off, the line endings are
/// looked at, and only then is the format recognised and its reader called.
///
/// Fails — rather than guessing — when the bytes are not UTF-8 or when no
/// format claims the content.
[[nodiscard]] std::expected<ReadResult, ReadError> readSubtitles(std::string_view content);

/// Renders subtitles into the text of a file of `format`.
///
/// The counterpart of `readSubtitles`: the byte order mark goes back on here,
/// so that no format has to know it exists.
[[nodiscard]] std::string
writeSubtitles(SubtitleFormat format, const WriteRequest& request, Utf8Bom bom);

} // namespace subedit::core
