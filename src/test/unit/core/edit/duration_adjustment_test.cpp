// Adjusting durations — issue #383.
//
// Gaupol's order, and only the end moves: reading speed, minimum, maximum, gap.
// What is under test is that order, what an inactive constraint means, where
// the last subtitle stops, and the count of what no end could satisfy.

#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/video_bounds.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::adjustDurations;
using subedit::core::CommandKind;
using subedit::core::Duration;
using subedit::core::DurationAdjustment;
using subedit::core::DurationConstraints;
using subedit::core::Project;
using subedit::core::ReadingSpeed;
using subedit::core::SacrificedConstraints;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle from(std::int64_t start, std::int64_t end, std::string_view text = "") {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = std::string{text}};
}

[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setSourceFile(SourceFile{.format = SubtitleFormat::SubRip});
    return project;
}

[[nodiscard]] DurationConstraints none() {
    return DurationConstraints{.speed = std::nullopt,
                               .minimum = std::nullopt,
                               .maximum = std::nullopt,
                               .gap = std::nullopt};
}

[[nodiscard]] Duration ms(std::int64_t count) {
    return Duration::fromMilliseconds(count);
}

[[nodiscard]] Timestamp endOf(const Project& project, std::size_t index) {
    return project.subtitleAt(SubtitleIndex::fromValue(index)).end;
}

/// Applies `constraints` to every subtitle of `project`, and hands back what
/// the adjustment reported.
[[nodiscard]] DurationAdjustment adjusting(Project& project,
                                           const DurationConstraints& constraints) {
    DurationAdjustment adjustment = adjustDurations(project, Selection::all(project), constraints);
    if (adjustment.command != nullptr)
        adjustment.command->apply(project);
    return adjustment;
}

} // namespace

TEST_CASE("the defaults are Gaupol's", "[edit][durations]") {
    const DurationConstraints defaults;

    // Compared whole rather than field by field: the analysis does not follow a
    // `REQUIRE` into the accesses after it.
    CHECK(defaults.speed == ReadingSpeed::create(15.0, true, false));
    CHECK(defaults.minimum == ms(1500));
    CHECK_FALSE(defaults.maximum.has_value());
    CHECK(defaults.gap == Duration::zero());
    CHECK(defaults.isAny());
    CHECK_FALSE(none().isAny());
}

TEST_CASE("a reading speed of zero or less is refused", "[edit][durations]") {
    CHECK_FALSE(ReadingSpeed::create(0.0, true, false).has_value());
    CHECK_FALSE(ReadingSpeed::create(-1.0, true, false).has_value());
    CHECK(ReadingSpeed::create(15.0, true, false).has_value());
}

TEST_CASE("a text too long for its duration is lengthened to its reading speed",
          "[edit][durations]") {
    // Eight characters at ten a second: eight hundred milliseconds.
    Project project = projectOf({from(0, 500, "Bonjour.")});
    DurationConstraints constraints = none();
    constraints.speed = ReadingSpeed::create(10.0, true, false);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(adjustment.adjusted == 1);
    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(800));
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).start == Timestamp::origin());
}

TEST_CASE("a duration longer than the text needs is shortened only when asked",
          "[edit][durations]") {
    Project project = projectOf({from(0, 5000, "Bonjour.")});
    DurationConstraints constraints = none();
    constraints.speed = ReadingSpeed::create(10.0, true, false);

    CHECK(adjusting(project, constraints).command == nullptr);
    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(5000));

    constraints.speed = ReadingSpeed::create(10.0, true, true);
    static_cast<void>(adjusting(project, constraints));
    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(800));
}

TEST_CASE("the length of a text counts characters, without its tags", "[edit][durations]") {
    // Three characters, four bytes and seven of markup: three hundred
    // milliseconds at ten a second.
    Project project = projectOf({from(0, 100, "<i>été</i>")});
    DurationConstraints constraints = none();
    constraints.speed = ReadingSpeed::create(10.0, true, false);

    static_cast<void>(adjusting(project, constraints));

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(300));
}

TEST_CASE("a minimum and a maximum bound the duration", "[edit][durations]") {
    Project project = projectOf({from(0, 500), from(10000, 20000)});
    DurationConstraints constraints = none();
    constraints.minimum = ms(1500);
    constraints.maximum = ms(6000);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(adjustment.adjusted == 2);
    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(1500));
    CHECK(endOf(project, 1) == Timestamp::fromMilliseconds(16000));
    CHECK_FALSE(adjustment.sacrificed.isAny());
}

TEST_CASE("a minimum of zero is a minimum of zero", "[edit][durations]") {
    // Gaupol's `minimum and …` would switch it off. An end before its start is
    // brought back to it.
    Project project = projectOf({from(1000, 400)});
    DurationConstraints constraints = none();
    constraints.minimum = Duration::zero();

    static_cast<void>(adjusting(project, constraints));

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(1000));
}

TEST_CASE("the maximum is applied after the reading speed, and wins over it", "[edit][durations]") {
    Project project = projectOf({from(0, 500, "Une phrase bien trop longue pour six secondes.")});
    DurationConstraints constraints = none();
    constraints.speed = ReadingSpeed::create(1.0, true, false);
    constraints.maximum = ms(6000);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(6000));
    CHECK(adjustment.sacrificed == SacrificedConstraints{.speed = 1});
}

TEST_CASE("the gap is applied last, and gives way to nothing", "[edit][durations]") {
    // The next subtitle starts at one second: a minimum of 1.5 s and a gap of
    // 200 ms cannot both hold, and the gap wins.
    Project project = projectOf({from(0, 500), from(1000, 2000)});
    DurationConstraints constraints = none();
    constraints.minimum = ms(1500);
    constraints.gap = ms(200);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(800));
    CHECK(adjustment.sacrificed == SacrificedConstraints{.minimum = 1});
}

TEST_CASE("the gap never moves an end before its start", "[edit][durations]") {
    // The next subtitle starts before this one: no end can leave the gap, and
    // the best there is is a duration of zero — which is said.
    Project project = projectOf({from(1000, 3000), from(900, 2000)});
    DurationConstraints constraints = none();
    constraints.gap = Duration::zero();

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(1000));
    CHECK(adjustment.sacrificed == SacrificedConstraints{.gap = 1});
}

TEST_CASE("the last subtitle has no next one, and no bound for it", "[edit][durations]") {
    Project project = projectOf({from(0, 1000), from(5000, 5500)});
    DurationConstraints constraints = none();
    constraints.minimum = ms(1500);
    constraints.gap = ms(200);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(endOf(project, 1) == Timestamp::fromMilliseconds(6500));
    CHECK_FALSE(adjustment.sacrificed.isAny());
}

TEST_CASE("the next subtitle bounds the gap whether it is selected or not", "[edit][durations]") {
    Project project = projectOf({from(0, 900), from(1000, 2000)});
    DurationConstraints constraints = none();
    constraints.gap = ms(300);

    const DurationAdjustment adjustment =
        adjustDurations(project,
                        Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0)),
                        constraints);
    REQUIRE(adjustment.command != nullptr);
    adjustment.command->apply(project);

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(700));
    CHECK(endOf(project, 1) == Timestamp::fromMilliseconds(2000));
}

TEST_CASE("what no end can satisfy is counted even where nothing moved", "[edit][durations]") {
    // Already at the gap, already short: the adjustment has nothing to move and
    // still something to say. The second lasts two seconds, so that the
    // minimum has nothing to say about it either.
    Project project = projectOf({from(0, 800), from(1000, 3000)});
    DurationConstraints constraints = none();
    constraints.minimum = ms(1500);
    constraints.gap = ms(200);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(adjustment.command == nullptr);
    CHECK(adjustment.adjusted == 0);
    CHECK(adjustment.sacrificed == SacrificedConstraints{.minimum = 1});
}

TEST_CASE("an adjustment is one entry in the history, and moves no start", "[edit][durations]") {
    Session session{projectOf({from(0, 500), from(3000, 3200), from(9000, 9100)})};

    DurationAdjustment adjustment = adjustDurations(
        session.project(), Selection::all(session.project()), DurationConstraints{});
    REQUIRE(adjustment.command != nullptr);
    CHECK(adjustment.command->kind() == CommandKind::AdjustDurations);
    CHECK(adjustment.adjusted == 3);
    static_cast<void>(session.apply(std::move(adjustment.command)));

    CHECK(endOf(session.project(), 0) == Timestamp::fromMilliseconds(1500));
    CHECK_FALSE(subedit::core::mayBreakOrder(CommandKind::AdjustDurations));

    static_cast<void>(session.undo());
    CHECK(endOf(session.project(), 0) == Timestamp::fromMilliseconds(500));
    CHECK(endOf(session.project(), 2) == Timestamp::fromMilliseconds(9100));
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("an adjustment can carry an end past the film, and is watched for it",
          "[edit][durations]") {
    // Lengthening moves ends later: the window says when one lands after the
    // end of the video, as it does after a shift.
    CHECK(subedit::core::movesPositions(CommandKind::AdjustDurations));
}

TEST_CASE("the maximum always holds, and a minimum above it gives way", "[edit][durations]") {
    // Applied after the speed and the minimum, followed only by the gap, which
    // can only bring an end earlier: nothing is left to break the maximum. A
    // minimum set above it is what is given up, and counted.
    Project project = projectOf({from(0, 500)});
    DurationConstraints constraints = none();
    constraints.minimum = ms(4000);
    constraints.maximum = ms(3000);

    const DurationAdjustment adjustment = adjusting(project, constraints);

    CHECK(endOf(project, 0) == Timestamp::fromMilliseconds(3000));
    CHECK(adjustment.sacrificed == SacrificedConstraints{.minimum = 1});
}
