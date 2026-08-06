#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include "fake_commands.hpp"

namespace {

using subedit::core::Change;
using subedit::core::ChangeKind;
using subedit::core::Command;
using subedit::core::CompositeCommand;
using subedit::core::Project;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::test::SetMainText;
using subedit::test::Tracing;

Project withThreeSubtitles() {
    Project project;
    project.setSubtitles({
        Subtitle{.mainText = "Un."},
        Subtitle{.mainText = "Deux."},
        Subtitle{.mainText = "Trois."},
    });
    return project;
}

} // namespace

TEST_CASE("a composite applies its commands in order", "[command][composite]") {
    std::vector<std::string> trace;
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<Tracing>(trace, "premier"));
    commands.push_back(std::make_unique<Tracing>(trace, "second"));
    CompositeCommand composite{std::move(commands)};
    Project project;

    composite.apply(project);

    CHECK(trace == std::vector<std::string>{"apply premier", "apply second"});
}

TEST_CASE("a composite reverts its commands in reverse order", "[command][composite]") {
    // The order is not a detail: undoing « insert then shift » by undoing the
    // insertion first would shift subtitles that are no longer there.
    std::vector<std::string> trace;
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(std::make_unique<Tracing>(trace, "premier"));
    commands.push_back(std::make_unique<Tracing>(trace, "second"));
    CompositeCommand composite{std::move(commands)};
    Project project;

    composite.revert(project);

    CHECK(trace == std::vector<std::string>{"revert second", "revert premier"});
}

TEST_CASE("a composite restores the exact state it started from", "[command][composite]") {
    Project project = withThreeSubtitles();
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(
        std::make_unique<SetMainText>(project, SubtitleIndex::fromValue(0), "Premier."));
    commands.push_back(
        std::make_unique<SetMainText>(project, SubtitleIndex::fromValue(2), "Troisième."));
    CompositeCommand composite{std::move(commands)};

    composite.apply(project);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Premier.");
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(2)).mainText == "Troisième.");

    composite.revert(project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(1)).mainText == "Deux.");
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(2)).mainText == "Trois.");
}

TEST_CASE("a composite reports what all of its commands touched", "[command][composite]") {
    const Project project = withThreeSubtitles();
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(
        std::make_unique<SetMainText>(project, SubtitleIndex::fromValue(0), "Premier."));
    commands.push_back(
        std::make_unique<SetMainText>(project, SubtitleIndex::fromValue(2), "Troisième."));
    const CompositeCommand composite{std::move(commands)};

    const std::vector<Change> changes = composite.describe();

    REQUIRE(changes.size() == 2);
    CHECK(changes[0].kind == ChangeKind::MainText);
    CHECK(changes[0].indices == std::vector{SubtitleIndex::fromValue(0)});
    CHECK(changes[1].indices == std::vector{SubtitleIndex::fromValue(2)});
}

TEST_CASE("an empty composite does nothing and says so", "[command][composite]") {
    CompositeCommand composite{{}};
    Project project = withThreeSubtitles();

    composite.apply(project);
    composite.revert(project);

    CHECK(composite.describe().empty());
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
}
