#pragma once

// How the tool names things to the user.
//
// Shared rather than repeated: `inspect` and `convert` both name formats, line
// endings and failures, and two copies of that vocabulary would drift apart —
// the same file would be "SubRip" in one report and "SRT" in the next.

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::cli {

/// The name of a format, as a report writes it.
[[nodiscard]] std::string_view nameOf(subedit::core::SubtitleFormat format);

/// The extension a file of that format is expected to carry, dot included.
[[nodiscard]] std::string_view extensionOf(subedit::core::SubtitleFormat format);

/// The name of a line ending, as a report writes it.
[[nodiscard]] std::string_view nameOf(subedit::core::Newline newline);

/// Why a file could not be read at all, in the second half of a sentence
/// starting with its path.
[[nodiscard]] std::string_view reasonOf(subedit::core::ReadErrorKind kind);

/// Why the file system refused, in the same shape.
[[nodiscard]] std::string_view reasonOf(subedit::core::FileErrorKind kind);

/// What a reader ran into, as a report writes it.
///
/// A phrase and not a sentence: it is the middle of a line that already names
/// the file and the line number, and it is followed by what was done about it.
[[nodiscard]] std::string_view nameOf(subedit::core::DiagnosticKind kind);

/// What was done about an anomaly, as a report writes it.
///
/// The distinction the core draws, said out loud: one of the two was settled
/// and needs nobody, the other was left alone because only the user can decide.
[[nodiscard]] std::string_view nameOf(subedit::core::Severity severity);

/// A frame rate, as a report writes it: "25", "23.9", "24000/1001".
///
/// **A decimal only when a decimal is exact.** A rate whose denominator divides
/// a thousand is written as one, trailing zeros trimmed; anything else is
/// written as its fraction. The NTSC rates fall on the second side, which is
/// the point: naming `24000/1001` "23.976" would report a conversion that did
/// not happen, and this line is the only place the user sees which rate was
/// actually used.
[[nodiscard]] std::string nameOf(subedit::core::FrameRate rate);

/// A count and its noun, agreeing: "1 subtitle", "2 subtitles".
///
/// Small, and worth it: "1 subtitles" is the kind of sloppiness a reader
/// notices immediately and that ends up copied into the manual, which is
/// generated from what the tool actually prints.
[[nodiscard]] std::string countOf(std::size_t count, std::string_view noun);

} // namespace subedit::cli
