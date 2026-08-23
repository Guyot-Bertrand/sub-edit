#pragma once

#include <subedit/core/io/file_system.hpp>

#include <filesystem>
#include <optional>

namespace subedit::core {

/// The film that lies beside `subtitlePath`, if one can be named without
/// guessing.
///
/// The rule is Gaupol's `find_video`, read strictly: in the directory of the
/// subtitle file, a video file whose name without extension is a **prefix** of
/// the subtitle file's own name without extension — `film.mkv` for
/// `film.fr.srt`, `film.srt` or `film.en.forced.srt`.
///
/// Two departures, both because a proposal that is silently wrong costs more
/// than no proposal at all:
///
/// - **the prefix is read segment by segment.** Gaupol compares raw strings,
///   where `fil.mkv` answers for `film.fr.srt`. The boundary is a dot.
/// - **the longest match wins, and a tie proposes nothing.** `film.fr.mkv`
///   beats `film.mkv` for `film.fr.srt`, being the closer of the two. But
///   `film.mkv` and `film.mp4` cannot be told apart, and neither the order of
///   the file system — which is not stable — nor the order of our own list of
///   extensions — which nobody can read — is an answer. Silence is; D5 then
///   does the rest, since the user's own choice is never overwritten.
///
/// What this never does is open anything: it reads names. Whether the film
/// plays is the player's answer to give, and it gives it out loud.
[[nodiscard]] std::optional<std::filesystem::path>
findVideoBeside(const FileSystem& files, const std::filesystem::path& subtitlePath);

} // namespace subedit::core
