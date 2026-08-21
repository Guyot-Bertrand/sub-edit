// Le nom qu'une action d'annulation donne à ce qu'elle défera — issue #130.
//
// La traduction vit dans `subedit::gui` et non dans le noyau, qui n'a rien à
// dire en français : `CommandKind` existe pour être traduit, et son commentaire
// le dit depuis la phase 2.

#include <subedit/core/command/command_kind.hpp>
#include <subedit/gui/command_label.hpp>

#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <optional>
#include <set>
#include <string>

namespace {

using subedit::core::CommandKind;
using subedit::gui::labelOf;
using subedit::gui::redoLabel;
using subedit::gui::undoLabel;

constexpr std::array kEveryKind = {
    CommandKind::SetText,
    CommandKind::SetStart,
    CommandKind::SetEnd,
    CommandKind::Insert,
    CommandKind::Remove,
    CommandKind::Shift,
    CommandKind::Transform,
    CommandKind::ConvertFrameRate,
    CommandKind::Sort,
    CommandKind::RemoveHearingImpaired,
};

} // namespace

TEST_CASE("every kind of command has a name of its own", "[gui][GUI-UNDO-01]") {
    // Deux noms identiques rendraient l'action ambiguë, et un nom vide la
    // rendrait muette. Le compilateur tient déjà l'exhaustivité du `switch` ;
    // ce test tient ce qu'il ne peut pas voir.
    std::set<std::string> seen;

    for (const CommandKind kind : kEveryKind) {
        const std::string name = labelOf(kind).toStdString();
        CHECK_FALSE(name.empty());
        seen.insert(name);
    }

    CHECK(seen.size() == kEveryKind.size());
}

TEST_CASE("an action names the operation it would defeat", "[gui][GUI-UNDO-01]") {
    CHECK(undoLabel(CommandKind::Shift).toStdString() == "Annuler : décalage");
    CHECK(undoLabel(CommandKind::SetText).toStdString() == "Annuler : modification du texte");
}

TEST_CASE("with nothing to defeat, the action says only what it is", "[gui][GUI-UNDO-01]") {
    // Pas « Annuler : rien » : l'action est alors inactive, et lui faire dire
    // quelque chose de son vide serait du bruit.
    CHECK(undoLabel(std::nullopt).toStdString() == "Annuler");
    CHECK(redoLabel(std::nullopt).toStdString() == "Rétablir");
}

TEST_CASE("redoing names what it would replay", "[gui][GUI-UNDO-02]") {
    CHECK(redoLabel(CommandKind::Sort).toStdString() == "Rétablir : tri");
}
