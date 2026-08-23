// What the dialogs decide, outside the dialogs — issue #131.
//
// `QtPrompts` is the one class of this library no test enters: each method
// opens a modal box, which spins its own event loop until somebody clicks. This
// file tests the things it decides on its own, taken out of there for that very
// reason: which format a filter names, what a button is worth, and which window
// its boxes sit over.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/video_file.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/qt_prompts.hpp>

#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <string_view>

namespace {

using subedit::core::SubtitleFormat;
using subedit::gui::choiceOf;
using subedit::gui::formatOfFilter;
using subedit::gui::QtPrompts;
using subedit::gui::subtitleFilters;
using subedit::gui::UnsavedChoice;
using subedit::gui::videoFilters;

} // namespace

TEST_CASE("the save filter names the format it will be written in", "[gui][GUI-SAVE-02]") {
    CHECK(formatOfFilter(QStringLiteral("WebVTT (*.vtt)")) == SubtitleFormat::WebVtt);
    CHECK(formatOfFilter(QStringLiteral("SubRip (*.srt)")) == SubtitleFormat::SubRip);
}

TEST_CASE("a filter that names both formats writes the one the project defaults to",
          "[gui][GUI-SAVE-02]") {
    // « Subtitles (*.srt *.vtt) » and « All files (*) » settle nothing. SubRip
    // rather than a refusal: it is the format the project writes when nobody
    // asks for another, and a dialog has no business failing on a question it
    // asked itself.
    CHECK(formatOfFilter(QStringLiteral("Subtitles (*.srt *.vtt)")) == SubtitleFormat::SubRip);
    CHECK(formatOfFilter(QStringLiteral("All files (*)")) == SubtitleFormat::SubRip);
}

TEST_CASE("every filter the dialog offers names a format", "[gui][GUI-SAVE-02]") {
    // What holds the two together: a filter added to the list without being
    // recognised would write SubRip under a `.vtt` extension.
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
    // Closing the box by its cross, pressing Escape, or any button somebody
    // adds without thinking: none of that authorises losing work.
    CHECK(choiceOf(QMessageBox::Cancel) == UnsavedChoice::Cancel);
    CHECK(choiceOf(QMessageBox::NoButton) == UnsavedChoice::Cancel);
    CHECK(choiceOf(QMessageBox::Ok) == UnsavedChoice::Cancel);
}

TEST_CASE("the boxes sit over the window that took them", "[gui][GUI-SAVE-03]") {
    // They used to sit over nothing: `subedit-gui` built its prompts before the
    // window — it has to, the window takes a reference to them — and passed
    // `nullptr` for want of anything better. No file box, no message box was
    // ever placed on the window.
    //
    // The window says it itself now, at its own construction, which is the one
    // place it cannot be forgotten from.
    QWidget window;
    QtPrompts prompts;

    CHECK(prompts.owner() == nullptr);

    prompts.ownedBy(&window);

    CHECK(prompts.owner() == &window);
}

// The chooser and the recognition read one list, which is the whole point of
// building the filter rather than writing it: a chooser that offered a file the
// rest of the program refuses to call a video would be a trap, and one that hid
// a file it accepts would be a mystery.
TEST_CASE("the video chooser filters on the extensions the core recognises", "[gui]") {
    const QStringList offered = videoFilters().split(QStringLiteral(";;"));

    REQUIRE(offered.size() == 2);
    for (const std::string_view extension : subedit::core::videoExtensions()) {
        const QString pattern =
            QStringLiteral("*") + QString::fromUtf8(extension.data(), qsizetype(extension.size()));
        CHECK(offered.at(0).contains(pattern));
        CHECK(subedit::core::isVideoFile("film" + std::string{extension}));
    }

    CHECK(offered.at(1) == QStringLiteral("All files (*)"));
}
