#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/order_policy.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {

using subedit::core::Boundary;
using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::OrderPolicy;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::SetPositionCommand;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

constexpr SubtitleIndex kFirst = SubtitleIndex::fromValue(0);
constexpr SubtitleIndex kSecond = SubtitleIndex::fromValue(1);

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

[[nodiscard]] Project projectOf() {
    Project project;
    project.setSubtitles({at(1000, 3000), at(5000, 7000)});
    return project;
}

[[nodiscard]] std::unique_ptr<SetPositionCommand>
setStart(const Project& project, SubtitleIndex index, std::int64_t milliseconds) {
    return std::make_unique<SetPositionCommand>(
        project, index, Boundary::Start, Timestamp::fromMilliseconds(milliseconds));
}

} // namespace

TEST_CASE("setting a start moves it and leaves the end alone", "[edit][setposition]") {
    Project project = projectOf();
    SetPositionCommand command{project, kFirst, Boundary::Start, Timestamp::fromMilliseconds(2000)};

    command.apply(project);

    CHECK(project.subtitleAt(kFirst).start.milliseconds() == 2000);
    CHECK(project.subtitleAt(kFirst).end.milliseconds() == 3000);
}

TEST_CASE("setting an end moves it and leaves the start alone", "[edit][setposition]") {
    Project project = projectOf();
    SetPositionCommand command{project, kFirst, Boundary::End, Timestamp::fromMilliseconds(4000)};

    command.apply(project);

    CHECK(project.subtitleAt(kFirst).end.milliseconds() == 4000);
    CHECK(project.subtitleAt(kFirst).start.milliseconds() == 1000);
}

TEST_CASE("undoing a position change puts the old one back", "[edit][setposition]") {
    Project project = projectOf();
    SetPositionCommand command{project, kFirst, Boundary::Start, Timestamp::fromMilliseconds(2000)};

    command.apply(project);
    command.revert(project);

    CHECK(project.subtitleAt(kFirst).start.milliseconds() == 1000);
}

TEST_CASE("an end may be set before its start", "[edit][setposition]") {
    // `end >= start` is not an invariant — ADR 0008. A real file holds the
    // anomaly, the user has to see it to fix it, and refusing the edit here
    // would be refusing to let them reach that state on purpose.
    Project project = projectOf();
    SetPositionCommand command{project, kFirst, Boundary::End, Timestamp::fromMilliseconds(500)};

    command.apply(project);

    CHECK(project.subtitleAt(kFirst).duration().milliseconds() == -500);
}

TEST_CASE("a position change reports positions, whichever boundary moved", "[edit][setposition]") {
    // A subtitle carries one pair of positions for both texts, so moving
    // either boundary concerns both documents. `ChangeKind::Positions` says
    // exactly that.
    const Project project = projectOf();
    const SetPositionCommand command{
        project, kFirst, Boundary::End, Timestamp::fromMilliseconds(4000)};

    REQUIRE(command.describe().size() == 1);
    CHECK(command.describe()[0].kind == ChangeKind::Positions);
    CHECK(command.describe()[0].indices == std::vector<SubtitleIndex>{kFirst});
}

TEST_CASE("a position change says which boundary it is", "[edit][setposition]") {
    // Not decoration: the strict order policy reads this to know whether a
    // sort has to follow, and only a start can break the order.
    const Project project = projectOf();
    const SetPositionCommand start{
        project, kFirst, Boundary::Start, Timestamp::fromMilliseconds(2000)};
    const SetPositionCommand end{project, kFirst, Boundary::End, Timestamp::fromMilliseconds(4000)};

    CHECK(start.kind() == CommandKind::SetStart);
    CHECK(end.kind() == CommandKind::SetEnd);
}

TEST_CASE("a position change marks both documents", "[edit][setposition]") {
    Session session{projectOf()};

    session.apply(setStart(session.project(), kFirst, 2000));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 1);
}

TEST_CASE("undoing then redoing a position change restores the exact state",
          "[edit][setposition]") {
    Session session{projectOf()};

    session.apply(setStart(session.project(), kFirst, 2000));
    session.undo();
    CHECK(session.project().subtitleAt(kFirst).start.milliseconds() == 1000);
    CHECK(session.modificationCount(Document::Main) == 0);

    session.redo();
    CHECK(session.project().subtitleAt(kFirst).start.milliseconds() == 2000);
    CHECK(session.modificationCount(Document::Main) == 1);
}

TEST_CASE("a strict session sorts after a start that broke the order", "[edit][setposition]") {
    // What `CommandKind::SetStart` is for: the policy reads the kind, and this
    // is the first real command that answers it.
    Session session{projectOf(), OrderPolicy::Strict};

    session.apply(setStart(session.project(), kSecond, 0));

    CHECK(session.project().subtitleAt(kFirst).start.milliseconds() == 0);
    CHECK(session.project().outOfOrder().empty());
    CHECK(session.undoableCount() == 1);
}

TEST_CASE("an end changed under a strict session undoes cleanly", "[edit][setposition]") {
    // That no sort is appended here is not what this test proves — on an
    // ordered project a sort moves nothing, so the two cases look identical
    // from outside. It is held by the two tests that can see it: `kind()`
    // answers `SetEnd` above, and `mayBreakOrder(SetEnd)` is false.
    Session session{projectOf(), OrderPolicy::Strict};

    session.apply(std::make_unique<SetPositionCommand>(
        session.project(), kFirst, Boundary::End, Timestamp::fromMilliseconds(9000)));
    session.undo();

    CHECK(session.project().subtitleAt(kFirst).end.milliseconds() == 3000);
    CHECK_FALSE(session.canUndo());
}
