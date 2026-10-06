#pragma once

// Turning subtitles into a file, and saying which step refused.

#include <subedit/cli/records.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

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
/// With `dryRun` the subtitles are still turned into bytes and the size is still
/// the answer, but the system is not asked to take them: a character the
/// encoding cannot carry fails a dry run as it fails a run, and no file is made.
///
/// The failure is the **second half of a sentence** whose first half is the
/// path the caller is working on — the shape every message of this surface
/// takes.
[[nodiscard]] std::expected<std::size_t, Failure>
writeSubtitlesTo(subedit::core::FileSystem& files,
                 const std::filesystem::path& out,
                 subedit::core::SubtitleFormat format,
                 const subedit::core::WriteRequest& request,
                 bool dryRun = false);

/// What to write for `subtitles` of `document`, in the shape of `source`: its line
/// endings, its encoding and its header — **a file written back as it was found**.
///
/// Written once for the subcommands that do not change the format; `convert`
/// builds its own request, since it crosses into another one.
[[nodiscard]] subedit::core::WriteRequest
writeRequestOf(std::span<const subedit::core::Subtitle> subtitles,
               subedit::core::Document document,
               const subedit::core::SourceFile& source);

/// Says how a file that was written back as it was found went, at the levels that
/// narrate it: at level three the bytes read and written, then what the reading had
/// to decide, and at level two the shape kept.
///
/// `written` is the size or sizes, already worded (« 120 », « 120 and 98 »). Written once
/// for the three subcommands that keep the shape: a libellé changed here changes for all.
void narrateKept(const Reporter& reporter,
                 std::string_view path,
                 std::size_t bytesRead,
                 const std::string& written,
                 bool dryRun,
                 const subedit::core::SourceFile& source,
                 std::span<const subedit::core::Diagnostic> diagnostics);

} // namespace subedit::cli
