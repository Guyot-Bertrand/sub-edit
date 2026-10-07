#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/video/frame_step.hpp>

#include <algorithm>
#include <cstdint>
#include <variant>

namespace subedit::core {

namespace {

/// The rate the video declares, if one is associated and it says.
[[nodiscard]] std::optional<CountedFrameRate> declaredBy(const Project& project) {
    if (const std::optional<AssociatedVideo>& video = project.video();
        video.has_value() && video->declared.has_value())
        return CountedFrameRate{.rate = *video->declared, .source = FrameRateSource::Video};
    return std::nullopt;
}

/// The rate of a document that is counted in frames, if it is.
[[nodiscard]] std::optional<CountedFrameRate> countedBy(const Project& project) {
    if (const auto* counted = std::get_if<MicroDvdFile>(&project.sourceFile().extras))
        return CountedFrameRate{.rate = counted->rate, .source = FrameRateSource::Document};
    return std::nullopt;
}

/// The grid the positions fall on, if they fall on one. A silent verdict has no grid to count by:
/// `shiftOntoGrid` answers nothing for it, and is the one place that says what a usable grid is.
[[nodiscard]] std::optional<CountedFrameRate> deducedFrom(const Project& project) {
    const FrameRateDeduction deduced = deduceFrameRate(project);
    if (!shiftOntoGrid(deduced).has_value())
        return std::nullopt;
    return CountedFrameRate{.rate = deduced.retained.rate, .source = FrameRateSource::Grid};
}

} // namespace

std::optional<CountedFrameRate> countedFrameRateOf(const Project& project) {
    if (const auto found = declaredBy(project))
        return found;
    if (const auto found = countedBy(project))
        return found;
    return deducedFrom(project);
}

std::optional<CountedFrameRate> numberingFrameRateOf(const Project& project) {
    if (const auto found = countedBy(project))
        return found;
    if (const auto found = declaredBy(project))
        return found;
    return deducedFrom(project);
}

Timestamp movedByFrames(Timestamp position, FrameRate rate, int frames) {
    const std::int64_t by = rate.millisecondsPerFrame().scale(frames);
    return Timestamp::fromMilliseconds(std::max<std::int64_t>(position.milliseconds() + by, 0));
}

} // namespace subedit::core
