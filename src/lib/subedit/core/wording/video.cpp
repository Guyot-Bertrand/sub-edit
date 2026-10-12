#include <subedit/core/i18n/messages.hpp>
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
    return std::string{
        translate("positions cannot be shown in frames: the file is not counted in frames, the "
                  "video declares no frame rate, and the positions fall on no grid")};
}

namespace {

/// Where the rate came from, as the sentence that names it reads.
[[nodiscard]] std::string_view sourceOf(FrameRateSource source) {
    switch (source) {
    case FrameRateSource::Video:
        return translate("the rate the video declares");
    case FrameRateSource::Document:
        return translate("the rate of the file");
    case FrameRateSource::Grid:
        return translate("the grid the positions fall on");
    }
    std::unreachable();
}

} // namespace

std::string framesShownAt(const CountedFrameRate& counted) {
    // TRANSLATORS: %1 is a frame rate, %2 where it came from, such as « the
    // rate the video declares ».
    return substitute(translate("Positions are frame numbers, counted at %1 fps, %2"),
                      {nameOf(counted.rate), sourceOf(counted.source)});
}

std::string noFrameToCountBy() {
    return std::string{
        translate("no frame rate to count a frame by: the video declares none, the file is not "
                  "counted in frames, and its positions fall on no grid. Associate a video that "
                  "declares one.")};
}

std::string videoStatusOf(const std::optional<std::filesystem::path>& video,
                          std::optional<FrameRate> declared) {
    if (!video.has_value())
        return std::string{translate("No video")};

    if (declared.has_value())
        // TRANSLATORS: %1 is the file name of a video, %2 the frame rate it declares.
        return substitute(translate("Video: %1, %2 fps"),
                          {video->filename().string(), nameOf(*declared)});
    // TRANSLATORS: %1 is the file name of a video.
    return substitute(translate("Video: %1"), {video->filename().string()});
}

} // namespace subedit::core
