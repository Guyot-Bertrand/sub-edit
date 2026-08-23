#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/video_file.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

constexpr char kSegmentSeparator = '.';

/// Tells whether a film named `videoStem` answers to `subtitleBase`.
///
/// Both are names stripped of their own extension: `film` and `film.fr`. The
/// film answers when the two are the same, or when the subtitle's name adds
/// segments to it — and a segment starts after a dot, never in the middle of a
/// word.
///
/// A subtitle path naming no file at all — a directory — arrives here with an
/// empty name, and answers no to everything, which is the right answer and
/// costs no guard of its own.
[[nodiscard]] bool namesTheSameWork(std::string_view videoStem, std::string_view subtitleBase) {
    if (videoStem == subtitleBase)
        return true;
    return subtitleBase.size() > videoStem.size() && subtitleBase.starts_with(videoStem) &&
           subtitleBase[videoStem.size()] == kSegmentSeparator;
}

} // namespace

std::optional<std::filesystem::path> findVideoBeside(const FileSystem& files,
                                                     const std::filesystem::path& subtitlePath) {
    const std::string base = subtitlePath.stem().string();

    const std::expected<std::vector<std::filesystem::path>, FileError> neighbours =
        files.filesIn(subtitlePath.parent_path());
    if (!neighbours.has_value())
        return std::nullopt;

    std::optional<std::filesystem::path> best;
    std::size_t bestLength = 0;
    bool tied = false;

    for (const std::filesystem::path& neighbour : *neighbours) {
        if (!isVideoFile(neighbour))
            continue;

        const std::string stem = neighbour.stem().string();
        if (!namesTheSameWork(stem, base))
            continue;

        // A longer name is a closer one, and it settles what shorter ones had
        // left tied: `film.fr.mkv` answers `film.fr.srt` whatever else the
        // directory holds under `film`.
        if (!best.has_value() || stem.size() > bestLength) {
            best = neighbour;
            bestLength = stem.size();
            tied = false;
        } else if (stem.size() == bestLength) {
            tied = true;
        }
    }

    return tied ? std::nullopt : best;
}

} // namespace subedit::core
