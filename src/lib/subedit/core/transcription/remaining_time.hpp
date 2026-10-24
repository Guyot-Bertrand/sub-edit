#pragma once

#include <chrono>
#include <deque>
#include <optional>

namespace subedit::core {

/// How long a transcription still has to run, worked out from how fast it has gone.
///
/// **Silent rather than wrong**: while too little is done to extrapolate from, it says
/// nothing. The speed is that of the last minute and not of the whole run, because the
/// first minutes carry the loading of the model and an engine that works by windows moves in
/// steps; the whole-run average would carry them to the end.
///
/// The clock is given to it and not read, so that a test fixes the time.
class RemainingTime {

public:
    using Clock = std::chrono::steady_clock;

    /// Notes that `fraction` of the work (0 to 1) was done at `now`. A fraction lower than the
    /// last is ignored: progress does not go back.
    void note(double fraction, Clock::time_point now);

    /// The time left if the work goes on as it has, or nothing while it cannot be said.
    [[nodiscard]] std::optional<std::chrono::seconds> left(Clock::time_point now) const;

private:
    struct Sample {
        Clock::time_point at;
        double fraction = 0.0;
    };

    std::deque<Sample> m_samples;
};

} // namespace subedit::core
