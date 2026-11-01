#include <subedit/core/transcription/speech_correction.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <numbers>
#include <utility>

namespace subedit::core {

namespace {

constexpr double kMillisecondsPerSecond = 1000.0;

/// The resolution the shifts are found at, and the one the frame rate is chosen at.
constexpr int kFinePerSecond = 20;
constexpr int kCoarsePerSecond = 4;

/// A stretch of film that one shift is found for, and how often one starts.
constexpr int kWindowSeconds = 120;
constexpr int kStepSeconds = 30;
constexpr int kLeastWindowSeconds = 40;

/// The furthest a file may be from the sound, either way.
constexpr int kMostShiftSeconds = 150;

/// What a change of shift costs per second of change, and what being away from 0 costs per second
/// away: the path is the one that agrees with the speech and moves least. Settled on the
/// measurements of #820.
constexpr double kChangePenaltyPerSecond = 0.03;
constexpr double kDistancePenaltyPerSecond = 0.0004;

/// A frame rate other than 1 is taken when it correlates this many times better than 1.
constexpr double kRatioMargin = 1.15;

constexpr std::array kFrameRates{23.976, 24.0, 25.0, 29.97, 30.0};

using Curve = std::vector<double>;
using Spectrum = std::vector<std::complex<double>>;

[[nodiscard]] std::size_t powerOfTwoAtLeast(std::size_t size) {
    std::size_t power = 1;
    while (power < size)
        power <<= 1U;
    return power;
}

/// The Fourier transform of `data`, in place; its length is a power of two. `inverse` undoes it,
/// scaling included.
void transform(Spectrum& data, bool inverse) {
    const std::size_t size = data.size();
    for (std::size_t i = 1, j = 0; i < size; ++i) {
        std::size_t bit = size >> 1U;
        for (; (j & bit) != 0; bit >>= 1U)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(data[i], data[j]);
    }
    for (std::size_t length = 2; length <= size; length <<= 1U) {
        const double angle =
            2.0 * std::numbers::pi / static_cast<double>(length) * (inverse ? 1.0 : -1.0);
        const std::complex<double> step{std::cos(angle), std::sin(angle)};
        for (std::size_t start = 0; start < size; start += length) {
            std::complex<double> turn{1.0, 0.0};
            for (std::size_t k = 0; k < length / 2; ++k) {
                const std::complex<double> even = data[start + k];
                const std::complex<double> odd = data[start + k + (length / 2)] * turn;
                data[start + k] = even + odd;
                data[start + k + (length / 2)] = even - odd;
                turn *= step;
            }
        }
    }
    if (inverse)
        for (std::complex<double>& value : data)
            value /= static_cast<double>(size);
}

/// The curve of a list of stretches: 1 where one is on, 0 elsewhere, `perSecond` points a second.
template<typename Stretch, typename Bounds>
[[nodiscard]] Curve
curveOf(std::span<const Stretch> stretches, Bounds bounds, int perSecond, std::size_t size) {
    Curve curve(size, 0.0);
    for (const Stretch& stretch : stretches) {
        const auto [from, to] = bounds(stretch);
        const auto first = static_cast<std::int64_t>(std::floor(from * perSecond));
        const auto last = static_cast<std::int64_t>(std::ceil(to * perSecond));
        for (std::int64_t at = std::max<std::int64_t>(first, 0);
             at < std::min<std::int64_t>(last, static_cast<std::int64_t>(size));
             ++at)
            curve[static_cast<std::size_t>(at)] = 1.0;
    }
    return curve;
}

[[nodiscard]] double seconds(Timestamp time) {
    return static_cast<double>(time.milliseconds()) / kMillisecondsPerSecond;
}

[[nodiscard]] Curve
speechCurve(std::span<const SpeechSpan> speech, int perSecond, std::size_t size) {
    return curveOf(
        speech,
        [](const SpeechSpan& span) { return std::pair{seconds(span.start), seconds(span.end)}; },
        perSecond,
        size);
}

/// The lines as a curve, their times divided by `ratio`.
[[nodiscard]] Curve
linesCurve(std::span<const LineTimes> lines, double ratio, int perSecond, std::size_t size) {
    return curveOf(
        lines,
        [ratio](const LineTimes& line) {
            return std::pair{seconds(line.start) / ratio, seconds(line.end) / ratio};
        },
        perSecond,
        size);
}

[[nodiscard]] double sumOf(const Curve& curve, std::size_t from, std::size_t to) {
    double sum = 0.0;
    for (std::size_t at = from; at < std::min(to, curve.size()); ++at)
        sum += curve[at];
    return sum;
}

/// The correlation, for each shift from `-reach` to `reach` points, of the speech `a[from, to)`
/// with the curve `b`, normalised to lie between -1 and 1. A shift `L` says the lines are `L`
/// points later than the speech. Shifts the data does not reach are 0.
[[nodiscard]] std::vector<double>
correlation(const Curve& a, const Curve& b, std::size_t from, std::size_t to, std::size_t reach) {
    const std::size_t aLength = to - from;
    const std::size_t bFrom = from > reach ? from - reach : 0;
    const std::size_t bTo = std::min(b.size(), to + reach);
    const std::size_t bLength = bTo - bFrom;

    const double aMean = sumOf(a, from, to) / static_cast<double>(aLength);
    const double bMean = sumOf(b, bFrom, bTo) / static_cast<double>(bLength);
    const std::size_t size = powerOfTwoAtLeast(aLength + bLength);
    Spectrum left(size);
    Spectrum right(size);
    double aNorm = 0.0;
    double bNorm = 0.0;
    for (std::size_t at = 0; at < aLength; ++at) {
        const double value = a[from + at] - aMean;
        left[at] = value;
        aNorm += value * value;
    }
    for (std::size_t at = 0; at < bLength; ++at) {
        const double value = b[bFrom + at] - bMean;
        right[at] = value;
        bNorm += value * value;
    }
    std::vector<double> scores((2 * reach) + 1, 0.0);
    const double norm = std::sqrt(aNorm * bNorm);
    if (norm <= std::numeric_limits<double>::epsilon())
        return scores;

    transform(left, false);
    transform(right, false);
    for (std::size_t at = 0; at < size; ++at)
        left[at] = std::conj(left[at]) * right[at];
    transform(left, true);

    // Position `from + u` of the speech meets position `from + u + L` of the lines, which is `u +
    // (from - bFrom) + L` of the slice; a slice index below 0 is data that is not there.
    const auto offset = static_cast<std::int64_t>(from - bFrom);
    for (std::size_t slot = 0; slot < scores.size(); ++slot) {
        const std::int64_t shift =
            static_cast<std::int64_t>(slot) - static_cast<std::int64_t>(reach);
        const std::int64_t index = offset + shift;
        if (index < 0 || std::cmp_greater_equal(index, size))
            continue;
        scores[slot] = left[static_cast<std::size_t>(index)].real() / norm;
    }
    return scores;
}

[[nodiscard]] double bestScore(const std::vector<double>& scores) {
    return scores.empty() ? 0.0 : *std::ranges::max_element(scores);
}

/// The frame rates a file may have been timed for, relative to the film's: every ratio between two
/// of the usual rates, and 1.
[[nodiscard]] std::vector<double> candidateRatios() {
    std::vector<double> ratios{1.0};
    for (const double from : kFrameRates)
        for (const double to : kFrameRates)
            if (from != to)
                ratios.push_back(from / to);
    return ratios;
}

/// How well the lines, once their frame rate is put right by `ratio`, lie on the speech as a whole.
[[nodiscard]] double
globalAgreement(const Curve& speech, std::span<const LineTimes> lines, double ratio) {
    const Curve curve = linesCurve(lines, ratio, kCoarsePerSecond, speech.size());
    return bestScore(correlation(speech,
                                 curve,
                                 0,
                                 speech.size(),
                                 static_cast<std::size_t>(kMostShiftSeconds) * kCoarsePerSecond));
}

[[nodiscard]] double chooseRatio(std::span<const SpeechSpan> speech,
                                 std::span<const LineTimes> lines,
                                 std::size_t length) {
    const std::size_t size = (length + 1) * kCoarsePerSecond;
    const Curve heard = speechCurve(speech, kCoarsePerSecond, size);
    const double plain = globalAgreement(heard, lines, 1.0);
    double best = 1.0;
    double bestScoreSoFar = plain * kRatioMargin;
    for (const double ratio : candidateRatios()) {
        if (ratio == 1.0)
            continue;
        const double score = globalAgreement(heard, lines, ratio);
        if (score > bestScoreSoFar) {
            best = ratio;
            bestScoreSoFar = score;
        }
    }
    return best;
}

/// The path through `scores` (one row per window, one slot per shift) that agrees most and moves
/// least: a shift costs for being away from 0 and for differing from the window before.
[[nodiscard]] std::vector<std::size_t> bestPath(const std::vector<std::vector<double>>& scores,
                                                std::size_t reach) {
    const std::size_t windows = scores.size();
    const std::size_t slots = (2 * reach) + 1;
    const double change = kChangePenaltyPerSecond / kFinePerSecond;
    const double distance = kDistancePenaltyPerSecond / kFinePerSecond;

    std::vector<double> best(slots);
    std::vector<std::vector<std::size_t>> from(windows, std::vector<std::size_t>(slots, 0));
    for (std::size_t slot = 0; slot < slots; ++slot)
        best[slot] = -scores[0][slot] +
                     distance * std::abs(static_cast<double>(slot) - static_cast<double>(reach));
    for (std::size_t window = 1; window < windows; ++window) {
        std::vector<double> floor = best;
        std::vector<std::size_t> origin(slots);
        for (std::size_t slot = 0; slot < slots; ++slot)
            origin[slot] = slot;
        for (std::size_t slot = 1; slot < slots; ++slot)
            if (floor[slot - 1] + change < floor[slot]) {
                floor[slot] = floor[slot - 1] + change;
                origin[slot] = origin[slot - 1];
            }
        for (std::size_t slot = slots - 1; slot-- > 0;)
            if (floor[slot + 1] + change < floor[slot]) {
                floor[slot] = floor[slot + 1] + change;
                origin[slot] = origin[slot + 1];
            }
        from[window] = origin;
        for (std::size_t slot = 0; slot < slots; ++slot)
            best[slot] =
                floor[slot] - scores[window][slot] +
                distance * std::abs(static_cast<double>(slot) - static_cast<double>(reach));
    }
    std::vector<std::size_t> path(windows);
    path.back() = static_cast<std::size_t>(std::ranges::min_element(best) - best.begin());
    for (std::size_t window = windows - 1; window > 0; --window)
        path[window - 1] = from[window][path[window]];
    return path;
}

} // namespace

Timestamp SpeechCorrection::corrected(Timestamp time) const {
    if (!fitted())
        return time;
    const double scaled = static_cast<double>(time.milliseconds()) / ratio;
    const auto key = [](const SpeechShift& shift) {
        return static_cast<double>(shift.at.milliseconds());
    };
    auto shift = static_cast<double>(shifts.front().shiftMilliseconds);
    if (scaled >= key(shifts.back())) {
        shift = static_cast<double>(shifts.back().shiftMilliseconds);
    } else if (scaled > key(shifts.front())) {
        const auto after = std::ranges::upper_bound(
            shifts, scaled, {}, [&key](const SpeechShift& one) { return key(one); });
        const SpeechShift& next = *after;
        const SpeechShift& before = *(after - 1);
        const double span = key(next) - key(before);
        const double along = (scaled - key(before)) / span;
        shift = static_cast<double>(before.shiftMilliseconds) +
                along * static_cast<double>(next.shiftMilliseconds - before.shiftMilliseconds);
    }
    return Timestamp::fromMilliseconds(std::llround(scaled + shift));
}

SpeechCorrection estimateSpeechCorrection(std::span<const SpeechSpan> speech,
                                          std::span<const LineTimes> lines) {
    SpeechCorrection correction;
    if (speech.empty() || lines.empty())
        return correction;

    double horizon = 0.0;
    for (const SpeechSpan& span : speech)
        horizon = std::max(horizon, seconds(span.end));
    double linesEnd = 0.0;
    for (const LineTimes& line : lines)
        linesEnd = std::max(linesEnd, seconds(line.end));
    const auto length = static_cast<std::size_t>(std::ceil(std::max(horizon, linesEnd * 1.26))) + 1;

    correction.ratio = chooseRatio(speech, lines, length);

    const std::size_t size = length * kFinePerSecond;
    const Curve heard = speechCurve(speech, kFinePerSecond, size);
    const Curve shown = linesCurve(lines, correction.ratio, kFinePerSecond, size);
    const std::size_t reach = static_cast<std::size_t>(kMostShiftSeconds) * kFinePerSecond;
    const std::size_t window = static_cast<std::size_t>(kWindowSeconds) * kFinePerSecond;
    const std::size_t least = static_cast<std::size_t>(kLeastWindowSeconds) * kFinePerSecond;
    const std::size_t step = static_cast<std::size_t>(kStepSeconds) * kFinePerSecond;

    std::vector<std::vector<double>> scores;
    std::vector<std::size_t> centres;
    for (std::size_t centre = 0; centre < size; centre += step) {
        const std::size_t from = centre > window / 2 ? centre - (window / 2) : 0;
        const std::size_t to = std::min(size, centre + (window / 2));
        if (to - from < least)
            continue;
        if (sumOf(heard, from, to) <= 0.0 ||
            sumOf(shown, from > reach ? from - reach : 0, std::min(size, to + reach)) <= 0.0)
            continue;
        scores.push_back(correlation(heard, shown, from, to, reach));
        centres.push_back(centre);
    }
    if (scores.empty())
        return correction;

    const std::vector<std::size_t> path = bestPath(scores, reach);
    std::vector<double> agreement;
    for (std::size_t at = 0; at < path.size(); ++at) {
        const double lag = static_cast<double>(path[at]) - static_cast<double>(reach);
        const double shift = -lag * kMillisecondsPerSecond / kFinePerSecond;
        const double centre =
            static_cast<double>(centres[at]) * kMillisecondsPerSecond / kFinePerSecond;
        const SpeechShift found{.at = Timestamp::fromMilliseconds(std::llround(centre)),
                                .shiftMilliseconds = std::llround(shift)};
        correction.shifts.push_back(found);
        agreement.push_back(scores[at][path[at]]);
    }
    std::ranges::nth_element(agreement,
                             agreement.begin() + static_cast<std::ptrdiff_t>(agreement.size() / 2));
    correction.agreement = std::clamp(agreement[agreement.size() / 2], 0.0, 1.0);
    return correction;
}

} // namespace subedit::core
