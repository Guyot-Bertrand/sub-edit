// The translation column, and the document an operation aims at — issue #431.
//
// **Two texts in one row, and one of them is the target.** The column of the
// current cell says which: the translation when the cell is in its column, the
// main text everywhere else. Every case below opens a project that has a
// translation, since that is the only project the question arises for; the
// plain one is what every other test file already covers.
//
// **What stays intact is checked as much as what changes.** An operation that
// reaches both texts would pass a check on the aimed one, and it is the other
// that a user would lose.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/letter_case.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/shift_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QLabel>
#include <QStringList>
#include <QTableView>
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
using subedit::core::LetterCase;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::SourceFile;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr int kStartColumn = 1;
constexpr int kTextColumn = 4;
constexpr int kTranslationColumn = 5;

constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

/// The three subtitles above, each with a translation, and the translation
/// file's format.
[[nodiscard]] OpenedFile translated(const InMemoryFileSystem& files,
                                    SubtitleFormat translationFormat = SubtitleFormat::SubRip) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());

    std::vector<Subtitle> subtitles{opened->project.subtitles().begin(),
                                    opened->project.subtitles().end()};
    constexpr std::array<const char*, 3> kTranslations = {"One.", "Two.", "Three."};
    for (std::size_t row = 0; row < subtitles.size() && row < kTranslations.size(); ++row)
        subtitles[row].translationText = kTranslations[row];
    opened->project.setSubtitles(std::move(subtitles));
    opened->project.setSourceFile(Document::Translation, SourceFile{.format = translationFormat});
    return std::move(*opened);
}

/// The same file, with no translation at all.
[[nodiscard]] OpenedFile plain(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] InMemoryFileSystem withThree() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    return files;
}

[[nodiscard]] std::string cellAt(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// Puts the current cell there **without selecting the row**: nothing selected
/// is what makes an operation aim at the whole file, and that is what most
/// cases want.
void currentAt(const MainWindow& window, int row, int column) {
    window.table()->selectionModel()->setCurrentIndex(window.table()->model()->index(row, column),
                                                      QItemSelectionModel::NoUpdate);
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("a project with no translation shows no translation column", "[gui][GUI-TRANS-04]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, plain(files), prompts};
    window.show();

    CHECK(window.table()->isColumnHidden(kTranslationColumn));
    CHECK_FALSE(window.translationColumnAction()->isEnabled());
}

TEST_CASE("a project with a translation shows the column", "[gui][GUI-TRANS-04]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();

    CHECK_FALSE(window.table()->isColumnHidden(kTranslationColumn));
    CHECK(window.translationColumnAction()->isEnabled());
    CHECK(window.translationColumnAction()->isChecked());
    CHECK(cellAt(window, 1, kTranslationColumn) == "Two.");
}

TEST_CASE("the entry of the View menu takes the column away and brings it back",
          "[gui][GUI-TRANS-04]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();

    window.translationColumnAction()->trigger();
    CHECK(window.table()->isColumnHidden(kTranslationColumn));

    window.translationColumnAction()->trigger();
    CHECK_FALSE(window.table()->isColumnHidden(kTranslationColumn));
}

TEST_CASE("the menu that holds the entry is called View", "[gui][GUI-TRANS-04]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    const MainWindow window{files, translated(files), prompts};

    CHECK(window.menuTitles().contains(QStringLiteral("&View")));
}

TEST_CASE("opening a file with no translation takes the column away", "[gui][GUI-TRANS-04]") {
    InMemoryFileSystem files = withThree();
    files.addFile("autre.srt", "1\n00:00:09,000 --> 00:00:10,000\nAilleurs.\n\n");
    FakePrompts prompts;
    prompts.nextFileToOpen = "autre.srt";
    MainWindow window{files, translated(files), prompts};
    window.show();
    REQUIRE_FALSE(window.table()->isColumnHidden(kTranslationColumn));

    window.openAction()->trigger();

    CHECK(window.table()->isColumnHidden(kTranslationColumn));
    CHECK_FALSE(window.translationColumnAction()->isEnabled());
}

TEST_CASE("a hidden column stays hidden when the file changes under it", "[gui][GUI-TRANS-04]") {
    // The choice to hide it is the user's, and an operation that refreshes the
    // window must not take it back.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    window.translationColumnAction()->trigger();
    REQUIRE(window.table()->isColumnHidden(kTranslationColumn));

    window.italicAction()->trigger();

    CHECK(window.table()->isColumnHidden(kTranslationColumn));
}

TEST_CASE("the text keeps room to be read beside the translation", "[gui][GUI-TRANS-04]") {
    // The last column shown takes what the others leave, and it is the
    // translation once there is one. The text then has the width it was given,
    // and a default of a hundred pixels would truncate every line of it.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();

    CHECK(window.table()->columnWidth(kTextColumn) >= 300);
}

TEST_CASE("the translation cells are typed in like the text ones", "[gui][GUI-TRANS-04]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    const MainWindow window{files, translated(files), prompts};

    const auto* delegate = window.table()->itemDelegateForColumn(kTranslationColumn);

    CHECK(dynamic_cast<const subedit::gui::TextDelegate*>(delegate) != nullptr);
}

TEST_CASE("without a translation column the italic entry aims at the main text",
          "[gui][GUI-TRANS-05]") {
    // The default, and the one that must not change for anybody who never opens
    // a translation.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, plain(files), prompts};
    window.show();

    window.italicAction()->trigger();

    CHECK(cellAt(window, 0, kTextColumn) == "<i>Un.</i>");
}

TEST_CASE("the italic entry aims at the main text while the current cell is not in the "
          "translation column",
          "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTextColumn);

    window.italicAction()->trigger();

    CHECK(cellAt(window, 0, kTextColumn) == "<i>Un.</i>");
    CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
}

TEST_CASE("the italic entry aims at the translation when the current cell is in its column",
          "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);

    window.italicAction()->trigger();

    CHECK(cellAt(window, 0, kTranslationColumn) == "<i>One.</i>");
    CHECK(cellAt(window, 0, kTextColumn) == "Un.");
}

TEST_CASE("the italic tag written is the one of the document aimed at", "[gui][GUI-TRANS-05]") {
    // The translation is in Advanced SSA, whose italic is a brace: the tags of
    // the main file would be the wrong ones.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files, SubtitleFormat::AdvancedSubStationAlpha), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);

    window.italicAction()->trigger();

    CHECK(cellAt(window, 0, kTranslationColumn) == R"({\i1}One.{\i0})");
}

TEST_CASE("the italic entry follows the format of the document aimed at", "[gui][GUI-TRANS-05]") {
    // LRC writes no style, and a translation in LRC is not one to italicise —
    // while the main text, in SubRip, still is.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files, SubtitleFormat::Lrc), prompts};
    window.show();

    currentAt(window, 0, kTextColumn);
    CHECK(window.italicAction()->isEnabled());

    currentAt(window, 0, kTranslationColumn);
    CHECK_FALSE(window.italicAction()->isEnabled());

    currentAt(window, 0, kTextColumn);
    CHECK(window.italicAction()->isEnabled());
}

TEST_CASE("the case entries aim at the translation, and leave the main text alone",
          "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 1, kTranslationColumn);

    window.caseAction(LetterCase::Upper)->trigger();

    CHECK(cellAt(window, 1, kTranslationColumn) == "TWO.");
    CHECK(cellAt(window, 1, kTextColumn) == "Deux.");
}

TEST_CASE("the case entries aim at the main text from any other column", "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 1, kTextColumn);

    window.caseAction(LetterCase::Upper)->trigger();

    CHECK(cellAt(window, 1, kTextColumn) == "DEUX.");
    CHECK(cellAt(window, 1, kTranslationColumn) == "Two.");
}

TEST_CASE("the dialogue dashes aim at the document of the current column", "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);

    window.dialogueDashesAction()->trigger();

    CHECK(cellAt(window, 0, kTranslationColumn) == "- One.");
    CHECK(cellAt(window, 0, kTextColumn) == "Un.");
}

TEST_CASE("removing the mentions aims at the translation, and leaves the main text alone",
          "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    auto opened = translated(files);
    std::vector<Subtitle> subtitles{opened.project.subtitles().begin(),
                                    opened.project.subtitles().end()};
    subtitles.at(1).translationText = "Two [coughs] again.";
    subtitles.at(1).mainText = "Deux [il tousse] encore.";
    opened.project.setSubtitles(std::move(subtitles));

    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, std::move(opened), prompts};
    window.show();
    currentAt(window, 1, kTranslationColumn);

    window.hearingImpairedAction()->trigger();

    CHECK(cellAt(window, 1, kTranslationColumn) == "Two again.");
    CHECK(cellAt(window, 1, kTextColumn) == "Deux [il tousse] encore.");
}

TEST_CASE("a translation the removal empties is emptied, and the subtitle stays",
          "[gui][GUI-TRANS-05]") {
    // The question #429 raised: on the main text, a subtitle left with nothing
    // is taken away; on the translation it would have taken the main text with
    // it, and that one was not aimed at.
    InMemoryFileSystem files = withThree();
    auto opened = translated(files);
    std::vector<Subtitle> subtitles{opened.project.subtitles().begin(),
                                    opened.project.subtitles().end()};
    subtitles.at(1).translationText = "[coughs]";
    opened.project.setSubtitles(std::move(subtitles));

    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, std::move(opened), prompts};
    window.show();
    currentAt(window, 1, kTranslationColumn);

    window.hearingImpairedAction()->trigger();

    REQUIRE(window.table()->model()->rowCount({}) == 3);
    CHECK(cellAt(window, 1, kTranslationColumn).empty());
    CHECK(cellAt(window, 1, kTextColumn) == "Deux.");
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.at(0) == "1 subtitle cleaned, 0 removed");
}

TEST_CASE("copying a translation and pasting it in the main column writes it there",
          "[gui][GUI-TRANS-05]") {
    // The clipboard carries texts and no document, which is what lets a
    // translation be pasted where the main text is.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    selectRow(window, 0);
    currentAt(window, 0, kTranslationColumn);
    window.copyAction()->trigger();

    window.table()->selectionModel()->clearSelection();
    selectRow(window, 2);
    currentAt(window, 2, kTextColumn);
    window.pasteAction()->trigger();

    CHECK(cellAt(window, 2, kTextColumn) == "One.");
    CHECK(cellAt(window, 2, kTranslationColumn) == "Three.");
}

TEST_CASE("copying a main text and pasting it in the translation column writes it there",
          "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    selectRow(window, 0);
    currentAt(window, 0, kTextColumn);
    window.copyAction()->trigger();

    window.table()->selectionModel()->clearSelection();
    selectRow(window, 2);
    currentAt(window, 2, kTranslationColumn);
    window.pasteAction()->trigger();

    CHECK(cellAt(window, 2, kTranslationColumn) == "Un.");
    CHECK(cellAt(window, 2, kTextColumn) == "Trois.");
}

TEST_CASE("a paste into the translation says what the format of the translation loses",
          "[gui][GUI-TRANS-05]") {
    // The notice names the format the text lands in, and that is the
    // translation's, not the main file's: LRC writes no italic, SubRip does.
    InMemoryFileSystem files = withThree();
    files.addFile("film.srt",
                  "1\n00:00:01,000 --> 00:00:02,000\n<i>Un.</i>\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n");
    FakePrompts prompts;
    MainWindow window{files, translated(files, SubtitleFormat::Lrc), prompts};
    window.show();
    selectRow(window, 0);
    currentAt(window, 0, kTextColumn);
    window.copyAction()->trigger();

    window.table()->selectionModel()->clearSelection();
    selectRow(window, 1);
    currentAt(window, 1, kTranslationColumn);
    window.pasteAction()->trigger();

    CHECK(cellAt(window, 1, kTranslationColumn) == "Un.");
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.at(0) == "pasting SubRip texts into LRC: 1 tag dropped");
}

TEST_CASE("cutting in the translation column empties the translation only", "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    selectRow(window, 1);
    currentAt(window, 1, kTranslationColumn);

    window.cutAction()->trigger();

    CHECK(cellAt(window, 1, kTranslationColumn).empty());
    CHECK(cellAt(window, 1, kTextColumn) == "Deux.");
}

TEST_CASE("the status bar says which text is aimed at when there are two", "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();

    currentAt(window, 0, kTextColumn);
    CHECK(window.targetStatus()->text() == QStringLiteral("Text: Main"));

    currentAt(window, 0, kTranslationColumn);
    CHECK(window.targetStatus()->text() == QStringLiteral("Text: Translation"));

    currentAt(window, 0, kStartColumn);
    CHECK(window.targetStatus()->text() == QStringLiteral("Text: Main"));
}

TEST_CASE("the status bar says nothing when there is only one text", "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, plain(files), prompts};
    window.show();

    CHECK(window.targetStatus()->text().isEmpty());
    CHECK_FALSE(window.targetStatus()->isVisible());
}

TEST_CASE("hiding the column brings the target back to the main text", "[gui][GUI-TRANS-05]") {
    // A column one cannot see is not one whose cell is current: the operation
    // that follows must not reach a text nobody is looking at.
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);
    window.translationColumnAction()->trigger();

    window.italicAction()->trigger();

    CHECK(cellAt(window, 0, kTextColumn) == "<i>Un.</i>");
    CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
    CHECK(window.targetStatus()->text().isEmpty());
}

TEST_CASE("an operation on positions moves both texts with the subtitle", "[gui][GUI-TRANS-05]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<subedit::gui::ShiftDialog&>(dialog).setTyped(QStringLiteral("00:00:01,000"));
    };
    MainWindow window{files, translated(files), prompts};
    window.show();
    currentAt(window, 0, kTranslationColumn);

    window.shiftAction()->trigger();

    CHECK(cellAt(window, 0, kStartColumn) == "00:00:02,000");
    CHECK(cellAt(window, 0, kTextColumn) == "Un.");
    CHECK(cellAt(window, 0, kTranslationColumn) == "One.");
    CHECK(cellAt(window, 2, kStartColumn) == "00:00:06,000");
}
