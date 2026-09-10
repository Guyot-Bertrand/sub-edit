// What the dialogs decide, outside the dialogs — issue #131.
//
// `QtPrompts` is the one class of this library no test enters: each method
// opens a modal box, which spins its own event loop until somebody clicks. This
// file tests the things it decides on its own, taken out of there for that very
// reason: which format a filter names, what a button is worth, and which window
// its boxes sit over.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/video_file.hpp>
#include <subedit/core/wording.hpp>
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
using subedit::gui::filterFor;
using subedit::gui::formatOfFilter;
using subedit::gui::QtPrompts;
using subedit::gui::subtitleFilters;
using subedit::gui::UnsavedChoice;
using subedit::gui::videoFilters;

} // namespace

TEST_CASE("the save filter names the format it will be written in", "[gui][GUI-FORMAT-02]") {
    // **Each of the nine, and read from the name.** Two extensions name two
    // formats each — `.sub` is SubViewer 2 and MicroDVD, `.txt` is MPL2 and
    // TMPlayer — so a filter told apart by its pattern would write one of the
    // pair under the other's name.
    for (const SubtitleFormat format : subedit::core::kSubtitleFormats) {
        INFO("format : " << subedit::core::nameOf(format));
        CHECK(formatOfFilter(filterFor(format)) == format);
    }

    CHECK(formatOfFilter(QStringLiteral("MicroDVD (*.sub)")) == SubtitleFormat::MicroDvd);
    CHECK(formatOfFilter(QStringLiteral("SubViewer 2 (*.sub)")) == SubtitleFormat::SubViewer2);
}

TEST_CASE("a filter that names no format writes the one the project defaults to",
          "[gui][GUI-FORMAT-02]") {
    // The first entry shows everything and « All files (*) » settles nothing.
    // SubRip rather than a refusal: it is the format the project writes when
    // nobody asks for another, and a dialog has no business failing on a
    // question it asked itself.
    CHECK(formatOfFilter(subtitleFilters().section(QStringLiteral(";;"), 0, 0)) ==
          SubtitleFormat::SubRip);
    CHECK(formatOfFilter(QStringLiteral("All files (*)")) == SubtitleFormat::SubRip);
}

TEST_CASE("the dialog offers the nine formats, and each names itself back",
          "[gui][GUI-FORMAT-01][GUI-FORMAT-02]") {
    // What holds the list and the recognition together: an entry added without
    // being recognised would write SubRip under somebody else's extension.
    const QStringList offered = subtitleFilters().split(QStringLiteral(";;"));

    // Everything, the nine, and anything.
    REQUIRE(offered.size() == 11);
    CHECK(offered.at(10) == QStringLiteral("All files (*)"));

    // The nine sit between the two, in the order the vocabulary declares them.
    qsizetype row = 1;
    for (const SubtitleFormat format : subedit::core::kSubtitleFormats) {
        INFO("format : " << subedit::core::nameOf(format));
        CHECK(offered.at(row) == filterFor(format));
        CHECK(formatOfFilter(offered.at(row)) == format);
        ++row;
    }
}

TEST_CASE("the first entry shows every extension the nine carry, once each",
          "[gui][GUI-FORMAT-01]") {
    // **The entry a user leaves selected**, so a file of any of the nine has to
    // show through it. Once each, because two of the extensions are shared and
    // a pattern written twice is a pattern nobody reads.
    const QString everything = subtitleFilters().section(QStringLiteral(";;"), 0, 0);

    for (const SubtitleFormat format : subedit::core::kSubtitleFormats) {
        const QString pattern =
            QStringLiteral("*") + QString::fromUtf8(subedit::core::extensionOf(format));
        INFO("motif : " << pattern.toStdString());
        CHECK(everything.contains(pattern));
        CHECK(everything.count(pattern) == 1);
    }
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
