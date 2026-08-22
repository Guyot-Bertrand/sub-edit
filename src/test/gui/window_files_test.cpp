// Open, save, save as, and what protects work that was never written —
// issue #131.
//
// Everything goes through a fake `Prompts`: what is tested is not Qt's dialog
// but what the window makes of the answer, including when the answer is
// « no ».

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/prompts.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QListWidget>
#include <QString>
#include <QTableView>
#include <QToolButton>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
using subedit::gui::SaveTarget;
using subedit::gui::UnsavedChoice;
using subedit::test::FakePrompts;

constexpr const char* kThree = "1\n"
                               "00:00:01,000 --> 00:00:02,000\n"
                               "Un.\n"
                               "\n"
                               "2\n"
                               "00:00:03,000 --> 00:00:04,000\n"
                               "Deux.\n"
                               "\n";

/// A file whose reading runs into something: the number of the second block is
/// missing, and the reader recovers.
constexpr const char* kNumberless = "1\n"
                                    "00:00:01,000 --> 00:00:02,000\n"
                                    "Un.\n"
                                    "\n"
                                    "00:00:03,000 --> 00:00:04,000\n"
                                    "Deux.\n"
                                    "\n";

[[nodiscard]] InMemoryFileSystem withFile(const std::string& path, const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(path, content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, 4), Qt::DisplayRole)
        .toString()
        .toStdString();
}

[[nodiscard]] bool edit(const MainWindow& window, int row, const char* typed) {
    return window.table()->model()->setData(
        window.table()->model()->index(row, 4), QString::fromUtf8(typed), Qt::EditRole);
}

} // namespace

TEST_CASE("saving writes the file the window was opened on", "[gui][GUI-SAVE-01]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};
    REQUIRE(edit(window, 0, "Un bis."));

    window.saveAction()->trigger();

    CHECK(files.contentOf("film.srt").value_or("").find("Un bis.") != std::string::npos);
    CHECK_FALSE(window.isWindowModified());
    // No question asked: the destination is known.
    CHECK(prompts.saveTargetAsked == 0);
}

TEST_CASE("saving a document that came from nowhere asks where", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "neuf.srt", .format = SubtitleFormat::SubRip};
    const MainWindow window{files, OpenedFile{}, prompts};

    window.saveAction()->trigger();

    CHECK(prompts.saveTargetAsked == 1);
    CHECK(files.contentOf("neuf.srt").has_value());
}

TEST_CASE("saving under another name moves the document there", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "film.vtt", .format = SubtitleFormat::WebVtt};
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};

    window.saveAsAction()->trigger();

    CHECK(files.contentOf("film.vtt").value_or("").starts_with("WEBVTT"));
    CHECK(window.windowTitle().toStdString().find("film.vtt") != std::string::npos);
    // The table's separator follows the format: the window writes WebVTT from
    // now on, so it shows periods.
    CHECK(window.table()
              ->model()
              ->data(window.table()->model()->index(0, 1), Qt::DisplayRole)
              .toString()
              .toStdString() == "00:00:01.000");
}

TEST_CASE("saving under another name asks first, and gives up if told to", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts; // nextSaveTarget vide : l'utilisateur a annulé
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};

    window.saveAsAction()->trigger();

    CHECK(prompts.saveTargetAsked == 1);
    CHECK(files.fileCount() == 1);
    CHECK(prompts.lastCurrent.format == SubtitleFormat::SubRip);
}

TEST_CASE("a save that cannot be written says so and keeps the changes", "[gui][GUI-SAVE-01]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};
    REQUIRE(edit(window, 0, "Un bis."));
    files.failNextWrite(FileErrorKind::PermissionDenied);

    window.saveAction()->trigger();

    CHECK(prompts.failures.size() == 1);
    // The work is not lost, and the mark still says so.
    CHECK(window.isWindowModified());
}

TEST_CASE("opening replaces what the window holds", "[gui][GUI-OPEN-01]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    files.addFile("autre.srt", "1\n00:00:09,000 --> 00:00:10,000\nAilleurs.\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};

    window.openAction()->trigger();

    CHECK(window.table()->model()->rowCount({}) == 1);
    CHECK(textAt(window, 0) == "Ailleurs.");
    CHECK(window.windowTitle().toStdString().find("autre.srt") != std::string::npos);
}

TEST_CASE("opening what cannot be read says so and leaves the window as it was",
          "[gui][GUI-OPEN-02]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    files.addFile("notes.txt", "rien de reconnaissable\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "notes.txt";
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};

    window.openAction()->trigger();

    CHECK(prompts.failures.size() == 1);
    CHECK(window.table()->model()->rowCount({}) == 2);
    CHECK(textAt(window, 0) == "Un.");
}

TEST_CASE("opening with unsaved changes asks, and cancelling opens nothing", "[gui][GUI-SAVE-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    files.addFile("autre.srt", "1\n00:00:09,000 --> 00:00:10,000\nAilleurs.\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};
    REQUIRE(edit(window, 0, "Un bis."));

    window.openAction()->trigger();

    CHECK(prompts.unsavedAsked == 1);
    // Neither opened, nor even asked which file: the question stops before
    // that.
    CHECK(prompts.openAsked == 0);
    CHECK(textAt(window, 0) == "Un bis.");
}

TEST_CASE("discarding unsaved changes opens the other file anyway", "[gui][GUI-SAVE-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    files.addFile("autre.srt", "1\n00:00:09,000 --> 00:00:10,000\nAilleurs.\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    prompts.nextUnsavedChoice = UnsavedChoice::Discard;
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};
    REQUIRE(edit(window, 0, "Un bis."));

    window.openAction()->trigger();

    CHECK(textAt(window, 0) == "Ailleurs.");
    // Discarded means discarded: the original file is untouched.
    CHECK(files.contentOf("film.srt").value_or("") == kThree);
}

TEST_CASE("choosing to save before opening writes, then opens", "[gui][GUI-SAVE-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    files.addFile("autre.srt", "1\n00:00:09,000 --> 00:00:10,000\nAilleurs.\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    prompts.nextUnsavedChoice = UnsavedChoice::Save;
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};
    REQUIRE(edit(window, 0, "Un bis."));

    window.openAction()->trigger();

    CHECK(files.contentOf("film.srt").value_or("").find("Un bis.") != std::string::npos);
    CHECK(textAt(window, 0) == "Ailleurs.");
}

TEST_CASE("a window with nothing unsaved closes without a question", "[gui][GUI-SAVE-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};

    CHECK(window.close());
    CHECK(prompts.unsavedAsked == 0);
}

TEST_CASE("closing with unsaved changes asks, and cancelling keeps the window",
          "[gui][GUI-SAVE-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    REQUIRE(edit(window, 0, "Un bis."));

    CHECK_FALSE(window.close());
    CHECK(prompts.unsavedAsked == 1);
}

TEST_CASE("the diagnostics of a reading are shown", "[gui][GUI-OPEN-03]") {
    InMemoryFileSystem files = withFile("bancal.srt", kNumberless);
    FakePrompts prompts;
    const MainWindow window{files, OpenedFile{}, prompts};
    prompts.nextFileToOpen = "bancal.srt";

    window.openAction()->trigger();

    REQUIRE(window.diagnostics() != nullptr);
    CHECK(window.diagnostics()->count() == 1);
    // The line of the file, which only the reading knows.
    CHECK(window.diagnostics()->lineAt(0).toStdString().starts_with("line 5:"));
}

TEST_CASE("a reading with nothing to report shows no panel", "[gui][GUI-OPEN-03]") {
    // An empty panel would say there is something to read.
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};

    CHECK(window.diagnostics()->count() == 0);
    CHECK_FALSE(window.diagnostics()->isVisibleTo(&window));
}

TEST_CASE("a diagnostic that quotes the file quotes it, and bounds it", "[gui][GUI-OPEN-03]") {
    // The excerpt comes from the file: quoted, or a line ending in a comma
    // would read as the rest of the sentence; bounded, or one absurd line would
    // push the panel off the screen. Neither is ours to trust.
    //
    // An unreadable timing line, and an outsized one: the reader reports it by
    // quoting it, which is exactly the case to bound.
    const std::string absurd(120, 'z');
    InMemoryFileSystem files = withFile("bancal.srt",
                                        "1\n"
                                        "00:00:01,000 --> 00:00:02,000\n"
                                        "Un.\n"
                                        "\n"
                                        "2\n"
                                        "00:00:03,000 --> " +
                                            absurd +
                                            "\n"
                                            "Deux.\n");
    FakePrompts prompts;
    const MainWindow window{files, fileIn(files, "bancal.srt"), prompts};

    REQUIRE(window.diagnostics()->count() >= 1);
    const std::string line = window.diagnostics()->lineAt(0).toStdString();
    CHECK(line.find('"') != std::string::npos);
    CHECK(line.find("…") != std::string::npos);
    CHECK(line.size() < absurd.size() + 60);
}

TEST_CASE("the diagnostics panel folds and unfolds", "[gui][GUI-OPEN-03]") {
    // Folded to start with: what a reading recovered from deserves to be
    // available, not to stand between the user and their table.
    InMemoryFileSystem files = withFile("bancal.srt", kNumberless);
    FakePrompts prompts;
    const MainWindow window{files, fileIn(files, "bancal.srt"), prompts};

    auto* toggle = window.diagnostics()->findChild<QToolButton*>();
    REQUIRE(toggle != nullptr);
    auto* lines = window.diagnostics()->findChild<QListWidget*>();
    REQUIRE(lines != nullptr);

    CHECK_FALSE(lines->isVisibleTo(window.diagnostics()));

    toggle->setChecked(true);
    CHECK(lines->isVisibleTo(window.diagnostics()));

    toggle->setChecked(false);
    CHECK_FALSE(lines->isVisibleTo(window.diagnostics()));
}

TEST_CASE("a save-as that cannot be written says so and moves nothing", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "ailleurs.vtt", .format = SubtitleFormat::WebVtt};
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};
    files.failNextWrite(FileErrorKind::PermissionDenied);

    window.saveAsAction()->trigger();

    CHECK(prompts.failures.size() == 1);
    // The document has not moved: the title and what « Save » aims at would be
    // wrong if the failure had let them move.
    CHECK(window.windowTitle().toStdString().find("film.srt") != std::string::npos);
}

TEST_CASE("giving up on the file dialog opens nothing", "[gui][GUI-OPEN-01]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts; // nextFileToOpen vide : l'utilisateur a renoncé
    const MainWindow window{files, fileIn(files, "film.srt"), prompts};

    window.openAction()->trigger();

    CHECK(prompts.openAsked == 1);
    CHECK(prompts.failures.empty());
    CHECK(textAt(window, 0) == "Un.");
}
