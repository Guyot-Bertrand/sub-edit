// The search follows the document aimed at — issue #433, decision D8.
//
// **The document is the one of the current column**, the rule every text
// operation of the window follows since #431, and there is no setting for it.
// The search itself is proved in the core, both texts and both dialects; what
// is under test here is what the window adds: the column read at each gesture,
// the box that names its field, and a match forgotten when the column changes.
//
// **What stays intact is checked as much as what changes**, as in the other
// translation cases: a replacement that reached both texts would pass a check on
// the aimed one, and it is the other that a user would lose.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/search_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::SearchDialog;
using subedit::test::FakePrompts;

constexpr int kTextColumn = 4;
constexpr int kTranslationColumn = 5;

/// « Marie » twice in the main text, once as it stands in the translation of
/// the first subtitle — the same word at the same place, which is what lets a
/// match found in one text be mistaken for a match in the other.
constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nRien.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nMarie et Marie.\n\n";

[[nodiscard]] InMemoryFileSystem withThree() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    return files;
}

[[nodiscard]] OpenedFile plain(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// The three subtitles above, each with a translation.
[[nodiscard]] OpenedFile translated(const InMemoryFileSystem& files) {
    OpenedFile opened = plain(files);
    std::vector<Subtitle> subtitles{opened.project.subtitles().begin(),
                                    opened.project.subtitles().end()};
    constexpr std::array<const char*, 3> kTranslations = {
        "Bonjour Marie.", "Nothing.", "Mary and Mary."};
    for (std::size_t row = 0; row < subtitles.size() && row < kTranslations.size(); ++row)
        subtitles[row].translationText = kTranslations[row];
    opened.project.setSubtitles(std::move(subtitles));
    opened.project.setSourceFile(Document::Translation,
                                 SourceFile{.format = SubtitleFormat::SubRip});
    return opened;
}

[[nodiscard]] std::string cellAt(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// Puts the current cell there **without selecting the row**, so that the
/// search aims at the whole file.
void currentAt(const MainWindow& window, int row, int column) {
    window.table()->selectionModel()->setCurrentIndex(window.table()->model()->index(row, column),
                                                      QItemSelectionModel::NoUpdate);
}

[[nodiscard]] std::vector<int> selectedRows(const MainWindow& window) {
    std::vector<int> rows;
    for (const QModelIndex& index : window.table()->selectionModel()->selectedRows())
        rows.push_back(index.row());
    return rows;
}

/// Opens the dialog and types `pattern` into it.
[[nodiscard]] SearchDialog& searching(MainWindow& window, const char* pattern) {
    window.findAndReplaceAction()->trigger();
    SearchDialog* dialog = window.searchDialog();
    REQUIRE(dialog != nullptr);
    dialog->patternField()->setText(QString::fromUtf8(pattern));
    return *dialog;
}

[[nodiscard]] std::string statusOf(const SearchDialog& dialog) {
    return dialog.statusLabel()->text().toStdString();
}

} // namespace

TEST_CASE("finding in the translation column looks in the translation, not the main text",
          "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);
    const SearchDialog& dialog = searching(window, "Mary");

    dialog.nextButton()->click();

    // « Mary » is in the translation of the third subtitle and nowhere in the
    // main text, which is what tells the two apart.
    CHECK(selectedRows(window) == std::vector<int>{2});
    CHECK(statusOf(dialog).empty());
}

TEST_CASE("finding in the main column does not look in the translation", "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTextColumn);
    const SearchDialog& dialog = searching(window, "Mary");

    dialog.nextButton()->click();

    CHECK(selectedRows(window).empty());
    CHECK(statusOf(dialog) == "\"Mary\" not found");
}

// The search moves the table to each match, and it went to the first column
// while it did: the next gesture then read a current cell that was no longer in
// the translation, and a search begun there went on in the main text.
TEST_CASE("moving to a match keeps the column of the current cell", "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);
    const SearchDialog& dialog = searching(window, "Mary");

    dialog.nextButton()->click();
    REQUIRE(selectedRows(window) == std::vector<int>{2});

    CHECK(window.table()->currentIndex().column() == kTranslationColumn);
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{2});
    CHECK(statusOf(dialog).empty());
}

TEST_CASE("replacing all in the translation leaves the main text, and undo gives it back",
          "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);
    const SearchDialog& dialog = searching(window, "Mary");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    CHECK(statusOf(dialog) == "replaced 2 matches");
    CHECK(cellAt(window, 2, kTranslationColumn) == "Sophie and Sophie.");
    CHECK(cellAt(window, 0, kTextColumn) == "Bonjour Marie.");
    CHECK(cellAt(window, 2, kTextColumn) == "Marie et Marie.");

    window.undoAction()->trigger();
    CHECK(cellAt(window, 2, kTranslationColumn) == "Mary and Mary.");
    CHECK(cellAt(window, 2, kTextColumn) == "Marie et Marie.");
}

TEST_CASE("replacing one match in the translation writes that text and finds the next",
          "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);
    const SearchDialog& dialog = searching(window, "Mary");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));
    dialog.nextButton()->click();

    dialog.replaceButton()->click();

    CHECK(cellAt(window, 2, kTranslationColumn) == "Sophie and Mary.");
    CHECK(cellAt(window, 2, kTextColumn) == "Marie et Marie.");
}

TEST_CASE("replacing all in the main column leaves the translation", "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTextColumn);
    const SearchDialog& dialog = searching(window, "Marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    CHECK(cellAt(window, 0, kTextColumn) == "Bonjour Sophie.");
    CHECK(cellAt(window, 2, kTextColumn) == "Sophie et Sophie.");
    CHECK(cellAt(window, 0, kTranslationColumn) == "Bonjour Marie.");
}

// **A match belongs to the document it was found in.** The translation of the
// first subtitle reads exactly what the main text does there, so a match kept
// across the change of column would be replaced without the user having seen it
// found in that text.
TEST_CASE("a match found in one column is not replaced from the other", "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTextColumn);
    const SearchDialog& dialog = searching(window, "Marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));
    dialog.nextButton()->click();
    REQUIRE(selectedRows(window) == std::vector<int>{0});

    currentAt(window, 0, kTranslationColumn);
    dialog.replaceButton()->click();

    // The press found the first match of the translation, as it does when no
    // match is known, and replaced nothing.
    CHECK(cellAt(window, 0, kTranslationColumn) == "Bonjour Marie.");
    CHECK(cellAt(window, 0, kTextColumn) == "Bonjour Marie.");
    CHECK(selectedRows(window) == std::vector<int>{0});
}

TEST_CASE("the box says which text it looks in, and changes with the column",
          "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTextColumn);

    const SearchDialog& dialog = searching(window, "Marie");
    CHECK_FALSE(dialog.fieldLabel()->isHidden());
    CHECK(dialog.fieldLabel()->text() == QStringLiteral("Searching in: Main"));

    currentAt(window, 0, kTranslationColumn);
    CHECK(dialog.fieldLabel()->text() == QStringLiteral("Searching in: Translation"));

    currentAt(window, 1, kTextColumn);
    CHECK(dialog.fieldLabel()->text() == QStringLiteral("Searching in: Main"));
}

TEST_CASE("a box opened in the translation column says so at once", "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);

    const SearchDialog& dialog = searching(window, "Marie");

    CHECK(dialog.fieldLabel()->text() == QStringLiteral("Searching in: Translation"));
}

TEST_CASE("the box says nothing when there is one text only", "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, plain(files), prompts};
    window.show();

    const SearchDialog& dialog = searching(window, "Marie");

    CHECK(dialog.fieldLabel()->isHidden());
}

TEST_CASE("taking the translation column away takes the field from the box",
          "[gui][GUI-SEARCH-03]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "Marie");
    REQUIRE_FALSE(dialog.fieldLabel()->isHidden());

    window.translationColumnAction()->trigger();

    CHECK(dialog.fieldLabel()->isHidden());
}
