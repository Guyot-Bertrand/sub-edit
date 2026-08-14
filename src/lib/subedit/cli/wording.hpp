#pragma once

// How the tool names things to the user.
//
// Shared rather than repeated: `inspect` and `convert` both name formats, line
// endings and failures, and two copies of that vocabulary would drift apart —
// the same file would be "SubRip" in one report and "SRT" in the next.

#include <subedit/core/format/file_system.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/model/source_file.hpp>

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

/// A count and its noun, agreeing: "1 subtitle", "2 subtitles".
///
/// Small, and worth it: "1 subtitles" is the kind of sloppiness a reader
/// notices immediately and that ends up copied into the manual, which is
/// generated from what the tool actually prints.
[[nodiscard]] std::string countOf(std::size_t count, std::string_view noun);

} // namespace subedit::cli
