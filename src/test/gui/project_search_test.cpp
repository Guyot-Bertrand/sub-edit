// The search, without the window — ADR 0034, issue #460.
//
// `ProjectSearch` reaches the projects through its `View`; these cases give it
// a double that holds a few pages and records what it was asked. The window's
// own cases, in `window_search_test.cpp` and `window_search_projects_test.cpp`,
// are unchanged: what they prove, this does too, one level down.

#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/project_search.hpp>
#include <subedit/gui/search_dialog.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::Document;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::SubtitleIndex;
using subedit::gui::ProjectPage;
using subedit::gui::ProjectSearch;
using subedit::gui::SearchDialog;

[[nodiscard]] std::unique_ptr<ProjectPage> pageOf(std::initializer_list<std::string_view> texts) {
    std::vector<subedit::core::Subtitle> subtitles;
    std::int64_t start = 0;
    for (const std::string_view text : texts) {
        subtitles.push_back(
            subedit::core::Subtitle{.start = subedit::core::Timestamp::fromMilliseconds(start),
                                    .end = subedit::core::Timestamp::fromMilliseconds(start + 1000),
                                    .mainText = std::string{text},
                                    .translationText = "T " + std::string{text}});
        start += 2000;
    }
    Project project;
    project.setSubtitles(std::move(subtitles));
    return ProjectPage::make(std::move(project));
}

/// The window, as the search sees it: pages, one of them shown, and a log of
/// what was asked.
class Desk final : public ProjectSearch::View {

public:
    std::vector<std::unique_ptr<ProjectPage>> pages;
    int current = 0;
    Document aimed = Document::Main;
    bool both = false;
    std::vector<int> shows;
    std::vector<int> moves;

    [[nodiscard]] int projectCount() const override { return static_cast<int>(pages.size()); }

    [[nodiscard]] ProjectPage& project(int index) override {
        return *pages.at(static_cast<std::size_t>(index));
    }

    [[nodiscard]] int shownProject() const override { return current; }

    void show(int index) override {
        current = index;
        shows.push_back(index);
    }

    [[nodiscard]] Document targetDocument() const override { return aimed; }

    [[nodiscard]] bool twoTexts() const override { return both; }

    [[nodiscard]] Selection selectionTarget() const override {
        return Selection::all(pages.at(static_cast<std::size_t>(current))->session->project());
    }

    void moveTo(int row) override { moves.push_back(row); }

    void apply(std::unique_ptr<subedit::core::Command> command,
               const Selection& /*target*/) override {
        ProjectPage& page = project(current);
        page.model->applied(page.session->apply(std::move(command)));
    }
};

[[nodiscard]] std::string textOf(const ProjectPage& page, std::size_t row) {
    return page.session->project().subtitleAt(SubtitleIndex::fromValue(row)).mainText;
}

[[nodiscard]] std::string statusOf(const ProjectSearch& search) {
    return search.dialog()->statusLabel()->text().toStdString();
}

/// Opens the dialog and types `pattern`.
SearchDialog& typing(ProjectSearch& search, const char* pattern) {
    search.open();
    SearchDialog* dialog = search.dialog();
    REQUIRE(dialog != nullptr);
    dialog->patternField()->setText(QString::fromUtf8(pattern));
    return *dialog;
}

} // namespace

TEST_CASE("the search finds in the project shown, and moves to the match", "[gui][GUI-SEARCH-01]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Rien.", "Bonjour Marie.", "Marie."}));
    ProjectSearch search{desk, &parent};
    typing(search, "marie");

    search.find(true);
    search.find(true);

    CHECK(desk.moves == std::vector<int>{1, 2});
    CHECK(desk.shows.empty());
    CHECK(statusOf(search).empty());
}

TEST_CASE("the search says what it did not find, and moves nothing", "[gui][GUI-SEARCH-01]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Rien."}));
    ProjectSearch search{desk, &parent};
    typing(search, "Sophie");

    search.find(true);

    CHECK(desk.moves.empty());
    CHECK(statusOf(search) == "\"Sophie\" not found");
}

TEST_CASE("across projects the search visits the tabs in order, and says it wrapped",
          "[gui][GUI-SEARCH-04]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Marie."}));
    desk.pages.push_back(pageOf({"Rien."}));
    desk.pages.push_back(pageOf({"Rien.", "Marie."}));
    ProjectSearch search{desk, &parent};
    typing(search, "marie").allProjectsCheck()->setChecked(true);

    search.find(true);
    CHECK(desk.current == 0);
    search.find(true);
    CHECK(desk.current == 2);
    CHECK(desk.moves.back() == 1);
    CHECK(statusOf(search).empty());

    search.find(true);
    CHECK(desk.current == 0);
    CHECK(statusOf(search) == "Search wrapped around");
}

TEST_CASE("replace all across projects is one entry of history in each project touched",
          "[gui][GUI-SEARCH-04]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Marie.", "Marie."}));
    desk.pages.push_back(pageOf({"Rien."}));
    desk.pages.push_back(pageOf({"Adieu Marie."}));
    desk.current = 1;
    ProjectSearch search{desk, &parent};
    SearchDialog& dialog = typing(search, "marie");
    dialog.allProjectsCheck()->setChecked(true);
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    search.replaceAll();

    CHECK(statusOf(search) == "replaced 3 matches in 2 projects");
    CHECK(desk.current == 1);
    CHECK(textOf(*desk.pages.at(0), 1) == "Sophie.");
    CHECK(textOf(*desk.pages.at(2), 0) == "Adieu Sophie.");
    CHECK(desk.pages.at(0)->session->undoableCount() == 1);
    CHECK(desk.pages.at(1)->session->undoableCount() == 0);
    CHECK(desk.pages.at(2)->session->undoableCount() == 1);
}

TEST_CASE("the search forgets a match when the text it aims at changes", "[gui][GUI-SEARCH-03]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Marie."}));
    desk.both = true;
    ProjectSearch search{desk, &parent};
    typing(search, "marie");
    search.find(true);
    REQUIRE(desk.pages.front()->match.has_value());
    CHECK(search.dialog()->fieldLabel()->text().toStdString() == "Searching in: Main");

    desk.aimed = Document::Translation;
    search.refresh();

    CHECK_FALSE(desk.pages.front()->match.has_value());
    CHECK(search.dialog()->fieldLabel()->text().toStdString() == "Searching in: Translation");
}

TEST_CASE("changing the pattern forgets the match of every project", "[gui][GUI-SEARCH-04]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Marie."}));
    desk.pages.push_back(pageOf({"Marie."}));
    ProjectSearch search{desk, &parent};
    SearchDialog& dialog = typing(search, "marie");
    search.find(true);
    desk.pages.at(1)->match =
        subedit::core::TextMatch{.index = SubtitleIndex::fromValue(0), .start = 0, .end = 5};

    dialog.patternField()->setText(QStringLiteral("Mari"));

    CHECK_FALSE(desk.pages.at(0)->match.has_value());
    CHECK_FALSE(desk.pages.at(1)->match.has_value());
}

TEST_CASE("the dialog outlives the search without calling it", "[gui][GUI-SEARCH-01]") {
    QWidget parent;
    Desk desk;
    desk.pages.push_back(pageOf({"Marie."}));
    SearchDialog* dialog = nullptr;
    {
        ProjectSearch search{desk, &parent};
        dialog = &typing(search, "marie");
    }

    // The search is gone, the dialog is not: pressing its buttons reaches
    // nothing — and, under the sanitizer, reads nothing freed.
    dialog->nextButton()->click();
    dialog->patternField()->setText(QStringLiteral("x"));

    CHECK(desk.moves.empty());
}
