#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// Where a clean grid starts, and where any structure at all stops.
///
/// The two numbers of the whole phase, written here and nowhere else. Ninety
/// is where the eight perfect fixtures sit — none falls below 99.4 — and fifty
/// is above every reading a file without a grid produced: the loudest is 15.3.
/// The gap between the two families is so wide that the exact placement is not
/// delicate, which is the property that makes a threshold acceptable at all.
constexpr double kCleanGrid = 90.0;
constexpr double kPartialGrid = 50.0;

/// Below this many starts, no verdict is honest.
///
/// The noise floor of the concentration is one over the square root of the
/// count: four points reach fifty by chance alone, which is the very threshold
/// that would call them a partial grid. Ten puts the floor near thirty-one, far
/// enough below fifty for the reading to mean something.
constexpr std::size_t kFewestStarts = 10;

/// How far a start may sit from the grid before it counts as leaving it.
///
/// One millisecond, because that is the unit positions are written in: a grid
/// at 23.976 lands on thirds of a millisecond, so its starts scatter by up to
/// half a one through rounding alone. Anything beyond is a position somebody
/// moved.
constexpr double kStrayMilliseconds = 1.0;

/// How much of a turn the phase must sweep for two candidates to be told apart.
///
/// A quarter, and it is not arbitrary: positions spread evenly over `x` turns
/// reach a concentration of about `sin(pi x) / (pi x)`, which crosses
/// `kCleanGrid` at roughly a quarter of a turn. Below that, the wrong candidate
/// still looks clean, and saying so is the point.
constexpr double kTurnsToSeparate = 0.25;

/// The three pairs where one rate is the whole multiple of the other, lower
/// first.
///
/// There are no others in the normalised set: 24 and 60 stand in a ratio of two
/// and a half, so only every second frame coincides. That is why this table can
/// be written out rather than computed — three lines, and a fourth would mean
/// the set of candidates had changed.
constexpr std::array<std::pair<StandardFrameRate, StandardFrameRate>, 3> kHarmonicPairs = {{
    {StandardFrameRate::Fps25, StandardFrameRate::Fps50},
    {StandardFrameRate::Fps29970, StandardFrameRate::Fps59940},
    {StandardFrameRate::Fps30, StandardFrameRate::Fps60},
}};

constexpr std::int64_t kMillisecondsPerSecond = 1000;

/// One full turn, in radians. Named because a phase is an angle here, and
/// `2.0 * pi` written inline reads as arithmetic rather than as a turn.
constexpr double kFullTurn = 2.0 * std::numbers::pi;

/// Half a turn, as the fraction the wrapping below folds around.
constexpr double kHalfTurn = 0.5;

[[nodiscard]] GridVerdict bandOf(double concentration) {
    if (concentration >= kCleanGrid)
        return GridVerdict::Clean;
    if (concentration >= kPartialGrid)
        return GridVerdict::Partial;
    return GridVerdict::Silent;
}

/// A phase, kept as the whole fraction it really is.
///
/// The remainder is taken **before** any division: that is what keeps a
/// two-hour file as precise as a two-second one, where dividing first would
/// spend the mantissa on the integer part of the frame count. It is also what
/// lets `directionOf` recognise the case below.
struct Phase {
    std::int64_t part;  ///< in `[0, whole)`
    std::int64_t whole; ///< how many parts make one turn
};

[[nodiscard]] Phase phaseOf(Timestamp position, FrameRate rate) {
    const std::int64_t frames = position.milliseconds() * rate.numerator();
    const std::int64_t whole = kMillisecondsPerSecond * rate.denominator();

    std::int64_t part = frames % whole;
    if (part < 0)
        part += whole;

    return Phase{.part = part, .whole = whole};
}

[[nodiscard]] double turnOf(Timestamp position, FrameRate rate) {
    const Phase phase = phaseOf(position, rate);
    return static_cast<double>(phase.part) / static_cast<double>(phase.whole);
}

/// The thousand directions a position can point to on a whole-numbered rate.
///
/// **A position is a whole number of milliseconds**, and a rate of `R` whole
/// frames per second divides a millisecond into `R/1000` of a frame: the phase
/// is therefore `(t·R mod 1000) / 1000`, which takes **a thousand values at
/// most**, whatever the position and whatever the rate.
///
/// Five of the eight candidates are whole — 24, 25, 30, 50 and 60 — so this
/// table answers five eighths of the work by lookup. It is **exact**, not an
/// approximation: the entries are the very angles the direct computation would
/// have produced. The three NTSC rates divide by 1001 and fall through to the
/// library, as they must.
///
/// **The table halves the cost**, measured back to back on four thousand
/// subtitles. No figure in seconds is written here on purpose: it would date
/// the day the machine changes, and `docs/mesures/performances.md` already
/// carries the measurement, version by version, under `deduce the frame rate`.
///
/// What remains after it is the three NTSC rates, which have no such property,
/// and the whole-number remainders — a division by a value the compiler cannot
/// see.
[[nodiscard]] const std::array<std::pair<double, double>, kMillisecondsPerSecond>&
wholeRateTable() {
    static const std::array<std::pair<double, double>, kMillisecondsPerSecond> table = [] {
        std::array<std::pair<double, double>, kMillisecondsPerSecond> built{};
        for (std::size_t part = 0; part < built.size(); ++part) {
            const double angle = 2.0 * std::numbers::pi * static_cast<double>(part) /
                                 static_cast<double>(kMillisecondsPerSecond);
            built[part] = {std::cos(angle), std::sin(angle)};
        }
        return built;
    }();
    return table;
}

/// The unit vector a position points to on `rate`'s grid.
[[nodiscard]] std::pair<double, double> directionOf(Timestamp position, FrameRate rate) {
    const Phase phase = phaseOf(position, rate);
    if (phase.whole == kMillisecondsPerSecond)
        return wholeRateTable()[static_cast<std::size_t>(phase.part)];

    const double angle =
        2.0 * std::numbers::pi * static_cast<double>(phase.part) / static_cast<double>(phase.whole);
    return {std::cos(angle), std::sin(angle)};
}

[[nodiscard]] double millisecondsPerFrameOf(FrameRate rate) {
    const Ratio perFrame = rate.millisecondsPerFrame();
    return static_cast<double>(perFrame.numerator()) / static_cast<double>(perFrame.denominator());
}

[[nodiscard]] double framesPerSecondOf(FrameRate rate) {
    return static_cast<double>(rate.numerator()) / static_cast<double>(rate.denominator());
}

/// Fits one candidate: the length of the mean unit vector, and its direction.
[[nodiscard]] GridFit fitOf(std::span<const Timestamp> starts, FrameRate rate) {
    if (starts.empty())
        return GridFit{.rate = rate, .concentration = 0.0, .phase = Duration::zero()};

    double alongX = 0.0;
    double alongY = 0.0;
    for (const Timestamp start : starts) {
        const auto [cosine, sine] = directionOf(start, rate);
        alongX += cosine;
        alongY += sine;
    }

    const auto count = static_cast<double>(starts.size());
    const double concentration = 100.0 * std::hypot(alongX, alongY) / count;

    double meanTurn = std::atan2(alongY, alongX) / kFullTurn;
    if (meanTurn < 0.0)
        meanTurn += 1.0;

    // Rounded to whole milliseconds, which is the unit positions are written
    // in — and then wrapped, because the rounding can land on the frame length
    // itself. A phase of one whole frame is a phase of none, and reporting it
    // as forty milliseconds would send a reader looking for a shift that is not
    // there.
    const std::int64_t perFrame = rate.millisecondsPerFrame().scale(1);
    std::int64_t phase = std::llround(meanTurn * millisecondsPerFrameOf(rate));
    if (phase >= perFrame)
        phase -= perFrame;

    return GridFit{
        .rate = rate,
        .concentration = concentration,
        .phase = Duration::fromMilliseconds(phase),
    };
}

/// Fits all eight candidates, in the order `kStandardFrameRates` lists them.
///
/// Built through an index sequence rather than eight written-out calls:
/// `GridFit` has no default state — a `FrameRate` is always some rate — so the
/// array cannot be filled after the fact, and eight literal indices would be
/// eight numbers to keep in step with the table.
template<std::size_t... Index>
[[nodiscard]] std::array<GridFit, sizeof...(Index)>
fitsOf(std::span<const Timestamp> starts, std::index_sequence<Index...> /*which*/) {
    return {fitOf(starts, FrameRate{kStandardFrameRates[Index]})...};
}

[[nodiscard]] std::array<GridFit, kStandardFrameRates.size()>
fitsOf(std::span<const Timestamp> starts) {
    return fitsOf(starts, std::make_index_sequence<kStandardFrameRates.size()>{});
}

/// The other member of `rate`'s harmonic pair, and which of the two is lower.
struct Harmonic {
    FrameRate lower;
    FrameRate higher;
};

[[nodiscard]] std::optional<Harmonic> harmonicOf(FrameRate rate) {
    for (const auto& [low, high] : kHarmonicPairs) {
        const FrameRate lower{low};
        const FrameRate higher{high};
        if (rate == lower || rate == higher)
            return Harmonic{.lower = lower, .higher = higher};
    }
    return std::nullopt;
}

[[nodiscard]] const GridFit& fitFor(const std::array<GridFit, kStandardFrameRates.size()>& ranked,
                                    FrameRate rate) {
    const auto* const found =
        std::ranges::find_if(ranked, [rate](const GridFit& fit) { return fit.rate == rate; });
    // The set of candidates is closed, so every rate handed here is in it.
    return *found;
}

/// The starts that leave `fit`'s grid by more than `kStrayMilliseconds`.
[[nodiscard]] std::vector<SubtitleIndex> straysOf(std::span<const Timestamp> starts,
                                                  const GridFit& fit) {
    const double perFrame = millisecondsPerFrameOf(fit.rate);
    const double meanTurn = static_cast<double>(fit.phase.milliseconds()) / perFrame;

    std::vector<SubtitleIndex> strays;
    for (std::size_t index = 0; index < starts.size(); ++index) {
        double away = turnOf(starts[index], fit.rate) - meanTurn;
        // Back into the half-open turn centred on zero: a start just before the
        // grid line is one millisecond away, not a whole frame away.
        away -= std::floor(away + kHalfTurn);

        if (std::abs(away) * perFrame > kStrayMilliseconds)
            strays.push_back(SubtitleIndex::fromValue(index));
    }
    return strays;
}

/// The candidates `span` is too short to tell apart from `retained`.
[[nodiscard]] std::vector<FrameRate> notSeparatedFrom(FrameRate retained, Duration span) {
    const double seconds = static_cast<double>(span.milliseconds()) / 1000.0;

    std::vector<FrameRate> tooClose;
    for (const StandardFrameRate standard : kStandardFrameRates) {
        const FrameRate candidate{standard};
        if (candidate == retained)
            continue;

        const double apart = std::abs(framesPerSecondOf(candidate) - framesPerSecondOf(retained));
        if (seconds * apart < kTurnsToSeparate)
            tooClose.push_back(candidate);
    }
    return tooClose;
}

} // namespace

FrameRateDeduction deduceFrameRate(std::span<const Timestamp> starts) {
    std::array<GridFit, kStandardFrameRates.size()> ranked = fitsOf(starts);
    std::ranges::sort(ranked, [](const GridFit& left, const GridFit& right) {
        return left.concentration > right.concentration;
    });

    const Duration span = starts.empty()
                              ? Duration::zero()
                              : Duration::fromMilliseconds(starts.back().milliseconds() -
                                                           starts.front().milliseconds());

    FrameRateDeduction deduction{
        .ranked = ranked,
        .retained = ranked.front(),
        .verdict = GridVerdict::Silent,
        .span = span,
        .starts = starts.size(),
    };

    // Too few starts and the noise floor alone would earn a verdict. Everything
    // above is still reported — the ranking, the span — because a caller may
    // want to show it; only the conclusion is withheld.
    if (starts.size() < kFewestStarts)
        return deduction;

    deduction.enoughStarts = true;

    // The harmonic rule, and the reason it compares bands rather than a margin:
    // a file truly on the higher rate sees the lower one collapse to the noise,
    // so the two land in different bands. A file on the lower rate puts both in
    // the same one, whatever rounding did to their order.
    if (const std::optional<Harmonic> pair = harmonicOf(ranked.front().rate)) {
        const GridFit& lower = fitFor(ranked, pair->lower);
        const GridFit& higher = fitFor(ranked, pair->higher);

        if (bandOf(lower.concentration) == bandOf(higher.concentration) &&
            bandOf(lower.concentration) != GridVerdict::Silent) {
            deduction.retained = lower;
            deduction.harmonic = higher.rate;
        }
    }

    deduction.verdict = bandOf(deduction.retained.concentration);
    if (deduction.verdict == GridVerdict::Silent)
        return deduction;

    deduction.notSeparated = notSeparatedFrom(deduction.retained.rate, span);
    deduction.strays = straysOf(starts, deduction.retained);
    return deduction;
}

std::size_t runsOfStrays(const FrameRateDeduction& deduction) {
    std::size_t runs = 0;
    for (std::size_t rank = 0; rank < deduction.strays.size(); ++rank) {
        const bool opensARun =
            rank == 0 || deduction.strays[rank].value() != deduction.strays[rank - 1].value() + 1;
        if (opensARun)
            ++runs;
    }
    return runs;
}

} // namespace subedit::core
