// Opening, saving and closing a project that holds a translation — issue #432.
//
// **Two documents, two files, two states.** Everything below is driven through
// the window's own entries, the way a user reaches it: the file the chooser
// hands back, the answer the dialog is given, the box that comes up. The core's
// alignment is tested in the core; what is under test here is what the window
// adds — the question asked before a translation is replaced, the account of
// what happened, and the independence of the two documents.

#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/open_translation_dialog.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::TranslationMethod;
using subedit::gui::MainWindow;
using subedit::gui::OpenTranslationDialog;
using subedit::gui::SaveTarget;
using subedit::gui::UnsavedChoice;
using subedit::gui::UnsavedDocumentsDialog;
using subedit::test::FakePrompts;

constexpr int kTextColumn = 4;
constexpr int kTranslationColumn = 5;

constexpr const char* kMain = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

/// One line for each subtitle, at the same positions.
constexpr const char* kFull = "1\n00:00:01,000 --> 00:00:02,000\nOne.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nTwo.\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\nThree.\n\n";

/// The second subtitle has no line. By position the third line finds the third
/// subtitle; by number it lands on the second.
constexpr const char* kMissingMiddle = "1\n00:00:01,000 --> 00:00:02,000\nOne.\n\n"
                                       "2\n00:00:05,000 --> 00:00:06,000\nThree.\n\n";

[[nodiscard]] InMemoryFileSystem filesystem() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kMain);
    files.addFile("film.en.srt", kFull);
    files.addFile("manquante.srt", kMissingMiddle);
    return files;
}

[[nodiscard]] subedit::core::OpenedFile mainOf(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string cellAt(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// Plays the user of `File ▸ Open Translation…`: the file the chooser gives
/// back, and the method the dialog is left on or told to change.
void openTranslation(MainWindow& window,
                     FakePrompts& prompts,
                     const char* path,
                     TranslationMethod method = TranslationMethod::Position) {
    prompts.nextFileToOpen = path;
    prompts.nextRun = true;
    prompts.fill = [method](QDialog& dialog) {
        if (auto* open = dynamic_cast<OpenTranslationDialog*>(&dialog))
            (method == TranslationMethod::Number ? open->numberButton() : open->positionButton())
                ->setChecked(true);
    };
    window.openTranslationAction()->trigger();
}

/// Types into a cell of the table, as an edit the window carries out.
[[nodiscard]] bool edit(const MainWindow& window, int row, int column, const char* text) {
    return window.table()->model()->setData(
        window.table()->model()->index(row, column), QString::fromUtf8(text), Qt::EditRole);
}

} // namespace

TEST_CASE("the entry that opens a translation is out with nothing to align it to",
          "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window{files, subedit::core::OpenedFile{}, prompts};

    CHECK_FALSE(window.openTranslationAction()->isEnabled());
}

TEST_CASE("a translation opened by position lands on the subtitles it falls in",
          "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "manquante.srt", TranslationMethod::Position);

    CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
    CHECK(cellAt(window, 1, kTranslationColumn).empty());
    CHECK(cellAt(window, 2, kTranslationColumn) == "Three.");
}

TEST_CASE("a translation opened by number goes to the nth subtitle", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "manquante.srt", TranslationMethod::Number);

    CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
    CHECK(cellAt(window, 1, kTranslationColumn) == "Three.");
    CHECK(cellAt(window, 2, kTranslationColumn).empty());
}

TEST_CASE("the main text is not touched by opening a translation", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "film.en.srt");

    CHECK(cellAt(window, 0, kTextColumn) == "Un.");
    CHECK(cellAt(window, 2, kTextColumn) == "Trois.");
    CHECK_FALSE(window.table()->isColumnHidden(kTranslationColumn));
}

TEST_CASE("the file chooser opens where the last file was", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    files.addFile("dossier/film.en.srt", kFull);
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};

    openTranslation(window, prompts, "dossier/film.en.srt");
    openTranslation(window, prompts, "film.en.srt");

    CHECK(prompts.lastOpenDirectory == std::filesystem::path{"dossier"});
}

TEST_CASE("giving up at the chooser or at the dialog opens nothing", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    prompts.nextFileToOpen = std::nullopt;
    window.openTranslationAction()->trigger();
    CHECK(window.table()->isColumnHidden(kTranslationColumn));
    CHECK_FALSE(window.undoAction()->isEnabled());

    prompts.nextFileToOpen = "film.en.srt";
    prompts.nextRun = false;
    window.openTranslationAction()->trigger();
    CHECK(window.table()->isColumnHidden(kTranslationColumn));
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("the main file cannot be opened as its own translation", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "film.srt");

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front().starts_with("film.srt: "));
    CHECK(window.table()->isColumnHidden(kTranslationColumn));
}

TEST_CASE("a translation that is not there says so, and changes nothing", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "absente.srt");

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front().starts_with("absente.srt: "));
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("the dialog says which file it is about", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    const MainWindow window{files, mainOf(files), prompts};
    std::string said;
    prompts.nextFileToOpen = "film.en.srt";
    prompts.nextRun = false;
    prompts.fill = [&said](QDialog& dialog) {
        if (const auto* open = dynamic_cast<OpenTranslationDialog*>(&dialog))
            said = open->text().toStdString();
    };

    window.openTranslationAction()->trigger();

    CHECK(said.find("film.en.srt") != std::string::npos);
}

TEST_CASE("an alignment where every line found its place is said in the status bar",
          "[gui][GUI-TRANS-02]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "film.en.srt");

    CHECK(prompts.outcomes.empty());
    CHECK(window.statusBar()->currentMessage().toStdString() ==
          subedit::core::noticeOf(subedit::core::TranslationOutcome{.attached = 3}));
}

TEST_CASE("an alignment that left a subtitle alone is said in a box to close",
          "[gui][GUI-TRANS-02]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "manquante.srt");

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() == subedit::core::noticeOf(subedit::core::TranslationOutcome{
                                          .attached = 2, .untranslated = 1}));
}

TEST_CASE("opening a translation is one entry of the history, and undoing it gives everything back",
          "[gui][GUI-TRANS-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE_FALSE(window.table()->isColumnHidden(kTranslationColumn));

    CHECK(window.undoAction()->text().toStdString() == "Undo: opening a translation");
    window.undoAction()->trigger();

    // The translation is gone: its texts, and with them the column.
    CHECK(window.table()->isColumnHidden(kTranslationColumn));
    CHECK(cellAt(window, 0, kTranslationColumn).empty());
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("opening a translation shows the column the user had hidden", "[gui][GUI-TRANS-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    window.translationColumnAction()->trigger();
    REQUIRE(window.table()->isColumnHidden(kTranslationColumn));

    openTranslation(window, prompts, "film.en.srt");

    CHECK_FALSE(window.table()->isColumnHidden(kTranslationColumn));
}

TEST_CASE("a translation that has just been opened is not modified", "[gui][GUI-TRANS-03]") {
    // Nothing was typed: it is what its file says. Closing must not ask.
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    openTranslation(window, prompts, "film.en.srt");

    CHECK_FALSE(window.isWindowModified());
    CHECK(window.close());
    CHECK(prompts.unsavedAsked == 0);
    CHECK(prompts.runAsked == 1);
}

TEST_CASE("a subtitle born of a line makes the main document modified, and only it",
          "[gui][GUI-TRANS-03]") {
    // The translation is what its file says. The main document, which has just
    // been given a subtitle, is not what its file says any more.
    InMemoryFileSystem files = filesystem();
    files.addFile("plus.srt",
                  "1\n00:00:01,000 --> 00:00:02,000\nOne.\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\nTwo.\n\n"
                  "3\n00:00:05,000 --> 00:00:06,000\nThree.\n\n"
                  "4\n00:00:07,000 --> 00:00:08,000\nFour.\n\n");
    FakePrompts prompts;
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;
    MainWindow window{files, mainOf(files), prompts};
    window.show();

    openTranslation(window, prompts, "plus.srt");

    REQUIRE(window.table()->model()->rowCount({}) == 4);
    CHECK(window.isWindowModified());
    CHECK_FALSE(window.close());
    CHECK(prompts.lastUnsavedDocument == Document::Main);
}

TEST_CASE("a modified translation is offered for saving before it is replaced",
          "[gui][GUI-TRANS-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));

    SECTION("cancelling keeps the translation as it is") {
        prompts.nextUnsavedChoice = UnsavedChoice::Cancel;

        openTranslation(window, prompts, "manquante.srt");

        CHECK(prompts.unsavedAsked == 1);
        CHECK(prompts.lastUnsavedDocument == Document::Translation);
        CHECK(cellAt(window, 0, kTranslationColumn) == "Uno.");
    }

    SECTION("discarding replaces it") {
        prompts.nextUnsavedChoice = UnsavedChoice::Discard;

        openTranslation(window, prompts, "manquante.srt");

        CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
        CHECK(cellAt(window, 1, kTranslationColumn).empty());
    }

    SECTION("saving writes it to its own file first") {
        prompts.nextUnsavedChoice = UnsavedChoice::Save;

        openTranslation(window, prompts, "manquante.srt");

        CHECK(files.contentOf("film.en.srt").value_or("").find("Uno.") != std::string::npos);
        CHECK(cellAt(window, 1, kTranslationColumn).empty());
    }
}

TEST_CASE("the translation saves to its own file and leaves the main one alone",
          "[gui][GUI-TRANS-06]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));

    window.saveTranslationAction()->trigger();

    CHECK(files.contentOf("film.en.srt").value_or("").find("Uno.") != std::string::npos);
    CHECK(files.contentOf("film.srt").value_or("") == kMain);
}

TEST_CASE("saving the main document leaves the translation file alone", "[gui][GUI-TRANS-06]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));

    window.saveAction()->trigger();

    CHECK(files.contentOf("film.srt").value_or("").find("Un bis.") != std::string::npos);
    CHECK(files.contentOf("film.en.srt").value_or("") == kFull);
}

TEST_CASE("each document has its own modified state, and the title follows either",
          "[gui][GUI-TRANS-06]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE_FALSE(window.isWindowModified());

    // The translation alone is modified: the title says so.
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    CHECK(window.isWindowModified());
    window.saveAction()->trigger();
    CHECK(window.isWindowModified());
    window.saveTranslationAction()->trigger();
    CHECK_FALSE(window.isWindowModified());

    // The main text alone.
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    CHECK(window.isWindowModified());
    window.saveTranslationAction()->trigger();
    CHECK(window.isWindowModified());
    window.saveAction()->trigger();
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("the entries that save a translation are out without one", "[gui][GUI-TRANS-06]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};

    CHECK_FALSE(window.saveTranslationAction()->isEnabled());
    CHECK_FALSE(window.saveTranslationAsAction()->isEnabled());

    openTranslation(window, prompts, "film.en.srt");

    CHECK(window.saveTranslationAction()->isEnabled());
    CHECK(window.saveTranslationAsAction()->isEnabled());
}

TEST_CASE("the translation is saved as another file, in the shape it is asked in",
          "[gui][GUI-TRANS-06]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    prompts.nextSaveTarget =
        SaveTarget{.path = "film.fr.vtt", .format = subedit::core::SubtitleFormat::WebVtt};

    window.saveTranslationAsAction()->trigger();

    CHECK(files.contentOf("film.fr.vtt").value_or("").starts_with("WEBVTT"));
    CHECK(files.contentOf("film.fr.vtt").value_or("").find("Two.") != std::string::npos);
    // The chooser was shown the file of the translation, not the main one.
    CHECK(prompts.lastCurrent.path == std::filesystem::path{"film.en.srt"});
    // Its next `Save` writes there.
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    window.saveTranslationAction()->trigger();
    CHECK(files.contentOf("film.fr.vtt").value_or("").find("Uno.") != std::string::npos);
    // And the main document is where it was.
    CHECK(files.contentOf("film.srt").value_or("") == kMain);
}

TEST_CASE("saving the translation under a format that loses something asks first",
          "[gui][GUI-TRANS-06]") {
    // LRC writes no end and cannot join two lines' worth of tags: the box is the
    // one of `Save As…`, and a refusal writes nothing.
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    prompts.nextSaveTarget =
        SaveTarget{.path = "film.en.lrc", .format = subedit::core::SubtitleFormat::Lrc};
    prompts.nextLossAccepted = false;

    window.saveTranslationAsAction()->trigger();

    REQUIRE(prompts.losses.size() == 1);
    CHECK_FALSE(files.contentOf("film.en.lrc").has_value());
}

TEST_CASE("the translation is not asked about the tags of the main text", "[gui][GUI-TRANS-06]") {
    // The main text is in italics, LRC has none; the translation holds no tag,
    // so writing it as LRC loses no tag — only the ends.
    InMemoryFileSystem files = filesystem();
    files.addFile("film.srt",
                  "1\n00:00:01,000 --> 00:00:02,000\n<i>Un.</i>\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                  "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n");
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    prompts.nextSaveTarget =
        SaveTarget{.path = "film.en.lrc", .format = subedit::core::SubtitleFormat::Lrc};

    window.saveTranslationAsAction()->trigger();

    REQUIRE(prompts.losses.size() == 1);
    CHECK(prompts.losses.front().find("tag") == std::string::npos);
}

TEST_CASE("a translation whose file cannot be written stays modified", "[gui][GUI-TRANS-06]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));

    files.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);
    window.saveTranslationAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front().starts_with("film.en.srt: "));
    CHECK(window.isWindowModified());
}

TEST_CASE("closing with only the main document modified asks the plain question",
          "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    const int runsBefore = prompts.runAsked;
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));

    CHECK_FALSE(window.close());

    CHECK(prompts.unsavedAsked == 1);
    CHECK(prompts.lastUnsavedDocument == Document::Main);
    CHECK(prompts.runAsked == runsBefore);
}

TEST_CASE("closing with only the translation modified asks the plain question about it",
          "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));

    CHECK_FALSE(window.close());

    CHECK(prompts.unsavedAsked == 1);
    CHECK(prompts.lastUnsavedDocument == Document::Translation);
}

TEST_CASE("closing with both documents modified asks one question, with one box each",
          "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    const int runsBefore = prompts.runAsked;
    std::size_t boxes = 0;
    prompts.nextRun = false;
    prompts.fill = [&boxes](QDialog& dialog) {
        if (const auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog))
            boxes = static_cast<std::size_t>(list->boxes().size());
    };

    CHECK_FALSE(window.close());

    CHECK(prompts.runAsked == runsBefore + 1);
    CHECK(prompts.unsavedAsked == 0);
    CHECK(boxes == 2);
}

TEST_CASE("saving the ticked documents writes those and only those", "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        if (auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog)) {
            list->boxes().at(0)->setChecked(false);
            list->saveButton()->click();
        }
    };

    CHECK(window.close());

    CHECK(files.contentOf("film.en.srt").value_or("").find("Uno.") != std::string::npos);
    CHECK(files.contentOf("film.srt").value_or("") == kMain);
}

TEST_CASE("closing without saving loses both and closes", "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        if (auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog))
            list->discardButton()->click();
    };

    CHECK(window.close());

    CHECK(files.contentOf("film.srt").value_or("") == kMain);
    CHECK(files.contentOf("film.en.srt").value_or("") == kFull);
}

TEST_CASE("cancelling the question closes nothing and writes nothing", "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    prompts.nextRun = false;
    prompts.fill = {};

    CHECK_FALSE(window.close());

    CHECK(files.contentOf("film.srt").value_or("") == kMain);
    CHECK(files.contentOf("film.en.srt").value_or("") == kFull);
}

TEST_CASE("a save that fails stops the closing", "[gui][GUI-CLOSE-01]") {
    // Saving what was ticked is part of the answer, and a document that could
    // not be written is one whose changes would be lost by closing.
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    prompts.nextRun = true;
    prompts.fill = [&files](QDialog& dialog) {
        if (auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog)) {
            files.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);
            list->saveButton()->click();
        }
    };

    CHECK_FALSE(window.close());

    CHECK_FALSE(prompts.failures.empty());
}

TEST_CASE("a main file gone from the disk counts as modified, and the list says so",
          "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    REQUIRE(files.remove("film.srt").has_value());
    std::vector<std::string> labels;
    prompts.nextRun = false;
    prompts.fill = [&labels](QDialog& dialog) {
        if (const auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog)) {
            for (const QCheckBox* box : list->boxes())
                labels.push_back(box->text().toStdString());
        }
    };

    CHECK_FALSE(window.close());

    REQUIRE(labels.size() == 2);
    CHECK(labels.at(0).find("gone from the disk") != std::string::npos);
    CHECK(labels.at(1).find("gone from the disk") == std::string::npos);
}

// **Opening no longer touches what was there.** It used to replace the main
// document and the translation with it, which is why opening once asked the
// same two-document question as closing; since #437 opening lands on a tab of
// its own, and `window_tabs_test.cpp` is where that tab's independence from
// this one is proved. `GUI-CLOSE-01`'s two-document question keeps its other
// cases here, all of them against a real close.
TEST_CASE("a box accepted without an answer closes nothing", "[gui][GUI-CLOSE-01]") {
    // The box said « accepted » and no button was pressed: what was not chosen
    // is not consent to lose the changes.
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    openTranslation(window, prompts, "film.en.srt");
    REQUIRE(edit(window, 0, kTextColumn, "Un bis."));
    REQUIRE(edit(window, 0, kTranslationColumn, "Uno."));
    prompts.nextRun = true;
    prompts.fill = {};

    CHECK_FALSE(window.close());

    CHECK(files.contentOf("film.srt").value_or("") == kMain);
}

TEST_CASE("what the reading of the translation ran into goes to the panel", "[gui][GUI-TRANS-02]") {
    // A block with no number is what the reading reports and recovers from: the
    // file is still opened, and the panel says what it met.
    InMemoryFileSystem files = filesystem();
    files.addFile("sans-numero.srt",
                  "00:00:01,000 --> 00:00:02,000\nOne.\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\nTwo.\n\n");
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    REQUIRE(window.diagnostics()->count() == 0);

    openTranslation(window, prompts, "sans-numero.srt");

    CHECK(window.diagnostics()->count() > 0);
    CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
}
