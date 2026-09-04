#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
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

/// Reads it without being told an encoding, weighing the bytes for one.
///
/// **The encoding is detected here and imposed by the overload above**, which
/// is the whole of the difference between the two: `--encoding` and a window
/// that was told nothing take different doors into the same reading.
///
/// The locale's encoding is never assumed — the one point of Gaupol's detection
/// this phase turns down — and what was guessed is said rather than kept, as
/// ADR 0008 has it. UTF-8 when the bytes propose nothing at all.
[[nodiscard]] std::expected<ReadResult, ReadError> readSubtitles(std::string_view content);

/// Renders subtitles into the bytes of a file of `format`.
///
/// The counterpart of `readSubtitles`: the text is written in the encoding the
/// request names, and that encoding's byte order mark goes back on here, so
/// that no format has to know either exists.
///
/// **Fails rather than substitutes.** A character the encoding cannot write —
/// an `ń` in Latin-1 — has no right answer, and ICU's own is a `?`. Written to
/// disk over the file it came from, that `?` is text lost in silence; the
/// failure names the character instead.
[[nodiscard]] std::expected<std::string, WriteError> writeSubtitles(SubtitleFormat format,
                                                                    const WriteRequest& request);

} // namespace subedit::core
