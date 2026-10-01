#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/video.hpp>

#include <optional>
#include <string>

namespace subedit::core {

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
