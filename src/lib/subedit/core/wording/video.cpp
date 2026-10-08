#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/video.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

std::string frameNumberText(Timestamp position, FrameRate rate) {
    return std::to_string(position.toFrame(rate).number());
}

std::string noFrameRateToShow() {
    return "positions cannot be shown in frames: the file is not counted in frames, the video "
           "declares no frame rate, and the positions fall on no grid";
}

namespace {

/// Where the rate came from, as the sentence that names it reads.
[[nodiscard]] std::string_view sourceOf(FrameRateSource source) {
    switch (source) {
    case FrameRateSource::Video:
        return "the rate the video declares";
    case FrameRateSource::Document:
        return "the rate of the file";
    case FrameRateSource::Grid:
        return "the grid the positions fall on";
    }
    std::unreachable();
}

} // namespace

std::string framesShownAt(const CountedFrameRate& counted) {
    return "Positions are frame numbers, counted at " + std::string{nameOf(counted.rate)} +
           " fps, " + std::string{sourceOf(counted.source)};
}

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
