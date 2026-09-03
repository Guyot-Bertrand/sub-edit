#pragma once

// Turning subtitles into a file, and saying which step refused.

#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

/// Writes `request` as a file of `format` at `out`, and says how much it wrote.
///
/// **Two steps that fail differently**, and this is where the two meet: the
/// subtitles become bytes, then the system takes them. A character the encoding
/// cannot write is not a disk that refuses, and only one of the two is the
/// user's to fix by choosing something else.
///
/// Written once rather than at each subcommand that writes: `convert` and the
/// five operations chained the same three calls, and the day one of them
/// learned to say why it had failed, the other would not have.
///
/// The failure is the **second half of a sentence** whose first half is the
/// path the caller is working on — the shape every message of this surface
/// takes.
[[nodiscard]] std::expected<std::size_t, std::string>
writeSubtitlesTo(subedit::core::FileSystem& files,
                 const std::filesystem::path& out,
                 subedit::core::SubtitleFormat format,
                 const subedit::core::WriteRequest& request);

} // namespace subedit::cli
