#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::core {

/// Reads a file whose format is not known in advance.
///
/// Ties together what the file is made of before any format sees it: the bytes
/// are checked as UTF-8, the byte order mark is taken off, the line endings are
/// looked at, and only then is the format recognised and its reader called.
///
/// Fails — rather than guessing — when the bytes are not UTF-8 or when no
/// format claims the content.
[[nodiscard]] std::expected<ReadResult, ReadError> readSubtitles(std::string_view content);

/// Renders subtitles into the bytes of a file of `format`.
///
/// The counterpart of `readSubtitles`: the byte order mark of the requested
/// encoding goes back on here, so that no format has to know it exists.
///
/// **The text itself is UTF-8, whatever encoding the request names.** Nothing
/// can name another one yet — reading only ever produces UTF-8 — and the
/// converter that would honour it comes with the phase that writes the others.
[[nodiscard]] std::string writeSubtitles(SubtitleFormat format, const WriteRequest& request);

} // namespace subedit::core
