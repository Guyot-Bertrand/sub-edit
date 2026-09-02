#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::core {

/// Reads a file whose format is not known in advance, in the encoding given.
///
/// Ties together what the file is made of before any format sees it: the mark
/// of `encoding` is taken off, the bytes are decoded into UTF-8 text, the line
/// endings are looked at, and only then is the format recognised and its reader
/// called.
///
/// **The bytes settle the mark, not the caller.** `encoding` names the charset
/// to read in; whether a mark was there is what the file says, and what the
/// result records — a caller asking for `utf-8` gets `utf-8-sig` back when the
/// file carried one.
///
/// Fails — rather than guessing — when the bytes do not decode in that encoding
/// or when no format claims the content.
[[nodiscard]] std::expected<ReadResult, ReadError> readSubtitles(std::string_view content,
                                                                 const Encoding& encoding);

/// Reads it without being told an encoding.
///
/// UTF-8 today, because nothing yet proposes another and assuming the locale's
/// is how a file gets read wrong in silence — the one point of Gaupol's
/// detection this phase turns down. Detection takes this overload's place, and
/// every caller of it, in the issue that follows.
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
