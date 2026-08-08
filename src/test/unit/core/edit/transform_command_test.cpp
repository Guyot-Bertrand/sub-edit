#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;
using subedit::core::TransformCommand;
using subedit::core::TransformReference;

constexpr SubtitleIndex kFirst = SubtitleIndex::fromValue(0);
constexpr SubtitleIndex kMiddle = SubtitleIndex::fromValue(1);
constexpr SubtitleIndex kLast = SubtitleIndex::fromValue(2);

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

/// Three subtitles starting at 1000, 2000 and 4000.
[[nodiscard]] Project threeSubtitles() {
    Project project;
    project.setSubtitles({at(1000, 1500), at(2000, 2500), at(4000, 4500)});
    return project;
}

[[nodiscard]] TransformReference reference(SubtitleIndex index, std::int64_t target) {
    return TransformReference{.index = index, .target = Timestamp::fromMilliseconds(target)};
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

/// Builds the transform of the whole project between two references, failing
/// the test if it could not be built.
[[nodiscard]] TransformCommand
transformOf(const Project& project, TransformReference first, TransformReference second) {
    std::optional<TransformCommand> command =
        TransformCommand::create(project, Selection::all(project), first, second);
    if (command.has_value())
        return *std::move(command);

    // `FAIL` throws, so what follows never runs. It is there because the
    // compiler cannot know that, and because returning an invented command
    // would make the failure harder to read than an exception.
    FAIL("these two references define a transform");
    throw std::logic_error{"unreachable"};
}

} // namespace

TEST_CASE("both references land exactly where they were asked to", "[edit][transform]") {
    // The property the whole formula exists for. The ratio here is 4001/3000,
    // which no intermediate rounding could carry: Gaupol's `round(r × t) +
    // constant` rounds the coefficient and the constant separately and misses.
    Project project = threeSubtitles();
    TransformCommand command =
        transformOf(project, reference(kFirst, 1000), reference(kLast, 5001));

    command.apply(project);

    CHECK(project.subtitleAt(kFirst).start.milliseconds() == 1000);
    CHECK(project.subtitleAt(kLast).start.milliseconds() == 5001);
}

TEST_CASE("a subtitle between the references follows the affine correction", "[edit][transform]") {
    // (2000 − 1000) × 4001/3000 is 1333.66…, rounded once to 1334, plus 1000.
    Project project = threeSubtitles();
    TransformCommand command =
        transformOf(project, reference(kFirst, 1000), reference(kLast, 5001));

    command.apply(project);

    CHECK(project.subtitleAt(kMiddle).start.milliseconds() == 2334);
}

TEST_CASE("a transform moves the ends as well as the starts", "[edit][transform]") {
    // The correction applies to a position, and both ends are positions. A
    // transform that moved only the starts would stretch every duration.
    Project project = threeSubtitles();
    TransformCommand command =
        transformOf(project, reference(kFirst, 2000), reference(kLast, 8000));

    command.apply(project);

    // The ratio is 6000/3000, that is two: every distance from the first
    // reference doubles.
    CHECK(boundsOf(project) == std::vector<std::int64_t>{2000, 3000, 4000, 5000, 8000, 9000});
}

TEST_CASE("references given the other way round define the same transform", "[edit][transform]") {
    Project project = threeSubtitles();
    TransformCommand command =
        transformOf(project, reference(kLast, 8000), reference(kFirst, 2000));

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{2000, 3000, 4000, 5000, 8000, 9000});
}

TEST_CASE("two references on the same subtitle are refused", "[edit][transform]") {
    // No transform is defined: the two conditions bear on one position, and
    // the ratio would divide by zero.
    const Project project = threeSubtitles();

    CHECK_FALSE(
        TransformCommand::create(
            project, Selection::all(project), reference(kFirst, 1000), reference(kFirst, 5000))
            .has_value());
}

TEST_CASE("two references on subtitles that start together are refused", "[edit][transform]") {
    // Two distinct subtitles, one same start: the denominator is still null,
    // and refusing on the index alone would let this one through.
    Project project;
    project.setSubtitles({at(1000, 1500), at(1000, 2500)});

    CHECK_FALSE(
        TransformCommand::create(
            project, Selection::all(project), reference(kFirst, 1000), reference(kMiddle, 5000))
            .has_value());
}

TEST_CASE("a transform leaves the subtitles outside the selection alone", "[edit][transform]") {
    Project project = threeSubtitles();
    const std::array<SubtitleIndex, 2> selected = {kFirst, kMiddle};
    std::optional<TransformCommand> command = TransformCommand::create(
        project, Selection::of(selected), reference(kFirst, 2000), reference(kLast, 8000));
    if (!command.has_value()) {
        FAIL("these two references define a transform");
        return;
    }

    command->apply(project);

    CHECK(project.subtitleAt(kLast).start.milliseconds() == 4000);
    CHECK(project.subtitleAt(kFirst).start.milliseconds() == 2000);
}

TEST_CASE("undoing a transform restores the exact positions", "[edit][transform]") {
    // The reason the command retains the previous positions rather than a
    // ratio: the operation rounds, so its inverse cannot be a second
    // calculation — it would land a millisecond off.
    Project project = threeSubtitles();
    const std::vector<std::int64_t> before = boundsOf(project);
    TransformCommand command =
        transformOf(project, reference(kFirst, 1000), reference(kLast, 5001));

    command.apply(project);
    command.revert(project);

    CHECK(boundsOf(project) == before);
}

TEST_CASE("a transform reports the positions it moved", "[edit][transform]") {
    const Project project = threeSubtitles();
    const std::array<SubtitleIndex, 2> selected = {kFirst, kMiddle};
    const std::optional<TransformCommand> command = TransformCommand::create(
        project, Selection::of(selected), reference(kFirst, 2000), reference(kLast, 8000));
    if (!command.has_value()) {
        FAIL("these two references define a transform");
        return;
    }

    const std::vector<subedit::core::Change> changes = command->describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Positions);
    CHECK(changes[0].indices == std::vector<SubtitleIndex>{kFirst, kMiddle});
}

TEST_CASE("a transform says what it is", "[edit][transform]") {
    const Project project = threeSubtitles();
    const TransformCommand command =
        transformOf(project, reference(kFirst, 2000), reference(kLast, 8000));

    CHECK(command.kind() == CommandKind::Transform);
}

TEST_CASE("a transform marks both documents", "[edit][transform]") {
    Session session{threeSubtitles()};
    std::optional<TransformCommand> command =
        TransformCommand::create(session.project(),
                                 Selection::all(session.project()),
                                 reference(kFirst, 2000),
                                 reference(kLast, 8000));
    if (!command.has_value()) {
        FAIL("these two references define a transform");
        return;
    }

    session.apply(std::make_unique<TransformCommand>(*std::move(command)));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 1);
}

TEST_CASE("undoing then redoing a transform restores the exact state", "[edit][transform]") {
    Session session{threeSubtitles()};
    const std::vector<std::int64_t> before = boundsOf(session.project());
    std::optional<TransformCommand> command =
        TransformCommand::create(session.project(),
                                 Selection::all(session.project()),
                                 reference(kFirst, 1000),
                                 reference(kLast, 5001));
    if (!command.has_value()) {
        FAIL("these two references define a transform");
        return;
    }

    session.apply(std::make_unique<TransformCommand>(*std::move(command)));
    session.undo();
    CHECK(boundsOf(session.project()) == before);

    session.redo();
    CHECK(session.project().subtitleAt(kLast).start.milliseconds() == 5001);
}
