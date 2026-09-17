// Setting a duration — issue #381.
//
// The command moves the end to `start + duration`, and only the end: that is
// decision D3 of the phase-10 spec, and what these cases hold.

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_duration_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::Duration;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::setDuration;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle from(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

/// Two subtitles: one to three seconds, then five to six.
[[nodiscard]] Project twoSubtitles() {
    Project project;
    project.setSubtitles({from(1000, 3000), from(5000, 6000)});
    return project;
}

[[nodiscard]] SubtitleIndex first() {
    return SubtitleIndex::fromValue(0);
}

} // namespace

TEST_CASE("a duration moves the end and leaves the start alone", "[edit][setduration]") {
    Project project = twoSubtitles();

    const std::unique_ptr<Command> command =
        setDuration(project, first(), Duration::fromMilliseconds(1500));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitleAt(first()).start == Timestamp::fromMilliseconds(1000));
    CHECK(project.subtitleAt(first()).end == Timestamp::fromMilliseconds(2500));
}

TEST_CASE("undoing a duration puts back the end it replaced", "[edit][setduration]") {
    Project project = twoSubtitles();

    const std::unique_ptr<Command> command =
        setDuration(project, first(), Duration::fromMilliseconds(1500));
    REQUIRE(command != nullptr);
    command->apply(project);
    command->revert(project);

    CHECK(project.subtitleAt(first()) == from(1000, 3000));
}

TEST_CASE("a duration may carry the end over the next subtitle", "[edit][setduration]") {
    // Reported by `scanAnomalies`, never refused: decision D4 of phase 5.
    Project project = twoSubtitles();

    const std::unique_ptr<Command> command =
        setDuration(project, first(), Duration::fromMilliseconds(10000));
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.subtitleAt(first()).end == Timestamp::fromMilliseconds(11000));
}

TEST_CASE("a duration reports a change of positions", "[edit][setduration]") {
    const Project project = twoSubtitles();
    const std::unique_ptr<Command> command =
        setDuration(project, first(), Duration::fromMilliseconds(1500));
    REQUIRE(command != nullptr);

    const std::vector<subedit::core::Change> changes = command->describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Positions);
    CHECK(changes[0].subtitles.count() == 1);
    CHECK(command->kind() == CommandKind::SetDuration);
}

TEST_CASE("a duration cannot break the order", "[edit][setduration]") {
    // Only a start can, and a duration moves none.
    CHECK_FALSE(subedit::core::mayBreakOrder(CommandKind::SetDuration));
}

TEST_CASE("a duration is one entry in the history, and redoes exactly", "[edit][setduration]") {
    Session session{twoSubtitles()};

    std::unique_ptr<Command> command = setDuration(session.project(), first(), Duration::zero());
    REQUIRE(command != nullptr);
    static_cast<void>(session.apply(std::move(command)));
    CHECK(session.nextUndoKind() == CommandKind::SetDuration);
    CHECK(session.project().subtitleAt(first()).end == Timestamp::fromMilliseconds(1000));

    static_cast<void>(session.undo());
    CHECK(session.project().subtitleAt(first()).end == Timestamp::fromMilliseconds(3000));
    CHECK_FALSE(session.canUndo());

    static_cast<void>(session.redo());
    CHECK(session.project().subtitleAt(first()).end == Timestamp::fromMilliseconds(1000));
}
