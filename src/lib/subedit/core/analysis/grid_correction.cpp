#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/time/ratio.hpp>

#include <cstdint>
#include <optional>

namespace subedit::core {

std::optional<Duration> shiftOntoGrid(const FrameRateDeduction& deduction) {
    if (deduction.verdict == GridVerdict::Silent)
        return std::nullopt;

    const Ratio perFrame = deduction.retained.rate.millisecondsPerFrame();
    const std::int64_t phase = deduction.retained.phase.milliseconds();

    // `phase <= perFrame / 2`, asked in whole numbers so that a frame lasting
    // 1001/24 of a millisecond is compared as it is rather than as 42.
    constexpr std::int64_t kHalves = 2;
    if (phase * kHalves * perFrame.denominator() <= perFrame.numerator())
        return Duration::fromMilliseconds(-phase);

    // Past halfway, the nearer grid line is the next one. `scale(1)` rounds the
    // frame to whole milliseconds, which is the unit the positions are written
    // in and therefore the only place the correction can land.
    return Duration::fromMilliseconds(perFrame.scale(1) - phase);
}

} // namespace subedit::core
