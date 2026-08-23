#include <subedit/core/model/video_file.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

/// Gaupol's `VIDEO_FILE_EXTENSIONS`, unchanged.
///
/// Iso-functionality decides what belongs here, and nothing else: adding the
/// ones somebody happens to own would make the two programs disagree on the
/// same directory, and there would be no arguing which is right.
constexpr std::array<std::string_view, 10> kVideoExtensions{
    ".avi", ".flv", ".m2ts", ".mkv", ".mov", ".mp4", ".ogg", ".ogv", ".vob", ".webm"};

[[nodiscard]] std::string lowered(std::string_view text) {
    std::string result{text};
    std::ranges::transform(result, result.begin(), [](unsigned char letter) {
        return static_cast<char>(std::tolower(letter));
    });
    return result;
}

} // namespace

bool isVideoFile(const std::filesystem::path& path) {
    // `.mkv` alone is a hidden file whose whole name is `.mkv`, and
    // `std::filesystem` says so by answering that it has no extension. Which
    // is the answer we want: a dotfile is not a film.
    const std::string extension = lowered(path.extension().string());
    return std::ranges::find(kVideoExtensions, extension) != kVideoExtensions.end();
}

} // namespace subedit::core
