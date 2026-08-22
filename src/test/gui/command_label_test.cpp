// The sentence an undo action reads — issue #130.
//
// Naming the operation is `core::nameOf`'s work, which the command line uses
// too; putting a verb in front of it is a menu's, and no report has any use for
// that. This file therefore tests the verb and its punctuation, and nothing
// else.

#include <subedit/core/command/command_kind.hpp>
#include <subedit/gui/command_label.hpp>

#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <optional>

namespace {

using subedit::core::CommandKind;
using subedit::gui::redoLabel;
using subedit::gui::undoLabel;

} // namespace

TEST_CASE("an action names the operation it would defeat", "[gui][GUI-UNDO-01]") {
    CHECK(undoLabel(CommandKind::Shift).toStdString() == "Undo: shifting");
    CHECK(undoLabel(CommandKind::SetText).toStdString() == "Undo: editing a text");
}

TEST_CASE("with nothing to defeat, the action says only what it is", "[gui][GUI-UNDO-01]") {
    // Not « Undo: nothing »: the action is disabled then, and making it say
    // something about its own emptiness would be noise.
    CHECK(undoLabel(std::nullopt).toStdString() == "Undo");
    CHECK(redoLabel(std::nullopt).toStdString() == "Redo");
}

TEST_CASE("redoing names what it would replay", "[gui][GUI-UNDO-02]") {
    CHECK(redoLabel(CommandKind::Sort).toStdString() == "Redo: sorting");
}
