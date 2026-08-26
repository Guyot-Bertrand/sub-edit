// Aligning positions on a frame grid, and everything that separates it from a
// conversion.
//
// The two accept the same arguments and do opposite things: a conversion scales
// the file and drags it by seconds over a feature film, an alignment moves each
// position by half a frame at most. Mistaking one for the other is silent, so
// the last case here holds them side by side rather than trusting the prose.

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <grid_fixtures.hpp>
#include <memory>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::ConvertFrameRateCommand;
using subedit::core::FrameRate;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::SnapCommand;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

const FrameRate kPal{StandardFrameRate::Fps25};
const FrameRate kCinema{StandardFrameRate::Fps24};

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

/// Whether every position of the project lies exactly on `rate`'s grid.
[[nodiscard]] bool allOnGrid(const Project& project, FrameRate rate) {
    return std::ranges::all_of(project.subtitles(), [rate](const Subtitle& subtitle) {
        return Timestamp::fromFrame(subtitle.start.toFrame(rate), rate) == subtitle.start &&
               Timestamp::fromFrame(subtitle.end.toFrame(rate), rate) == subtitle.end;
    });
}

[[nodiscard]] std::vector<std::int64_t> boundsOf(const Project& project) {
    std::vector<std::int64_t> bounds;
    bounds.reserve(project.count() * 2);
    for (const Subtitle& subtitle : project.subtitles()) {
        bounds.push_back(subtitle.start.milliseconds());
        bounds.push_back(subtitle.end.milliseconds());
    }
    return bounds;
}

/// The largest distance any position moved between the two states.
[[nodiscard]] std::int64_t furthestMove(const std::vector<std::int64_t>& before,
                                        const std::vector<std::int64_t>& after) {
    std::int64_t furthest = 0;
    for (std::size_t rank = 0; rank < before.size(); ++rank)
        furthest = std::max(furthest, std::abs(after[rank] - before[rank]));
    return furthest;
}

[[nodiscard]] std::unique_ptr<SnapCommand> snapTo(const Project& project, FrameRate rate) {
    return std::make_unique<SnapCommand>(project, Selection::all(project), rate);
}

/// A project holding `subtitles`, declared at `rate`.
[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles, FrameRate rate) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setFrameRate(rate);
    return project;
}

/// The two positions ADR 0013 was measured on, an hour apart.
[[nodiscard]] std::vector<Subtitle> twoFarApart() {
    return {at(1010, 1020), at(3600017, 3600020)};
}

} // namespace

TEST_CASE("aligning puts every position on the grid", "[edit][snap]") {
    Session session{projectOf(twoFarApart(), kCinema)};

    session.apply(snapTo(session.project(), kPal));

    CHECK(allOnGrid(session.project(), kPal));
}

TEST_CASE("aligning moves nothing by more than half a frame", "[edit][snap]") {
    Session session{subedit::test::gridProject("grille-24.srt", kCinema)};

    const std::vector<std::int64_t> before = boundsOf(session.project());
    session.apply(snapTo(session.project(), kPal));

    // A frame at 25 frames per second lasts forty milliseconds, so no position
    // may travel more than twenty.
    CHECK(furthestMove(before, boundsOf(session.project())) <= 20);
}

TEST_CASE("aligning twice on the same rate changes nothing", "[edit][snap]") {
    Session session{projectOf(twoFarApart(), kCinema)};

    session.apply(snapTo(session.project(), kPal));
    const std::vector<std::int64_t> once = boundsOf(session.project());

    session.apply(snapTo(session.project(), kPal));

    CHECK(boundsOf(session.project()) == once);
}

TEST_CASE("aligning on the rate a file already sits on changes nothing", "[edit][snap]") {
    Session session{subedit::test::gridProject("grille-24.srt", kCinema)};

    const std::vector<std::int64_t> before = boundsOf(session.project());
    session.apply(snapTo(session.project(), kCinema));

    CHECK(boundsOf(session.project()) == before);
}

TEST_CASE("aligning keeps the starts in order", "[edit][snap]") {
    Session session{subedit::test::gridProject("grille-24.srt", kCinema)};

    session.apply(snapTo(session.project(), kPal));

    // Rounding to the nearest frame is monotone, so two starts a frame or more
    // apart stay in order. Closer than that they may coincide, and coinciding
    // is not being out of order.
    CHECK(std::ranges::is_sorted(
        session.project().subtitles(),
        [](const Subtitle& left, const Subtitle& right) { return left.start < right.start; }));
}

TEST_CASE("aligning re-declares the rate of the project", "[edit][snap]") {
    Session session{projectOf({at(1010, 1020)}, kCinema)};

    session.apply(snapTo(session.project(), kPal));

    CHECK(session.project().frameRate() == kPal);
}

TEST_CASE("undoing an alignment puts the positions back", "[edit][snap]") {
    Session session{projectOf(twoFarApart(), kCinema)};
    const std::vector<std::int64_t> before = boundsOf(session.project());

    session.apply(snapTo(session.project(), kPal));
    session.undo();

    CHECK(boundsOf(session.project()) == before);
    CHECK(session.project().frameRate() == kCinema);
}

TEST_CASE("an alignment says it moved positions", "[edit][snap]") {
    Project project;
    project.setSubtitles({at(1010, 1020)});
    project.setFrameRate(kCinema);

    const SnapCommand command{project, Selection::all(project), kPal};

    CHECK(command.kind() == CommandKind::Snap);
    REQUIRE(command.describe().size() == 1);
    CHECK(command.describe().front().kind == ChangeKind::Positions);
}

TEST_CASE("aligning is not converting, and the gap is seconds", "[edit][snap]") {
    // The whole point of D11, held to a measurement rather than to prose. The
    // same file, the same pair of rates, and two operations that a user could
    // confuse: one moves it by half a frame, the other by seconds.
    Session aligned{subedit::test::gridProject("grille-24.srt", kCinema)};
    Session converted{subedit::test::gridProject("grille-24.srt", kCinema)};

    const std::vector<std::int64_t> before = boundsOf(aligned.project());

    aligned.apply(snapTo(aligned.project(), kPal));
    converted.apply(std::make_unique<ConvertFrameRateCommand>(
        converted.project(), Selection::all(converted.project()), kCinema, kPal));

    CHECK(furthestMove(before, boundsOf(aligned.project())) <= 20);
    CHECK(furthestMove(before, boundsOf(converted.project())) > 20000);
}
