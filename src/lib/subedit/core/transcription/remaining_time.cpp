#include <subedit/core/transcription/remaining_time.hpp>

#include <algorithm>
#include <cmath>

namespace subedit::core {

namespace {

/// How far back the speed is measured.
constexpr std::chrono::seconds kWindow{60};

/// Below this much done, or this much time spent, an estimate is a guess.
constexpr double kMinimumFraction = 0.02;
constexpr std::chrono::seconds kMinimumSpan{5};

} // namespace

void RemainingTime::note(double fraction, Clock::time_point now) {
    if (!m_samples.empty() && fraction < m_samples.back().fraction)
        return;
    m_samples.push_back({.at = now, .fraction = fraction});
    // Keeps one sample older than the window, as the origin of the speed.
    while (m_samples.size() > 2 && now - m_samples[1].at >= kWindow)
        m_samples.pop_front();
}

std::optional<std::chrono::seconds> RemainingTime::left(Clock::time_point now) const {
    if (m_samples.size() < 2)
        return std::nullopt;
    const Sample& last = m_samples.back();
    const Sample& first = m_samples.front();
    if (last.fraction < kMinimumFraction || last.at - first.at < kMinimumSpan)
        return std::nullopt;
    const double gained = last.fraction - first.fraction;
    if (gained <= 0.0)
        return std::nullopt;

    const double span = std::chrono::duration<double>(last.at - first.at).count();
    const double perUnit = span / gained;
    const double since = std::chrono::duration<double>(now - last.at).count();
    const double remaining = ((1.0 - last.fraction) * perUnit) - since;
    return std::chrono::seconds{
        static_cast<std::chrono::seconds::rep>(std::llround(std::max(remaining, 0.0)))};
}

} // namespace subedit::core
