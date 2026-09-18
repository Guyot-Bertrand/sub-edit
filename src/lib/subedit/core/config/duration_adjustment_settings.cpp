#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/time/duration.hpp>

#include <optional>

namespace subedit::core {

namespace {

/// The duration of `milliseconds` when `enabled`, and nothing otherwise.
[[nodiscard]] std::optional<Duration> whenEnabled(bool enabled, std::int64_t milliseconds) {
    if (!enabled)
        return std::nullopt;
    return Duration::fromMilliseconds(milliseconds);
}

} // namespace

DurationConstraints constraintsOf(const DurationAdjustmentSettings& settings) {
    std::optional<ReadingSpeed> speed;
    if (settings.lengthen || settings.shorten)
        speed =
            ReadingSpeed::create(settings.charactersPerSecond, settings.lengthen, settings.shorten);

    return DurationConstraints{
        .speed = speed,
        .minimum = whenEnabled(settings.minimumEnabled, settings.minimumMilliseconds),
        .maximum = whenEnabled(settings.maximumEnabled, settings.maximumMilliseconds),
        .gap = whenEnabled(settings.gapEnabled, settings.gapMilliseconds)};
}

} // namespace subedit::core
