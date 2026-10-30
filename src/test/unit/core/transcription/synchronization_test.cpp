// What both surfaces do with a matching: which subtitles are looked for, the plan, the command
// (issue #814, phase 23).

#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/transcription/synchronization.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

using subedit::core::LineMatch;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

Project projectOf(std::size_t count) {
    Project project;
    std::vector<subedit::core::Subtitle> subtitles;
    for (std::size_t line = 0; line < count; ++line) {
        const auto start = static_cast<std::int64_t>(line) * 3000;
        subtitles.push_back({.start = at(start),
                             .end = at(start + 1000),
                             .mainText = "line " + std::to_string(line)});
    }
    project.setSubtitles(std::move(subtitles));
    return project;
}

LineMatch found(std::int64_t start) {
    return {.place = {.start = at(start), .end = at(start + 900)}, .words = 2, .matched = 2};
}

} // namespace

TEST_CASE("the subtitles that begin in a range are the ones looked for", "[synchronization]") {
    const Project project = projectOf(5);

    const auto all = subedit::core::subtitlesBeginningIn(project, std::nullopt, std::nullopt);
    CHECK(all.subtitles.size() == 5);
    CHECK(all.lines.front() == "line 0");

    const auto inside = subedit::core::subtitlesBeginningIn(project, at(3000), at(9000));
    CHECK(inside.subtitles == std::vector<std::size_t>{1, 2, 3});

    const auto after = subedit::core::subtitlesBeginningIn(project, at(7000), std::nullopt);
    CHECK(after.subtitles == std::vector<std::size_t>{3, 4});
}

TEST_CASE("the subtitles of a selection are the ones looked for", "[synchronization]") {
    const Project project = projectOf(5);

    const auto picked = subedit::core::subtitlesOf(
        project, Selection::range(SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(2)));

    CHECK(picked.subtitles == std::vector<std::size_t>{1, 2});
    CHECK(picked.lines == std::vector<std::string>{"line 1", "line 2"});
}

TEST_CASE("a plan says where each subtitle goes, and its command puts them there as one entry",
          "[synchronization]") {
    Project project = projectOf(3);
    const std::vector<LineMatch> matches{
        found(500), LineMatch{.words = 2, .matched = 0}, found(6500)};

    const auto plan = subedit::core::planSynchronization(project, {0, 1, 2}, matches, {});

    CHECK(plan.report.notFound == 1);
    CHECK(plan.retimed[0].times.start == at(500));
    CHECK_FALSE(plan.retimed[1].moved);
    CHECK(plan.originals[2].start == at(6000));

    auto command = subedit::core::synchronizationCommand(project, plan);
    REQUIRE(command != nullptr);
    CHECK(command->kind() == subedit::core::CommandKind::Synchronize);
    command->apply(project);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).start == at(500));
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(1)).start == at(3000));
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(2)).start == at(6500));
    command->revert(project);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).start == at(0));
}

TEST_CASE("a subtitle heard where it already is does not count as moved", "[synchronization]") {
    const Project project = projectOf(2);
    const auto exact = [](std::int64_t start) {
        return LineMatch{
            .place = {.start = at(start), .end = at(start + 1000)}, .words = 2, .matched = 2};
    };
    const std::vector<LineMatch> matches{exact(0), exact(3000)};

    const auto plan = subedit::core::planSynchronization(project, {0, 1}, matches, {});

    CHECK(plan.changed() == 0);
    CHECK_FALSE(plan.changes(0));
    CHECK(subedit::core::synchronizationCommand(project, plan) == nullptr);
}

TEST_CASE("nothing moves, no command", "[synchronization]") {
    const Project project = projectOf(2);
    const std::vector<LineMatch> matches{LineMatch{.words = 2, .matched = 0},
                                         LineMatch{.words = 2, .matched = 0}};

    const auto plan = subedit::core::planSynchronization(project, {0, 1}, matches, {});

    CHECK(subedit::core::synchronizationCommand(project, plan) == nullptr);
}
