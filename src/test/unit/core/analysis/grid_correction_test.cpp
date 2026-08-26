// Bringing a file back onto the grid it was written on, and what separates that
// from aligning it.
//
// The two coincide on a clean file and part company on a partial one, which is
// their whole reason for existing side by side: one leaves the strays where
// they are, the other absorbs them.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <grid_fixtures.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace {

using subedit::core::deduceFrameRate;
using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::ShiftCommand;
using subedit::core::shiftOntoGrid;
using subedit::core::SnapCommand;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

const FrameRate kCinema{StandardFrameRate::Fps24};
const FrameRate kPal{StandardFrameRate::Fps25};

/// Whether the fixture has a grid to be brought back to at all.
[[nodiscard]] bool hasCorrection(std::string_view fixture) {
    return shiftOntoGrid(deduceFrameRate(subedit::test::gridStarts(fixture))).has_value();
}

/// The correction for a fixture, ending the case when there is none.
///
/// `value_or` and not a dereference: the `REQUIRE` above it has already ended
/// the case if there is nothing to unwrap, and this form says so to a static
/// checker as well as to a reader.
[[nodiscard]] Duration amountFor(std::string_view fixture) {
    const std::optional<Duration> amount =
        shiftOntoGrid(deduceFrameRate(subedit::test::gridStarts(fixture)));
    REQUIRE(amount.has_value());
    return amount.value_or(Duration::zero());
}

[[nodiscard]] std::vector<Timestamp> startsOf(const Project& project) {
    std::vector<Timestamp> starts;
    starts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        starts.push_back(subtitle.start);
    return starts;
}

/// Whether every start of the project falls exactly on `rate`'s grid.
[[nodiscard]] bool everyStartOnGrid(const Project& project, FrameRate rate) {
    return std::ranges::all_of(project.subtitles(), [rate](const Subtitle& subtitle) {
        return Timestamp::fromFrame(subtitle.start.toFrame(rate), rate) == subtitle.start;
    });
}

/// The gaps between consecutive starts, which a rigid shift must not touch.
[[nodiscard]] std::vector<std::int64_t> gapsOf(const Project& project) {
    const std::vector<Timestamp> starts = startsOf(project);
    std::vector<std::int64_t> gaps;
    for (std::size_t rank = 1; rank < starts.size(); ++rank)
        gaps.push_back((starts[rank] - starts[rank - 1]).milliseconds());
    return gaps;
}

[[nodiscard]] std::size_t straysAfter(const Project& project) {
    return deduceFrameRate(startsOf(project)).strays.size();
}

} // namespace

TEST_CASE("a file with no grid has nowhere to be brought back to", "[analysis][grid]") {
    // 26.3 frames per second: regular, and none of the eight candidates. A
    // phase measured on noise would move the file by an arbitrary amount.
    CHECK_FALSE(hasCorrection("grille-absurde.srt"));
}

TEST_CASE("a file already on its grid is asked to move by nothing", "[analysis][grid]") {
    CHECK(amountFor("grille-24.srt") == Duration::zero());
}

TEST_CASE("the amount is the shortest way back to the grid", "[analysis][grid]") {
    // A frame at 24 frames per second lasts 41.7 milliseconds, so no correction
    // may exceed 21: past halfway, the nearer grid line is the next one.
    CHECK(std::abs(amountFor("grille-24-decalee.srt").milliseconds()) <= 21);
}

TEST_CASE("a shifted grid comes back exactly onto it", "[analysis][grid]") {
    // Written on 24 and translated by 2999 milliseconds — the shift Gaupol
    // applies as such. Not one start sits on a frame before the correction.
    Session session{subedit::test::gridProject("grille-24-decalee.srt", kCinema)};
    REQUIRE_FALSE(everyStartOnGrid(session.project(), kCinema));

    session.apply(std::make_unique<ShiftCommand>(Selection::all(session.project()),
                                                 amountFor("grille-24-decalee.srt")));

    CHECK(everyStartOnGrid(session.project(), kCinema));
}

TEST_CASE("bringing back to the grid keeps every gap", "[analysis][grid]") {
    Session session{subedit::test::gridProject("grille-24-decalee.srt", kCinema)};
    const std::vector<std::int64_t> before = gapsOf(session.project());

    session.apply(std::make_unique<ShiftCommand>(Selection::all(session.project()),
                                                 amountFor("grille-24-decalee.srt")));

    // A rigid shift moves everything by the same amount, so the relative
    // timing is preserved **exactly**. That is what separates it from aligning.
    CHECK(gapsOf(session.project()) == before);
}

TEST_CASE("on a partial file, coming back is not aligning", "[analysis][grid]") {
    // 25 frames per second with one start in five moved by hand. Bringing the
    // file back to its grid leaves those where their editor put them; aligning
    // absorbs them. Both are defensible, and they are not the same operation.
    const std::size_t straysBefore =
        deduceFrameRate(subedit::test::gridStarts("melange-disperse.srt")).strays.size();
    REQUIRE(straysBefore > 20);

    Session brought{subedit::test::gridProject("melange-disperse.srt", kPal)};
    brought.apply(std::make_unique<ShiftCommand>(Selection::all(brought.project()),
                                                 amountFor("melange-disperse.srt")));

    Session aligned{subedit::test::gridProject("melange-disperse.srt", kPal)};
    aligned.apply(
        std::make_unique<SnapCommand>(aligned.project(), Selection::all(aligned.project()), kPal));

    CHECK(straysAfter(brought.project()) == straysBefore);
    CHECK(straysAfter(aligned.project()) == 0);
}
