#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/command/history.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "fake_commands.hpp"

namespace {

using subedit::core::ChangeKind;
using subedit::core::Command;
using subedit::core::CommandKind;
using subedit::core::CompositeCommand;
using subedit::core::Document;
using subedit::core::History;
using subedit::core::Project;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::test::Declaring;
using subedit::test::SetMainText;

Project withThreeSubtitles() {
    Project project;
    project.setSubtitles({
        Subtitle{.mainText = "Un."},
        Subtitle{.mainText = "Deux."},
        Subtitle{.mainText = "Trois."},
    });
    return project;
}

std::unique_ptr<Command> setFirstText(const Project& project, std::string text) {
    return std::make_unique<SetMainText>(project, SubtitleIndex::fromValue(0), std::move(text));
}

} // namespace

TEST_CASE("a fresh history has nothing to undo or redo", "[command][history]") {
    const History history;

    CHECK_FALSE(history.canUndo());
    CHECK_FALSE(history.canRedo());
    CHECK(history.undoableCount() == 0);
    CHECK(history.redoableCount() == 0);
}

TEST_CASE("applying a command changes the project and becomes undoable", "[command][history]") {
    Project project = withThreeSubtitles();
    History history;

    history.apply(setFirstText(project, "Premier."), project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Premier.");
    CHECK(history.canUndo());
    CHECK(history.undoableCount() == 1);
}

TEST_CASE("undoing then redoing restores the exact state", "[command][history]") {
    Project project = withThreeSubtitles();
    History history;
    history.apply(setFirstText(project, "Premier."), project);

    history.undo(project);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
    CHECK_FALSE(history.canUndo());
    CHECK(history.canRedo());

    history.redo(project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Premier.");
    CHECK(history.canUndo());
    CHECK_FALSE(history.canRedo());
}

TEST_CASE("a new command drops what could have been redone", "[command][history]") {
    // Otherwise redo would replay a command whose starting state no longer
    // exists.
    Project project = withThreeSubtitles();
    History history;
    history.apply(setFirstText(project, "Premier."), project);
    history.undo(project);
    REQUIRE(history.canRedo());

    history.apply(setFirstText(project, "Autre."), project);

    CHECK_FALSE(history.canRedo());
    CHECK(history.undoableCount() == 1);
}

TEST_CASE("undoing or redoing nothing is harmless", "[command][history]") {
    Project project = withThreeSubtitles();
    History history;

    history.undo(project);
    history.redo(project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
    CHECK(history.modificationCount(Document::Main) == 0);
}

TEST_CASE("a group of seven commands is undone in one go", "[command][history]") {
    Project project = withThreeSubtitles();
    History history;
    constexpr int kGroupSize = 7;
    std::vector<std::unique_ptr<Command>> group;
    group.reserve(kGroupSize);
    for (int step = 0; step < kGroupSize; ++step)
        group.push_back(setFirstText(project, "étape " + std::to_string(step)));

    history.apply(std::make_unique<CompositeCommand>(CommandKind::SetText, std::move(group)),
                  project);

    CHECK(history.undoableCount() == 1);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "étape 6");

    history.undo(project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
    CHECK(history.modificationCount(Document::Main) == 0);
}

TEST_CASE("a group counts as one modification, not as many", "[command][history]") {
    // Returning to zero after undoing proves nothing on its own: a counter
    // that went up seven times and back down seven times would also land
    // there. What has to hold is that the group moved it **once**, which is
    // what makes the count mean « how far from the file » rather than « how
    // many commands ran ».
    Project project = withThreeSubtitles();
    History history;
    constexpr int kGroupSize = 7;
    std::vector<std::unique_ptr<Command>> group;
    group.reserve(kGroupSize);
    for (int step = 0; step < kGroupSize; ++step)
        group.push_back(setFirstText(project, "étape " + std::to_string(step)));

    history.apply(std::make_unique<CompositeCommand>(CommandKind::SetText, std::move(group)),
                  project);

    CHECK(history.modificationCount(Document::Main) == 1);
}

TEST_CASE("the modification counter follows the actions and their undoing", "[command][history]") {
    // An integer rather than a boolean, precisely so that undoing back to the
    // save point says « unmodified » again.
    Project project;
    History history;

    history.apply(std::make_unique<Declaring>(ChangeKind::MainText), project);
    history.apply(std::make_unique<Declaring>(ChangeKind::MainText), project);
    CHECK(history.modificationCount(Document::Main) == 2);
    CHECK(history.hasUnsavedChanges(Document::Main));

    history.undo(project);
    history.undo(project);

    CHECK(history.modificationCount(Document::Main) == 0);
    CHECK_FALSE(history.hasUnsavedChanges(Document::Main));
}

TEST_CASE("the counter is kept per document", "[command][history]") {
    Project project;
    History history;

    history.apply(std::make_unique<Declaring>(ChangeKind::TranslationText), project);

    CHECK(history.modificationCount(Document::Main) == 0);
    CHECK(history.modificationCount(Document::Translation) == 1);
}

TEST_CASE("a change of positions marks both documents", "[command][history]") {
    Project project;
    History history;

    history.apply(std::make_unique<Declaring>(ChangeKind::Positions), project);

    CHECK(history.modificationCount(Document::Main) == 1);
    CHECK(history.modificationCount(Document::Translation) == 1);
}

TEST_CASE("saving moves the point the counter returns to", "[command][history]") {
    Project project;
    History history;
    history.apply(std::make_unique<Declaring>(ChangeKind::MainText), project);

    history.markSaved(Document::Main);
    CHECK(history.modificationCount(Document::Main) == 0);

    history.undo(project);

    // Undoing past the save point is a difference too, in the other direction.
    CHECK(history.modificationCount(Document::Main) == -1);
    CHECK(history.hasUnsavedChanges(Document::Main));
}

TEST_CASE("the history is bounded and drops its oldest entries", "[command][history]") {
    Project project = withThreeSubtitles();
    History history{3};

    for (int step = 0; step < 5; ++step)
        history.apply(setFirstText(project, "étape " + std::to_string(step)), project);

    CHECK(history.undoableCount() == 3);
    CHECK(history.canUndo());
}

TEST_CASE("clearing the history leaves the project untouched", "[command][history]") {
    Project project = withThreeSubtitles();
    History history;
    history.apply(setFirstText(project, "Premier."), project);

    history.clear();

    CHECK_FALSE(history.canUndo());
    CHECK_FALSE(history.canRedo());
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Premier.");
}
