// Les trois dialogues d'opération — issue #132.
//
// **Ils s'éprouvent sans jamais entrer dans `exec()`.** Ce sont nos widgets :
// un test en construit un, remplit ses champs et lit ce qu'il en fait. Seule
// la boucle modale reste hors d'atteinte, et elle est derrière `Prompts::run`.

#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QLineEdit>
#include <catch2/catch_test_macros.hpp>

#include <optional>

namespace {

using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::StandardFrameRate;
using subedit::core::Timestamp;
using subedit::gui::FrameRateDialog;
using subedit::gui::ShiftDialog;
using subedit::gui::TransformDialog;
using subedit::gui::TypedReference;

} // namespace

TEST_CASE("the shift dialog reads a signed duration", "[gui][GUI-SHIFT-01]") {
    ShiftDialog dialog{4};

    dialog.setTyped(QStringLiteral("00:00:02,500"));
    CHECK(dialog.shift() == Duration::fromMilliseconds(2500));

    dialog.setTyped(QStringLiteral("-0:01,250"));
    CHECK(dialog.shift() == Duration::fromMilliseconds(-1250));
}

TEST_CASE("a shift that cannot be read is no shift at all", "[gui][GUI-SHIFT-01]") {
    // Plutôt que d'inventer une durée. Le dialogue refuse alors d'être validé,
    // ce qui est la seule façon de ne pas décaler un fichier au hasard.
    ShiftDialog dialog{4};

    dialog.setTyped(QStringLiteral("bientôt"));

    CHECK_FALSE(dialog.shift().has_value());
    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("the shift dialog says what it is about to touch", "[gui][GUI-SHIFT-01]") {
    // Le compte, parce que « la sélection ou tout le fichier » est une règle
    // qu'on ne devine pas devant une boîte de dialogue.
    const ShiftDialog whole{4};
    const ShiftDialog some{2};

    CHECK(whole.targetLabel().toStdString() == "4 subtitles");
    CHECK(some.targetLabel().toStdString() == "2 subtitles");
}

TEST_CASE("the transform dialog reads two references", "[gui][GUI-TRANSFORM-01]") {
    TransformDialog dialog{4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("00:00:09,000"));

    // Le repère entier plutôt que ses champs : clang-tidy ne reconnaît pas le
    // REQUIRE de Catch2 comme une vérification, et la convention du projet est
    // de comparer l'option elle-même.
    CHECK(dialog.first() ==
          TypedReference{.number = 1, .target = Timestamp::fromMilliseconds(1000)});
    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
    CHECK(dialog.isComplete());
}

TEST_CASE("two references on the same subtitle define no transform", "[gui][GUI-TRANSFORM-01]") {
    // Le noyau le refuse déjà — `TransformCommand::create` rend `nullopt` sur
    // un dénominateur nul. Le dialogue le dit avant, plutôt que de laisser
    // l'utilisateur valider pour rien.
    TransformDialog dialog{4};

    dialog.setTyped(2, QStringLiteral("00:00:01,000"), 2, QStringLiteral("00:00:09,000"));

    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("a reference outside the file cannot be asked for", "[gui][GUI-TRANSFORM-01]") {
    // Non pas refusé après coup, mais impossible à saisir : le champ est borné
    // par le nombre de sous-titres. Un neuvième repère dans un fichier de
    // quatre retombe sur le quatrième.
    TransformDialog dialog{4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 9, QStringLiteral("00:00:09,000"));

    CHECK(dialog.second() ==
          TypedReference{.number = 4, .target = Timestamp::fromMilliseconds(9000)});
}

TEST_CASE("an unreadable reference position is refused", "[gui][GUI-TRANSFORM-01]") {
    TransformDialog dialog{4};

    dialog.setTyped(1, QStringLiteral("00:00:01,000"), 4, QStringLiteral("plus tard"));

    CHECK_FALSE(dialog.second().has_value());
    CHECK_FALSE(dialog.isComplete());
}

TEST_CASE("the frame rate dialog opens on the rate of the project", "[gui][GUI-FRAMERATE-01]") {
    // Pré-rempli, et sans heuristique : le fichier ne porte pas sa fréquence,
    // et se tromper décale tout sans rien signaler.
    const FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps25});
}

TEST_CASE("the frame rate dialog reads both rates", "[gui][GUI-FRAMERATE-01]") {
    FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    dialog.setRates(FrameRate{StandardFrameRate::Fps23976}, FrameRate{StandardFrameRate::Fps25});

    CHECK(dialog.input() == FrameRate{StandardFrameRate::Fps23976});
    CHECK(dialog.output() == FrameRate{StandardFrameRate::Fps25});
    CHECK(dialog.isComplete());
}

TEST_CASE("converting a rate into itself changes nothing, and the dialog says so",
          "[gui][GUI-FRAMERATE-01]") {
    FrameRateDialog dialog{4, FrameRate{StandardFrameRate::Fps25}};

    dialog.setRates(FrameRate{StandardFrameRate::Fps25}, FrameRate{StandardFrameRate::Fps25});

    CHECK_FALSE(dialog.isComplete());
}
