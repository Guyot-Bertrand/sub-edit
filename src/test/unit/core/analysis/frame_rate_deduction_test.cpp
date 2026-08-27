// The deduction, held to the table its fixtures publish.
//
// `src/test/data/grilles/LISEZMOI.md` carries two tables: what each fixture is,
// and what each one gives on the eight candidates. The numbers below are that
// second table, and `./src/scripts/subtitle-fixtures.py --measure` reproduces
// it. They are constants and not a matcher: thirteen fixtures and eight
// candidates make a table of numbers, and a table reads better than a verb.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <grid_fixtures.hpp>
#include <span>
#include <string_view>
#include <vector>

namespace {

using subedit::core::deduceFrameRate;
using subedit::core::FrameRate;
using subedit::core::FrameRateDeduction;
using subedit::core::GridVerdict;
using subedit::core::Project;
using subedit::core::runsOfStrays;
using subedit::core::StandardFrameRate;
using subedit::core::Timestamp;

FrameRateDeduction deductionOf(std::string_view name) {
    return deduceFrameRate(subedit::test::gridStarts(name));
}

/// The concentration a candidate reaches on a fixture, or zero if the rate is
/// not among the eight — which cannot happen, the set being closed.
double concentrationOn(const FrameRateDeduction& deduction, StandardFrameRate standard) {
    const FrameRate wanted{standard};
    const auto* const found = std::ranges::find_if(
        deduction.ranked, [wanted](const auto& fit) { return fit.rate == wanted; });
    return found == deduction.ranked.end() ? 0.0 : found->concentration;
}

/// A perfect 25 fps grid of exactly `count` starts, with an irregular step.
///
/// The count is the whole subject: `kFewestStarts` decides from how many starts
/// a verdict is honest, and nothing but the count may differ between the two
/// sides of that frontier.
[[nodiscard]] std::vector<Timestamp> gridOf(std::size_t count) {
    constexpr std::array<std::int64_t, 3> kGapsInFrames = {47, 61, 53};
    constexpr std::int64_t kFrame = 40; // one frame at 25 fps, in milliseconds

    std::vector<Timestamp> starts;
    std::int64_t frame = 0;
    for (std::size_t rank = 0; rank < count; ++rank) {
        starts.push_back(Timestamp::fromMilliseconds(frame * kFrame));
        frame += kGapsInFrames[rank % kGapsInFrames.size()];
    }
    return starts;
}

/// Positions whose concentration on the 25 fps grid is exactly
/// `aligned / (aligned + 2 × pairs)`, in per cent.
///
/// **Chosen to the digit rather than measured afterwards**, and that is what
/// makes it usable on a frontier. A frame at 25 lasts 40 milliseconds, so a
/// position's phase is `(t mod 40) / 40`: positions at `t ≡ 0` all point the
/// same way, while a pair at `t ≡ 10` and `t ≡ 30` points a quarter turn either
/// side and **cancels exactly** — the two unit vectors are opposite. What
/// survives the sum is the aligned ones, over the count.
///
/// The thirteen fixtures cannot do this. They fall frankly on one side or the
/// other — the lowest clean grid is at 99.4, the loudest silent one at 15.3 —
/// so they show that the method separates clear cases, not that a threshold is
/// where the spec says it is.
[[nodiscard]] std::vector<Timestamp> concentratedOn25(std::size_t aligned, std::size_t pairs) {
    constexpr std::int64_t kFrame = 40;
    constexpr std::int64_t kQuarter = 10;

    std::vector<Timestamp> starts;
    std::int64_t at = 0;
    for (std::size_t rank = 0; rank < aligned; ++rank) {
        starts.push_back(Timestamp::fromMilliseconds(at));
        at += kFrame;
    }
    for (std::size_t rank = 0; rank < pairs; ++rank) {
        starts.push_back(Timestamp::fromMilliseconds(at + kQuarter));
        at += kFrame;
        starts.push_back(Timestamp::fromMilliseconds(at + (3 * kQuarter)));
        at += kFrame;
    }
    return starts;
}

/// A grid at `rate`, translated by `offset`, with an irregular step.
///
/// The step alternates rather than repeating: a constant one makes the phases
/// periodic, and a periodic phase can concentrate on a candidate the file was
/// never written on.
std::vector<Timestamp> translatedGrid(StandardFrameRate rate, std::int64_t offset) {
    constexpr std::array<std::int64_t, 3> kGapsInFrames = {47, 61, 53};
    constexpr std::int64_t kLastFrame = 7200;

    const FrameRate exact{rate};
    std::vector<Timestamp> starts;
    std::int64_t frame = 0;
    for (std::size_t step = 0; frame <= kLastFrame; ++step) {
        starts.push_back(Timestamp::fromMilliseconds(
            Timestamp::fromFrame(subedit::core::Frame::fromNumber(frame), exact).milliseconds() +
            offset));
        frame += kGapsInFrames[step % kGapsInFrames.size()];
    }
    return starts;
}

struct Expectation {
    std::string_view fixture;
    StandardFrameRate rate;
};

/// Every perfect grid, and the candidate it must yield.
constexpr std::array<Expectation, 8> kPerfectGrids = {{
    {.fixture = "grille-23-976.srt", .rate = StandardFrameRate::Fps23976},
    {.fixture = "grille-24.srt", .rate = StandardFrameRate::Fps24},
    {.fixture = "grille-25.srt", .rate = StandardFrameRate::Fps25},
    {.fixture = "grille-29-97.srt", .rate = StandardFrameRate::Fps29970},
    {.fixture = "grille-30.srt", .rate = StandardFrameRate::Fps30},
    {.fixture = "grille-50.srt", .rate = StandardFrameRate::Fps50},
    {.fixture = "grille-59-94.srt", .rate = StandardFrameRate::Fps59940},
    {.fixture = "grille-60.srt", .rate = StandardFrameRate::Fps60},
}};

} // namespace

TEST_CASE("every perfect grid yields the rate it was written on", "[analysis][deduction]") {
    for (const Expectation& one : kPerfectGrids) {
        const FrameRateDeduction deduction = deductionOf(one.fixture);

        INFO("fixture : " << one.fixture);
        CHECK(deduction.verdict == GridVerdict::Clean);
        CHECK(deduction.retained.rate == FrameRate{one.rate});
        CHECK(deduction.retained.concentration > 99.0);
        CHECK(deduction.strays.empty());
    }
}

TEST_CASE("the ranking is ordered, and holds all eight candidates", "[analysis][deduction]") {
    const FrameRateDeduction deduction = deductionOf("grille-24.srt");

    CHECK(deduction.ranked.size() == 8);
    CHECK(std::ranges::is_sorted(deduction.ranked, [](const auto& left, const auto& right) {
        return left.concentration > right.concentration;
    }));
}

TEST_CASE("a grid that is none of the eight stays silent", "[analysis][deduction]") {
    // 26.3 frames per second: perfectly regular, and no candidate's business.
    // The answer has to be "I do not know" rather than a bad rate — it is what
    // makes the deduction usable at all.
    const FrameRateDeduction deduction = deductionOf("grille-absurde.srt");

    CHECK(deduction.verdict == GridVerdict::Silent);
    CHECK(deduction.retained.concentration < 20.0);
}

TEST_CASE("a shifted grid is still a grid", "[analysis][deduction]") {
    // 24 frames per second, translated by 2999 ms. An earlier method looked for
    // a grid of zero phase and scored a file like this one at zero.
    const FrameRateDeduction deduction = deductionOf("grille-24-decalee.srt");

    CHECK(deduction.verdict == GridVerdict::Clean);
    CHECK(deduction.retained.rate == FrameRate{StandardFrameRate::Fps24});
    CHECK(deduction.retained.concentration > 99.0);
    CHECK(deduction.retained.phase.milliseconds() != 0);
}

TEST_CASE("the lower of a harmonic pair is the one retained", "[analysis][deduction]") {
    // A grid at 25 is included in a grid at 50, so a file written on 25 scores
    // 100 on both. The implication holds one way only: grille-50 gives 15.8 on
    // 25. Taking the maximum is therefore a coin flip.
    const FrameRateDeduction deduction = deductionOf("grille-25.srt");

    CHECK(deduction.retained.rate == FrameRate{StandardFrameRate::Fps25});
    CHECK(concentrationOn(deduction, StandardFrameRate::Fps50) > 99.0);
    REQUIRE(deduction.harmonic.has_value());
    CHECK(deduction.harmonic == FrameRate{StandardFrameRate::Fps50});
}

TEST_CASE("a file truly on the higher rate keeps it", "[analysis][deduction]") {
    // The rule must not swallow a genuine 50: there, the lower candidate
    // collapses to the noise, and the pair is not ambiguous at all.
    const FrameRateDeduction deduction = deductionOf("grille-50.srt");

    CHECK(deduction.retained.rate == FrameRate{StandardFrameRate::Fps50});
    CHECK(concentrationOn(deduction, StandardFrameRate::Fps25) < 20.0);
    CHECK_FALSE(deduction.harmonic.has_value());
}

TEST_CASE("the harmonic rule survives a partial file", "[analysis][deduction]") {
    // Written on 29.97, and the maximum says 59.94 — 66.4 against 65.1. This is
    // the case that proves the rule is not a precaution: the argmax is wrong.
    const FrameRateDeduction deduction = deductionOf("melange-groupe.srt");

    CHECK(deduction.retained.rate == FrameRate{StandardFrameRate::Fps29970});
    REQUIRE(deduction.harmonic.has_value());
    CHECK(deduction.harmonic == FrameRate{StandardFrameRate::Fps59940});
}

TEST_CASE("too short a span cannot separate 23.976 from 24", "[analysis][deduction]") {
    // The two grids drift by one millisecond per second of film, and a frame
    // lasts 41.7: the phase on the wrong candidate makes a full turn every
    // forty-two seconds. Ten seconds is a quarter of one.
    const FrameRateDeduction deduction = deductionOf("grille-24-courte.srt");

    CHECK(deduction.retained.rate == FrameRate{StandardFrameRate::Fps24});
    CHECK(concentrationOn(deduction, StandardFrameRate::Fps23976) > 90.0);
    CHECK(std::ranges::find(deduction.notSeparated, FrameRate{StandardFrameRate::Fps23976}) !=
          deduction.notSeparated.end());
}

TEST_CASE("a long span separates them without appeal", "[analysis][deduction]") {
    const FrameRateDeduction deduction = deductionOf("grille-24.srt");

    CHECK(concentrationOn(deduction, StandardFrameRate::Fps23976) < 10.0);
    CHECK(deduction.notSeparated.empty());
}

TEST_CASE("a retimed section shows as a few long runs", "[analysis][deduction]") {
    const FrameRateDeduction deduction = deductionOf("melange-groupe.srt");

    CHECK(deduction.verdict == GridVerdict::Partial);
    CHECK(deduction.strays.size() > 40);
    CHECK(runsOfStrays(deduction) < 10);
}

TEST_CASE("positions corrected by hand show as many runs of one", "[analysis][deduction]") {
    const FrameRateDeduction deduction = deductionOf("melange-disperse.srt");

    CHECK(deduction.verdict == GridVerdict::Partial);
    CHECK(deduction.strays.size() > 20);
    // One run per stray: nothing is grouped, which is exactly what a file
    // edited subtitle by subtitle looks like.
    CHECK(runsOfStrays(deduction) == deduction.strays.size());
}

TEST_CASE("the deduction reports the span it had to work with", "[analysis][deduction]") {
    const FrameRateDeduction full = deductionOf("grille-24.srt");
    const FrameRateDeduction brief = deductionOf("grille-24-courte.srt");

    CHECK(full.span.milliseconds() > 500000);
    CHECK(brief.span.milliseconds() < 15000);
    CHECK(full.starts > brief.starts);
}

TEST_CASE("too few positions say nothing rather than something", "[analysis][deduction]") {
    // Two starts always look concentrated, and three nearly always do: the
    // noise floor is one over the square root of the count. A verdict drawn
    // from four points would be a coin toss wearing a number.
    const std::vector<Timestamp> two = {Timestamp::fromMilliseconds(1000),
                                        Timestamp::fromMilliseconds(2000)};

    const FrameRateDeduction deduction = deduceFrameRate(two);

    CHECK(deduction.verdict == GridVerdict::Silent);
    CHECK(deduction.starts == 2);
}

TEST_CASE("no position at all is not a crash", "[analysis][deduction]") {
    // The span is spelled out because an empty brace would name both overloads
    // at once — an empty span and a default-built project are each one
    // conversion away — and neither is more nearly what is meant here.
    const FrameRateDeduction deduction = deduceFrameRate(std::span<const Timestamp>{});

    CHECK(deduction.verdict == GridVerdict::Silent);
    CHECK(deduction.starts == 0);
    CHECK(deduction.span == subedit::core::Duration::zero());
}

TEST_CASE("a grid that starts before the origin is still a grid", "[analysis][deduction]") {
    // `Timestamp` holds positions before the origin on purpose — shifting a
    // file backwards legitimately produces them, and refusing to represent one
    // would turn an editing operation into a special case. The deduction has to
    // be as indifferent to the sign as the type is: a phase is a phase whether
    // the frame is counted forwards or backwards from zero.
    //
    // The offset is 2003 milliseconds and not 2000: a whole number of frames
    // would leave every position on the grid line it started on, and the
    // negative side of the arithmetic would never be reached.
    const std::vector<Timestamp> before = translatedGrid(StandardFrameRate::Fps24, -2003);
    const std::vector<Timestamp> after = translatedGrid(StandardFrameRate::Fps24, 2003);

    REQUIRE(before.front().milliseconds() < 0);

    const FrameRateDeduction early = deduceFrameRate(before);
    const FrameRateDeduction late = deduceFrameRate(after);

    CHECK(early.verdict == GridVerdict::Clean);
    CHECK(early.retained.rate == FrameRate{StandardFrameRate::Fps24});
    CHECK(early.strays.empty());

    // The same grid, the same reading: where the origin happens to sit changes
    // nothing about whether the positions fall on frames.
    CHECK_THAT(early.retained.concentration,
               Catch::Matchers::WithinAbs(late.retained.concentration, 1e-9));
}

TEST_CASE("a project is read exactly as its starts alone", "[analysis][deduction]") {
    // The overload exists so that no caller writes the gathering loop, and the
    // only thing it owes anyone is that it changes nothing: the ends are as
    // absent from its reading as they are from the other one.
    const Project project =
        subedit::test::gridProject("grille-24-decalee.srt", FrameRate{StandardFrameRate::Fps24});

    const FrameRateDeduction fromProject = deduceFrameRate(project);
    const FrameRateDeduction fromStarts =
        deduceFrameRate(subedit::test::gridStarts("grille-24-decalee.srt"));

    CHECK(fromProject.verdict == fromStarts.verdict);
    CHECK(fromProject.retained.rate == fromStarts.retained.rate);
    CHECK(fromProject.retained.phase == fromStarts.retained.phase);
    CHECK(fromProject.starts == fromStarts.starts);
    CHECK(fromProject.span == fromStarts.span);
    CHECK(fromProject.strays == fromStarts.strays);
    CHECK_THAT(fromProject.retained.concentration,
               Catch::Matchers::WithinAbs(fromStarts.retained.concentration, 1e-9));
}

TEST_CASE("an empty project is not a crash either", "[analysis][deduction]") {
    const FrameRateDeduction deduction = deduceFrameRate(Project{});

    CHECK(deduction.verdict == GridVerdict::Silent);
    CHECK(deduction.starts == 0);
    CHECK(deduction.span == subedit::core::Duration::zero());
}

// The three thresholds of the whole phase, each held on both sides of its own
// frontier — issue #225.
//
// They live in the implementation, one copy each, with their reason written
// next to them. What was missing was never the number; it was a test that
// notices when it moves. **Moving any of the three by one point makes one of
// the three cases below fail**, which is the only property that makes a
// threshold checkable at all.

TEST_CASE("a clean grid starts at ninety, and not at eighty-nine", "[analysis][deduction]") {
    // 91 aligned against 5 cancelling pairs is 91 of 101, which is 90.099 — a
    // tenth of a point above. Two fewer aligned is 89 of 99, which is 89.899, a
    // tenth below. Nothing else differs between the two.
    const FrameRateDeduction above = deduceFrameRate(concentratedOn25(91, 5));
    const FrameRateDeduction below = deduceFrameRate(concentratedOn25(89, 5));

    REQUIRE(above.retained.rate == FrameRate{StandardFrameRate::Fps25});
    REQUIRE(below.retained.rate == FrameRate{StandardFrameRate::Fps25});

    CHECK_THAT(above.retained.concentration, Catch::Matchers::WithinAbs(90.099, 0.001));
    CHECK(above.verdict == GridVerdict::Clean);

    CHECK_THAT(below.retained.concentration, Catch::Matchers::WithinAbs(89.899, 0.001));
    CHECK(below.verdict == GridVerdict::Partial);
}

TEST_CASE("a partial grid starts at fifty, and not at forty-nine", "[analysis][deduction]") {
    // 51 of 101 is 50.495; 49 of 99 is 49.495. Below the second the deduction
    // names no rate at all, which is the answer a closed set of candidates
    // exists to be able to give.
    const FrameRateDeduction above = deduceFrameRate(concentratedOn25(51, 25));
    const FrameRateDeduction below = deduceFrameRate(concentratedOn25(49, 25));

    REQUIRE(above.retained.rate == FrameRate{StandardFrameRate::Fps25});

    CHECK_THAT(above.retained.concentration, Catch::Matchers::WithinAbs(50.495, 0.001));
    CHECK(above.verdict == GridVerdict::Partial);

    CHECK_THAT(below.retained.concentration, Catch::Matchers::WithinAbs(49.495, 0.001));
    CHECK(below.verdict == GridVerdict::Silent);
}

TEST_CASE("ten starts earn a verdict, and nine do not", "[analysis][deduction]") {
    const FrameRateDeduction nine = deduceFrameRate(gridOf(9));
    const FrameRateDeduction ten = deduceFrameRate(gridOf(10));

    // Both sit perfectly on the grid, and the ranking says so in both cases.
    // What the tenth start buys is not a better fit but the right to conclude
    // from one — which is why the silent verdict below has to be read next to
    // `enoughStarts` rather than alone.
    CHECK_THAT(nine.ranked.front().concentration, Catch::Matchers::WithinAbs(100.0, 1e-9));
    CHECK_THAT(ten.ranked.front().concentration, Catch::Matchers::WithinAbs(100.0, 1e-9));

    CHECK(!nine.enoughStarts);
    CHECK(nine.verdict == GridVerdict::Silent);

    CHECK(ten.enoughStarts);
    CHECK(ten.verdict == GridVerdict::Clean);
    CHECK(ten.retained.rate == FrameRate{StandardFrameRate::Fps25});
}
