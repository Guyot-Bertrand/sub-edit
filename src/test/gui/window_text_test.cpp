// The case and the dialogue dashes, from the window — issue #379.
//
// Both rewrite the visible text and leave the tags alone, which is what the
// tag-aware parser of #378 is for. What is checked here is the surface: the
// five entries, the target they act on, and what the window says of it.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/letter_case.hpp>
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
using subedit::core::LetterCase;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\n<i>bonjour</i> marie\n\n"
                             "2\n00:00:03,000 --> 00:00:04,000\nau revoir\n\n";

[[nodiscard]] InMemoryFileSystem withFile(const char* content) {
    InMemoryFileSystem files;
    files.addFile("film.srt", content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
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

TEST_CASE("the four cases are four entries, and the tags do not follow", "[gui][GUI-CASE-01]") {
    InMemoryFileSystem files = withFile(kTwo);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.caseAction(LetterCase::Upper)->trigger();
    CHECK(textAt(window, 0) == "<i>BONJOUR</i> MARIE");
    CHECK(textAt(window, 1) == "AU REVOIR");

    window.caseAction(LetterCase::Title)->trigger();
    CHECK(textAt(window, 0) == "<i>Bonjour</i> Marie");

    window.caseAction(LetterCase::Sentence)->trigger();
    CHECK(textAt(window, 0) == "<i>Bonjour</i> marie");

    window.caseAction(LetterCase::Lower)->trigger();
    CHECK(textAt(window, 0) == "<i>bonjour</i> marie");
}

TEST_CASE("a case change enters the history, and comes back out", "[gui][GUI-CASE-01]") {
    InMemoryFileSystem files = withFile(kTwo);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.caseAction(LetterCase::Upper)->trigger();
    REQUIRE(window.undoAction()->isEnabled());
    CHECK(window.undoAction()->text().toStdString() == "Undo: changing the case");
    CHECK(prompts.outcomes.back() == "2 subtitles recased");

    window.undoAction()->trigger();
    CHECK(textAt(window, 0) == "<i>bonjour</i> marie");
}

TEST_CASE("only what is selected takes the case", "[gui][GUI-CASE-01]") {
    InMemoryFileSystem files = withFile(kTwo);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    selectRow(window, 1);
    window.caseAction(LetterCase::Upper)->trigger();

    CHECK(textAt(window, 0) == "<i>bonjour</i> marie");
    CHECK(textAt(window, 1) == "AU REVOIR");
}

TEST_CASE("a target already in the case asked for is not an operation", "[gui][GUI-CASE-01]") {
    InMemoryFileSystem files = withFile(kTwo);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.caseAction(LetterCase::Lower)->trigger();

    CHECK(prompts.outcomes.back() == "nothing to change");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("one entry puts the dialogue dashes on and takes them off", "[gui][GUI-DASH-01]") {
    InMemoryFileSystem files = withFile(kTwo);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.dialogueDashesAction()->trigger();
    CHECK(textAt(window, 0) == "<i>- bonjour</i> marie");
    CHECK(textAt(window, 1) == "- au revoir");
    CHECK(prompts.outcomes.back() == "2 subtitles dashed");
    CHECK(window.undoAction()->text().toStdString() == "Undo: adding dialogue dashes");

    window.dialogueDashesAction()->trigger();
    CHECK(textAt(window, 0) == "<i>bonjour</i> marie");
    CHECK(textAt(window, 1) == "au revoir");
    CHECK(prompts.outcomes.back() == "2 subtitles undashed");
    CHECK(window.undoAction()->text().toStdString() == "Undo: removing dialogue dashes");
}

TEST_CASE("one subtitle without a dash sends the whole target into them", "[gui][GUI-DASH-01]") {
    InMemoryFileSystem files = withFile("1\n00:00:01,000 --> 00:00:02,000\n- bonjour\n\n"
                                        "2\n00:00:03,000 --> 00:00:04,000\nau revoir\n\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.dialogueDashesAction()->trigger();

    CHECK(textAt(window, 0) == "- bonjour");
    CHECK(textAt(window, 1) == "- au revoir");
    // One was already right, so only the other was rewritten.
    CHECK(prompts.outcomes.back() == "1 subtitle dashed");
}

TEST_CASE("the five entries are out on an empty document", "[gui][GUI-CASE-01]") {
    InMemoryFileSystem files = withFile("");
    files.addFile("vide.srt", "1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n");
    auto opened = openProject(files, "vide.srt");
    REQUIRE(opened.has_value());

    FakePrompts prompts;
    MainWindow window{files, std::move(*opened), prompts};
    window.show();
    window.table()->selectAll();
    window.removeAction()->trigger();

    CHECK_FALSE(window.dialogueDashesAction()->isEnabled());
    CHECK_FALSE(window.caseAction(LetterCase::Upper)->isEnabled());
}

TEST_CASE("a blank row has no case and no dash to give", "[gui][GUI-DASH-01]") {
    // The one target that changes nothing either way: a row with no text is no
    // replica, so it asks for no dash — and taking off what is not there is not
    // an operation.
    InMemoryFileSystem files = withFile("1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n"
                                        "2\n00:00:03,000 --> 00:00:04,000\n\n");
    FakePrompts prompts;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    selectRow(window, 1);
    window.dialogueDashesAction()->trigger();
    CHECK(prompts.outcomes.back() == "nothing to change");

    window.caseAction(LetterCase::Sentence)->trigger();
    CHECK(prompts.outcomes.back() == "nothing to change");
    CHECK_FALSE(window.undoAction()->isEnabled());
}
