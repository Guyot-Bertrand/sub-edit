#include <subedit/core/video/frame_step.hpp>
#include <subedit/core/video/seeking.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace subedit::core {

namespace {

/// How near a reported rate must be to a standard one, relatively, to be that one.
constexpr double kStandardTolerance = 1e-4;

/// Rates that are not standard are read to this many parts of a frame per second.
constexpr std::int64_t kThousandths = 1000;

} // namespace

std::optional<Timestamp>
neighbourStart(std::span<const Subtitle> subtitles, Timestamp where, bool next) {
    std::optional<Timestamp> found;
    for (const Subtitle& subtitle : subtitles) {
        if (next &&
            subtitle.start.milliseconds() > where.milliseconds() + kNeighbourMarginMilliseconds) {
            if (!found.has_value() || subtitle.start < *found)
                found = subtitle.start;
        } else if (!next && subtitle.end.milliseconds() <
                                where.milliseconds() - kNeighbourMarginMilliseconds) {
            if (!found.has_value() || subtitle.start > *found)
                found = subtitle.start;
        }
    }
    return found;
}

std::optional<std::size_t>
rowFrom(std::span<const Subtitle> subtitles, Timestamp where, bool next) {
    if (subtitles.empty())
        return std::nullopt;

    // Walked from the front for the next, from the back for the previous.
    for (std::size_t at = 0; at < subtitles.size(); ++at) {
        const std::size_t row = next ? at : subtitles.size() - 1 - at;
        if (next ? subtitles[row].start > where : subtitles[row].start < where)
            return row;
    }
    return next ? subtitles.size() - 1 : 0;
}

Insertion insertionAt(std::span<const Subtitle> subtitles, Timestamp where) {
    const auto rank = static_cast<std::size_t>(std::ranges::count_if(
        subtitles, [where](const Subtitle& subtitle) { return subtitle.start <= where; }));

    Timestamp end = where + Duration::fromMilliseconds(kInsertedLengthMilliseconds);
    if (rank < subtitles.size() && subtitles[rank].start < end)
        end = subtitles[rank].start;
    return Insertion{.rank = rank, .end = end};
}

Timestamp jumpedBy(Timestamp where, Duration length, int seconds, int direction) {
    constexpr std::int64_t kMillisecondsPerSecond = 1000;
    const std::int64_t jump = static_cast<std::int64_t>(seconds) * kMillisecondsPerSecond;
    return Timestamp::fromMilliseconds(std::clamp<std::int64_t>(
        where.milliseconds() + (direction < 0 ? -jump : jump), 0, length.milliseconds()));
}

Timestamp withLeadIn(Timestamp edge, std::int64_t contextMilliseconds) {
    return Timestamp::fromMilliseconds(
        std::max<std::int64_t>(edge.milliseconds() - contextMilliseconds, 0));
}

Timestamp steppedTo(Timestamp here, Duration length, FrameRate rate, int frames) {
    const std::int64_t oneFrame = rate.millisecondsPerFrame().scale(1);
    const std::int64_t last = std::max<std::int64_t>(length.milliseconds() - oneFrame, 0);
    const Timestamp moved = movedByFrames(here, rate, frames);
    return Timestamp::fromMilliseconds(std::clamp<std::int64_t>(moved.milliseconds(), 0, last));
}

std::optional<FrameRate> frameRateNear(double framesPerSecond) {
    if (!std::isfinite(framesPerSecond) || framesPerSecond <= 0.0)
        return std::nullopt;

    for (const StandardFrameRate standard : kStandardFrameRates) {
        const FrameRate rate{standard};
        const double value = static_cast<double>(rate.framesPerSecond().numerator()) /
                             static_cast<double>(rate.framesPerSecond().denominator());
        if (std::abs(framesPerSecond - value) <= value * kStandardTolerance)
            return rate;
    }
    return FrameRate::create(std::llround(framesPerSecond * static_cast<double>(kThousandths)),
                             kThousandths);
}

} // namespace subedit::core
