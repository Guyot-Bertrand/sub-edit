// La phrase qu'une action d'annulation lit — issue #130.
//
// Nommer l'opération est le travail de `core::nameOf`, que la ligne de commande
// utilise aussi ; mettre un verbe devant est celui d'un menu, et aucun rapport
// n'en a l'usage. Ce fichier n'éprouve donc que le verbe et sa ponctuation.

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
    // Pas « Undo: nothing » : l'action est alors inactive, et lui faire dire
    // quelque chose de son vide serait du bruit.
    CHECK(undoLabel(std::nullopt).toStdString() == "Undo");
    CHECK(redoLabel(std::nullopt).toStdString() == "Redo");
}

TEST_CASE("redoing names what it would replay", "[gui][GUI-UNDO-02]") {
    CHECK(redoLabel(CommandKind::Sort).toStdString() == "Redo: sorting");
}
