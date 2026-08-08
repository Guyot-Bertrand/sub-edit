#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Duration;
using subedit::core::OrderPolicy;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::ShiftCommand;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

[[nodiscard]] Project threeSubtitles() {
    Project project;
    project.setSubtitles({at(1000, 2000), at(3000, 4000), at(5000, 6000)});
    return project;
}

/// The start and end of each subtitle, in milliseconds.
[[nodiscard]] std::vector<std::int64_t> boundsOf(const Project& project) {
    std::vector<std::int64_t> bounds;
    bounds.reserve(project.count() * 2);
    for (const Subtitle& subtitle : project.subtitles()) {
        bounds.push_back(subtitle.start.milliseconds());
        bounds.push_back(subtitle.end.milliseconds());
    }
    return bounds;
}

[[nodiscard]] Selection lastOnly() {
    const std::array<SubtitleIndex, 1> indices = {SubtitleIndex::fromValue(2)};
    return Selection::of(indices);
}

[[nodiscard]] Duration milliseconds(std::int64_t count) {
    return Duration::fromMilliseconds(count);
}

} // namespace

TEST_CASE("a shift moves both ends of every selected subtitle", "[edit][shift]") {
    Project project = threeSubtitles();
    ShiftCommand command{Selection::all(project), milliseconds(500)};

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{1500, 2500, 3500, 4500, 5500, 6500});
}

TEST_CASE("a shift leaves the subtitles it was not given alone", "[edit][shift]") {
    Project project = threeSubtitles();
    ShiftCommand command{lastOnly(), milliseconds(1000)};

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{1000, 2000, 3000, 4000, 6000, 7000});
}

TEST_CASE("a shift preserves durations", "[edit][shift]") {
    // Moving both ends by the same amount is what makes a shift a shift: no
    // subtitle stays on screen any longer or shorter than it did.
    Project project = threeSubtitles();
    ShiftCommand command{Selection::all(project), milliseconds(-250)};

    command.apply(project);

    for (const Subtitle& subtitle : project.subtitles())
        CHECK(subtitle.duration().milliseconds() == 1000);
}

TEST_CASE("a negative shift may take a subtitle before the origin", "[edit][shift]") {
    // Positions before the start of the video are valid — a shift backwards
    // may legitimately land there, and refusing it would turn an editing
    // operation into a special case.
    Project project = threeSubtitles();
    ShiftCommand command{Selection::all(project), milliseconds(-2000)};

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{-1000, 0, 1000, 2000, 3000, 4000});
}

TEST_CASE("undoing a shift applies it the other way", "[edit][shift]") {
    // What the command retains is the duration, not the positions: its inverse
    // is exact, and that is the economy ADR 0010 asks for.
    Project project = threeSubtitles();
    const std::vector<std::int64_t> before = boundsOf(project);
    ShiftCommand command{Selection::all(project), milliseconds(1234)};

    command.apply(project);
    command.revert(project);

    CHECK(boundsOf(project) == before);
}

TEST_CASE("a shift of nothing is not a special case", "[edit][shift]") {
    Project project = threeSubtitles();
    const std::vector<std::int64_t> before = boundsOf(project);
    ShiftCommand command{Selection::of({}), milliseconds(1000)};

    command.apply(project);

    CHECK(boundsOf(project) == before);
}

TEST_CASE("a shift reports the positions it moved", "[edit][shift]") {
    const ShiftCommand command{lastOnly(), milliseconds(1000)};

    const std::vector<subedit::core::Change> changes = command.describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Positions);
    CHECK(changes[0].indices == std::vector<SubtitleIndex>{SubtitleIndex::fromValue(2)});
}

TEST_CASE("a shift says what it is", "[edit][shift]") {
    const ShiftCommand command{lastOnly(), milliseconds(1000)};

    CHECK(command.kind() == CommandKind::Shift);
}

TEST_CASE("a partial shift that breaks the order is reported under a lenient session",
          "[edit][shift]") {
    // The core reports and does not repair: the file keeps the order it came
    // with, and `outOfOrder` names the line that no longer follows.
    Session session{threeSubtitles()};

    session.apply(std::make_unique<ShiftCommand>(lastOnly(), milliseconds(-4500)));

    CHECK(session.project().outOfOrder() ==
          std::vector<SubtitleIndex>{SubtitleIndex::fromValue(2)});
    CHECK(boundsOf(session.project()) ==
          std::vector<std::int64_t>{1000, 2000, 3000, 4000, 500, 1500});
}

TEST_CASE("a partial shift that breaks the order is sorted under a strict session",
          "[edit][shift]") {
    Session session{threeSubtitles(), OrderPolicy::Strict};

    session.apply(std::make_unique<ShiftCommand>(lastOnly(), milliseconds(-4500)));

    CHECK(session.project().outOfOrder().empty());
    CHECK(boundsOf(session.project()) ==
          std::vector<std::int64_t>{500, 1500, 1000, 2000, 3000, 4000});
    CHECK(session.undoableCount() == 1);
}

TEST_CASE("a broken order undoes exactly under a lenient session", "[edit][shift]") {
    Session session{threeSubtitles()};
    const std::vector<std::int64_t> before = boundsOf(session.project());

    session.apply(std::make_unique<ShiftCommand>(lastOnly(), milliseconds(-4500)));
    session.undo();

    CHECK(boundsOf(session.project()) == before);
    CHECK(session.project().outOfOrder().empty());
}

TEST_CASE("a broken order undoes exactly under a strict session", "[edit][shift]") {
    // Harder than the lenient case: undoing has to take back the sort as well,
    // and put the rows back where the file had them.
    Session session{threeSubtitles(), OrderPolicy::Strict};
    const std::vector<std::int64_t> before = boundsOf(session.project());

    session.apply(std::make_unique<ShiftCommand>(lastOnly(), milliseconds(-4500)));
    session.undo();

    CHECK(boundsOf(session.project()) == before);
    CHECK_FALSE(session.canUndo());
}

TEST_CASE("a shift marks both documents", "[edit][shift]") {
    Session session{threeSubtitles()};

    session.apply(
        std::make_unique<ShiftCommand>(Selection::all(session.project()), milliseconds(1000)));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 1);
}

TEST_CASE("undoing then redoing a shift restores the exact state", "[edit][shift]") {
    Session session{threeSubtitles()};

    session.apply(
        std::make_unique<ShiftCommand>(Selection::all(session.project()), milliseconds(1000)));
    session.undo();
    CHECK(boundsOf(session.project()) ==
          std::vector<std::int64_t>{1000, 2000, 3000, 4000, 5000, 6000});

    session.redo();
    CHECK(boundsOf(session.project()) ==
          std::vector<std::int64_t>{2000, 3000, 4000, 5000, 6000, 7000});
}
