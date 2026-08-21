// Ce que les dialogues décident, hors des dialogues — issue #131.
//
// `QtPrompts` est la seule classe de cette bibliothèque qu'aucun test ne
// traverse : chaque méthode ouvre une boîte modale, qui tourne sa propre boucle
// d'événements jusqu'à ce que quelqu'un clique. Ce fichier éprouve les deux
// choses qu'elle décide seule, sorties de là pour cette raison : quel format un
// filtre désigne, et ce que vaut un bouton.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/qt_prompts.hpp>

#include <QMessageBox>
#include <QString>
#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::core::SubtitleFormat;
using subedit::gui::choiceOf;
using subedit::gui::formatOfFilter;
using subedit::gui::subtitleFilters;
using subedit::gui::UnsavedChoice;

} // namespace

TEST_CASE("the save filter names the format it will be written in", "[gui][GUI-SAVE-02]") {
    CHECK(formatOfFilter(QStringLiteral("WebVTT (*.vtt)")) == SubtitleFormat::WebVtt);
    CHECK(formatOfFilter(QStringLiteral("SubRip (*.srt)")) == SubtitleFormat::SubRip);
}

TEST_CASE("a filter that names both formats writes the one the project defaults to",
          "[gui][GUI-SAVE-02]") {
    // « Subtitles (*.srt *.vtt) » et « All files (*) » ne tranchent pas. SubRip
    // plutôt qu'un refus : c'est le format que le projet écrit sans qu'on lui
    // demande, et un dialogue n'a pas à échouer sur une question qu'il a posée
    // lui-même.
    CHECK(formatOfFilter(QStringLiteral("Subtitles (*.srt *.vtt)")) == SubtitleFormat::SubRip);
    CHECK(formatOfFilter(QStringLiteral("All files (*)")) == SubtitleFormat::SubRip);
}

TEST_CASE("every filter the dialog offers names a format", "[gui][GUI-SAVE-02]") {
    // Ce qui tient les deux ensemble : un filtre ajouté à la liste sans être
    // reconnu ferait écrire du SubRip sous une extension `.vtt`.
    const QStringList offered = subtitleFilters().split(QStringLiteral(";;"));

    REQUIRE(offered.size() == 4);
    CHECK(formatOfFilter(offered.at(2)) == SubtitleFormat::WebVtt);
    CHECK(formatOfFilter(offered.at(1)) == SubtitleFormat::SubRip);
}

TEST_CASE("the two explicit answers about unsaved changes are honoured", "[gui][GUI-SAVE-03]") {
    CHECK(choiceOf(QMessageBox::Save) == UnsavedChoice::Save);
    CHECK(choiceOf(QMessageBox::Discard) == UnsavedChoice::Discard);
}

TEST_CASE("anything that is not an explicit answer cancels", "[gui][GUI-SAVE-03]") {
    // Fermer la boîte par la croix, appuyer sur Échap, ou n'importe quel bouton
    // qu'on ajouterait sans y penser : rien de tout cela n'autorise à perdre un
    // travail.
    CHECK(choiceOf(QMessageBox::Cancel) == UnsavedChoice::Cancel);
    CHECK(choiceOf(QMessageBox::NoButton) == UnsavedChoice::Cancel);
    CHECK(choiceOf(QMessageBox::Ok) == UnsavedChoice::Cancel);
}
