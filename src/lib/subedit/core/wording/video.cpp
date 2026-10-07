#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/video.hpp>

#include <optional>
#include <string>

namespace subedit::core {

std::string noFrameToCountBy() {
    return "no frame rate to count a frame by: the video declares none, the file is not counted in "
           "frames, and its positions fall on no grid. Associate a video that declares one.";
}

std::string videoStatusOf(const std::optional<std::filesystem::path>& video,
                          std::optional<FrameRate> declared) {
    if (!video.has_value())
        return "No video";

    std::string text = "Video: " + video->filename().string();
    if (declared.has_value())
        text += ", " + nameOf(*declared) + " fps";
    return text;
}

} // namespace subedit::core
