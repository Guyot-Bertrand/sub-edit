// The one button that writes a tag — issue #365.
//
// **What it buys is the vocabulary a user no longer has to know.** The text of
// a subtitle is the text of its file, tags included: an `.ass` says `{\i1}` and
// a `.sub` says `{Y:i}`, and before this entry existed there was nowhere to
// learn it. The tests below check the two things that follow from that — the
// right tag for the document open, and an entry that goes out where a format
// carries no style at all.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr const char* kSubRip = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                                "2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n";

/// Advanced SSA, whose events carry the fields its `Format:` line names.
constexpr const char* kAdvancedSsa =
    "[Script Info]\nScriptType: v4.00+\n\n"
    "[V4+ Styles]\n"
    "Format: Name, Fontname\nStyle: Default,Arial\n\n"
    "[Events]\n"
    "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
    "Dialogue: 0,0:00:01.00,0:00:02.00,Default,,0,0,0,,Bonjour.\n";

/// A blank row among two written ones, as an insertion leaves one.
constexpr const char* kWithBlank = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                                   "2\n00:00:03,000 --> 00:00:04,000\n\n"
                                   "3\n00:00:05,000 --> 00:00:06,000\nAu revoir.\n\n";

/// LRC, which has no way of saying anything about a style.
constexpr const char* kLrc = "[00:01.00]Bonjour.\n[00:03.00]Au revoir.\n";

[[nodiscard]] InMemoryFileSystem withFile(const char* name, const char* content) {
    InMemoryFileSystem files;
    files.addFile(name, content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* name) {
    auto opened = openProject(files, name);
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

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("the entry writes the italic tag of the document open", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == "<i>Bonjour.</i>");
    CHECK(textAt(window, 1) == "<i>Au revoir.</i>");
}

TEST_CASE("the same entry writes braces on an Advanced SSA", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.ass", kAdvancedSsa);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.ass"), prompts};
    window.show();

    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == R"({\i1}Bonjour.{\i0})");
}

TEST_CASE("pressing it twice leaves the text as it was found", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();
    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "Au revoir.");
}

TEST_CASE("only what is selected is touched", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    selectRow(window, 1);
    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "<i>Au revoir.</i>");
}

TEST_CASE("the operation enters the history and comes back out", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();
    REQUIRE(window.undoAction()->isEnabled());
    CHECK(window.undoAction()->text().toStdString() == "Undo: putting in italics");

    window.undoAction()->trigger();
    CHECK(textAt(window, 0) == "Bonjour.");
}

TEST_CASE("the entry says how many subtitles it moved", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();
    CHECK(prompts.outcomes.back() == "2 subtitles put in italics");

    window.italicAction()->trigger();
    CHECK(prompts.outcomes.back() == "2 subtitles taken out of italics");
}

TEST_CASE("a blank row gains no tags, and its selection changes nothing", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kWithBlank);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    // The whole file: the blank row is carried along and stays blank, where an
    // `<i></i>` around nothing would be visible in the table.
    window.italicAction()->trigger();
    CHECK(textAt(window, 0) == "<i>Bonjour.</i>");
    CHECK(textAt(window, 1).empty());
    CHECK(textAt(window, 2) == "<i>Au revoir.</i>");

    window.table()->selectionModel()->clearSelection();
    selectRow(window, 1);

    // Nothing but blank rows: no operation, and nothing added to the history.
    const bool undoable = window.undoAction()->isEnabled();
    window.italicAction()->trigger();
    CHECK(prompts.outcomes.back() == "nothing to change");
    CHECK(window.undoAction()->isEnabled() == undoable);
    CHECK(window.undoAction()->text().toStdString() == "Undo: putting in italics");
}

TEST_CASE("a format that carries no style leaves the entry out", "[gui][GUI-ITALIC-02]") {
    InMemoryFileSystem files = withFile("film.lrc", kLrc);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.lrc"), prompts};
    window.show();

    // Out and not gone: what a user of an LRC has to learn is that there is
    // nothing to type, and an entry that disappeared would teach nothing.
    CHECK_FALSE(window.italicAction()->isEnabled());
    CHECK(window.menuTitles().contains(QStringLiteral("&Tools")));
}
