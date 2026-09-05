// Open, save, save as, and what protects work that was never written —
// issue #131.
//
// Everything goes through a fake `Prompts`: what is tested is not Qt's dialog
// but what the window makes of the answer, including when the answer is
// « no ».

#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/prompts.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QString>
#include <QTableView>
#include <QToolButton>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <expected>
#include <optional>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::EncodingRefusal;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::Newline;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::SaveTarget;
using subedit::gui::UnsavedChoice;

/// L'encodage de ce nom, ou un test en échec.
[[nodiscard]] Encoding named(const char* name) {
    const std::expected<Encoding, EncodingRefusal> encoding =
        Encoding::create(name, ByteOrderMark::Absent);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas cet encodage");
        return Encoding::utf8(ByteOrderMark::Absent);
    }
    return *encoding;
}

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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
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
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();

    window.saveAction()->trigger();

    CHECK(prompts.saveTargetAsked == 1);
    CHECK(files.contentOf("neuf.srt").has_value());
}

TEST_CASE("saving as writes in the encoding and the endings chosen", "[gui][GUI-ENC-02]") {
    // Latin-1 et CRLF, demandés dans la boîte : ce que la fenêtre en fait est un
    // fichier écrit ainsi, et un document qui porte désormais cette forme.
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9.\n\n");
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "copie.srt",
                                        .format = SubtitleFormat::SubRip,
                                        .encoding = named("iso-8859-1"),
                                        .newline = Newline::CrLf};
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    const std::string written = files.contentOf("copie.srt").value_or("");
    CHECK(written.find("\r\n") != std::string::npos);
    // Latin-1 : la lettre accentuée tient sur un octet, et non sur deux.
    CHECK(written.find("caf\xE9") != std::string::npos);

    // Le document porte cette forme désormais : un `Save` qui ne demande rien
    // réécrit le même fichier, pas celui qu'il était avant.
    window.saveAction()->trigger();

    CHECK(files.contentOf("copie.srt").value_or("") == written);
}

TEST_CASE("saving as opens on the encoding the file was read in", "[gui][GUI-ENC-02]") {
    // Ce que le fichier porte, et non ce qu'un réglage se rappelle : l'écrire
    // autrement sans qu'on l'ait demandé perdrait ce que la lecture a gardé.
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9.\n\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK(prompts.lastProposedEncoding == named("iso-8859-1"));
}

TEST_CASE("a document that came from no file opens on the encoding last chosen",
          "[gui][GUI-ENC-03]") {
    // Le seul cas où le réglage sert : un document neuf n'a pas d'encodage à
    // lui, et ce que l'utilisateur écrit d'habitude est la meilleure réponse.
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();

    subedit::core::Settings settings;
    settings.writeEncoding = named("windows-1250");
    window.applySettings(settings);

    window.saveAsAction()->trigger();

    CHECK(prompts.lastProposedEncoding == settings.writeEncoding);
}

TEST_CASE("the encoding chosen is what the settings carry away", "[gui][GUI-ENC-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{
        .path = "copie.srt", .format = SubtitleFormat::SubRip, .encoding = named("windows-1250")};
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK(window.settings().writeEncoding == named("windows-1250"));
}

TEST_CASE("the status bar says the encoding, mark included", "[gui][GUI-ENC-01]") {
    // **Issue #313.** The window said the encoding in a diagnostic and nowhere
    // else, so it said it only when the encoding had been *guessed*: a file
    // that declares its own with a byte order mark showed nothing at all, where
    // `inspect` writes « UTF-16LE, from its byte order mark ». The command line
    // had three answers, the window one and a half.
    //
    // A Latin-1 file here — guessed, and worth a diagnostic — and a UTF-8 file
    // with a mark below, which declares itself and is worth none. Both now say
    // what they are.
    InMemoryFileSystem files;
    files.addFile("latin.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9.\n\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "latin.srt"), prompts};
    window.show();

    CHECK(window.encodingStatus()->text().toStdString() == "Encoding: ISO-8859-1, no BOM");
}

TEST_CASE("a file that declares its encoding says so too", "[gui][GUI-ENC-01]") {
    // The case the diagnostic could never carry: nothing was guessed, so
    // nothing was said. The status bar says what the document *is* rather than
    // what its reading did, which is why it answers here as well.
    InMemoryFileSystem files;
    files.addFile("marque.srt",
                  std::string{"\xEF\xBB\xBF"} + "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "marque.srt"), prompts};
    window.show();

    CHECK(window.encodingStatus()->text().toStdString() == "Encoding: UTF-8, BOM");
}

TEST_CASE("saving under another name moves the encoding shown with it", "[gui][GUI-ENC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "copie.srt",
                                        .format = SubtitleFormat::SubRip,
                                        .encoding = named("windows-1250"),
                                        .newline = Newline::Lf};
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
    REQUIRE(window.encodingStatus()->text().toStdString() == "Encoding: UTF-8, no BOM");

    window.saveAsAction()->trigger();

    CHECK(window.encodingStatus()->text().toStdString() == "Encoding: windows-1250, no BOM");
}

TEST_CASE("a save under another name that fails moves nothing", "[gui][GUI-SAVE-02]") {
    // **The defect of issue #309.** The target shape is put in place before the
    // writing — `saveProject` writes what the project carries — and it was not
    // undone when the writing failed: the document then aimed at a file that
    // does not exist, and `Save` wrote somewhere other than where anyone
    // thought.
    //
    // The `ł` and Latin-1 are the shortest road to that failure, and it asks
    // nothing of the disk: no Latin encoding can write that letter.
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nŁódź.\n\n");
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "copie.srt",
                                        .format = SubtitleFormat::SubRip,
                                        .encoding = named("iso-8859-1"),
                                        .newline = Newline::Lf};
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    // Nothing was written, and that is what the message says.
    CHECK_FALSE(files.contentOf("copie.srt").has_value());
    REQUIRE_FALSE(prompts.failures.empty());
    CHECK_THAT(prompts.failures.back(),
               ContainsSubstring("holds a character the chosen encoding cannot write"));

    // **And the document has not moved.** What proves it is the next gesture:
    // `Save` writes the open file back, in UTF-8, where the `ł` has its place.
    // With the defect it aimed at `copie.srt` in Latin-1 and failed a second
    // time — hence the count of failures rather than the absence of the file,
    // which did not tell the two apart.
    REQUIRE(prompts.failures.size() == 1);

    window.saveAction()->trigger();

    CHECK(prompts.failures.size() == 1);
    CHECK_FALSE(files.contentOf("copie.srt").has_value());
    CHECK_THAT(files.contentOf("film.srt").value_or(""), ContainsSubstring("Łódź"));
}

TEST_CASE("saving under another name moves the document there", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "film.vtt", .format = SubtitleFormat::WebVtt};
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK(prompts.saveTargetAsked == 1);
    CHECK(files.fileCount() == 1);
    CHECK(prompts.lastCurrent.format == SubtitleFormat::SubRip);
}

TEST_CASE("a save that cannot be written says so and keeps the changes", "[gui][GUI-SAVE-01]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    // **The cause, and not one sentence for all of them** — issue #154. The
    // window used to say « holds nothing recognisable as a subtitle » for an
    // absent file as readily as for this one, and had less to go on than the
    // terminal at the very point where a user needs it most.
    CHECK(prompts.failures.front() == "notes.txt: is in no format this tool knows");
    CHECK(window.table()->model()->rowCount({}) == 2);
    CHECK(textAt(window, 0) == "Un.");
}

TEST_CASE("opening a file that is not there says so, and not something else",
          "[gui][GUI-OPEN-02]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    prompts.nextFileToOpen = "absent.srt";
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front() == "absent.srt: does not exist");
    CHECK(window.table()->model()->rowCount({}) == 2);
}

TEST_CASE("opening with unsaved changes asks, and cancelling opens nothing", "[gui][GUI-SAVE-03]") {
    InMemoryFileSystem files = withFile("film.srt", kThree);
    files.addFile("autre.srt", "1\n00:00:09,000 --> 00:00:10,000\nAilleurs.\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
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
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();
    prompts.nextFileToOpen = "bancal.srt";

    window.openAction()->trigger();

    REQUIRE(window.diagnostics() != nullptr);
    CHECK(window.diagnostics()->count() == 1);
    // The line of the file, which only the reading knows.
    CHECK(window.diagnostics()->lineAt(0).toStdString().starts_with("line 5:"));
}

TEST_CASE("the encoding a reading had to guess is shown, without a line", "[gui][GUI-ENC-01]") {
    // The bytes were weighed before a single line existed, so there is no place
    // to name — and a panel that said "line 0" would name one that is not
    // there. Latin-1: a lone 0xE9 where UTF-8 would need a second byte.
    InMemoryFileSystem files =
        withFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9 pr\xE8s du port.\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    REQUIRE(window.diagnostics()->count() == 1);
    const std::string line = window.diagnostics()->lineAt(0).toStdString();
    CHECK_FALSE(line.starts_with("line "));
    CHECK(line.starts_with("an encoding nothing declared"));
}

TEST_CASE("a reading with nothing to report shows no panel", "[gui][GUI-OPEN-03]") {
    // An empty panel would say there is something to read.
    InMemoryFileSystem files = withFile("film.srt", kThree);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

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
    MainWindow window{files, fileIn(files, "bancal.srt"), prompts};
    window.show();

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
    MainWindow window{files, fileIn(files, "bancal.srt"), prompts};
    window.show();

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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();
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
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.openAction()->trigger();

    CHECK(prompts.openAsked == 1);
    CHECK(prompts.failures.empty());
    CHECK(textAt(window, 0) == "Un.");
}
