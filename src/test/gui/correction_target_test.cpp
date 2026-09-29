// The correction assistant's target — issue #505.
//
// Pure resolution, tested directly against real `ProjectPage` objects: no
// window, no `View`. `ProjectSearch::replaceAllAcrossProjects` walks every
// open project the same way, through `View::projectCount()`/`project(int)`,
// but never turns that walk into a reusable value — this is the first place
// that does.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QItemSelectionModel>
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::CorrectionScope;
using subedit::gui::correctionTargetsOf;
using subedit::gui::ProjectPage;

constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                             "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n";

[[nodiscard]] std::unique_ptr<ProjectPage> pageOn(const char* content) {
    InMemoryFileSystem files;
    files.addFile("/film.srt", content);
    auto opened = openProject(files, "/film.srt");
    REQUIRE(opened.has_value());
    return ProjectPage::make(std::move(opened->project));
}

} // namespace

TEST_CASE("the selection scope reads the current page's own selection, and nothing else",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo));
    pages.push_back(pageOn(kTwo));
    pages[0]->tableSelection->select(pages[0]->model->index(1, 0),
                                     QItemSelectionModel::Select | QItemSelectionModel::Rows);

    const auto targets = correctionTargetsOf(CorrectionScope::Selection, Document::Main, pages, 0);

    REQUIRE(targets.size() == 1);
    CHECK(targets[0].project == &pages[0]->session->project());
    CHECK(targets[0].selection.count() == 1);
}

TEST_CASE("the current-project scope takes the whole file of the page on screen",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo));
    pages.push_back(pageOn(kTwo));

    const auto targets =
        correctionTargetsOf(CorrectionScope::CurrentProject, Document::Main, pages, 1);

    REQUIRE(targets.size() == 1);
    CHECK(targets[0].project == &pages[1]->session->project());
    CHECK(targets[0].selection.count() == 2);
}

TEST_CASE("the all-projects scope takes every open project, whole", "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo));
    pages.push_back(pageOn(kTwo));

    const auto targets =
        correctionTargetsOf(CorrectionScope::AllProjects, Document::Main, pages, 0);

    REQUIRE(targets.size() == 2);
    CHECK(targets[0].project == &pages[0]->session->project());
    CHECK(targets[1].project == &pages[1]->session->project());
}

TEST_CASE("a project with no translation is left out when the target is the translation",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo)); // no translation file opened

    const auto targets =
        correctionTargetsOf(CorrectionScope::AllProjects, Document::Translation, pages, 0);

    CHECK(targets.empty());
}

TEST_CASE("the current-project scope answers nothing on the translation without one",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo)); // no translation file opened

    const auto targets =
        correctionTargetsOf(CorrectionScope::CurrentProject, Document::Translation, pages, 0);

    CHECK(targets.empty());
}

TEST_CASE("the selection scope answers nothing with no row selected", "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo)); // nothing selected

    const auto targets = correctionTargetsOf(CorrectionScope::Selection, Document::Main, pages, 0);

    CHECK(targets.empty());
}
