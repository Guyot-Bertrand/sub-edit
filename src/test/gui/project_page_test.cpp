// What makes a project blank — issue #477.
//
// A blank project is the one a window starts on, or that `New Project` makes:
// nothing in it anybody could lose. A file opened then takes its place rather
// than a tab of its own. Each case below keeps one thing in the project, and
// one is enough for it not to be blank.

#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/project_page.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace {

using subedit::core::Document;
using subedit::core::Project;
using subedit::core::SourceFile;
using subedit::gui::isBlank;
using subedit::gui::ProjectPage;

[[nodiscard]] Project oneSubtitle() {
    Project project;
    project.setSubtitles(
        {subedit::core::Subtitle{.start = subedit::core::Timestamp::fromMilliseconds(1000),
                                 .end = subedit::core::Timestamp::fromMilliseconds(2000),
                                 .mainText = "Un."}});
    return project;
}

} // namespace

TEST_CASE("a project with nothing in it is blank", "[gui][GUI-TABS-01]") {
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(Project{});

    CHECK(isBlank(*page));
}

TEST_CASE("a project with a file is not blank, even an empty one", "[gui][GUI-TABS-01]") {
    Project project;
    project.setSourceFile(SourceFile{.path = "vide.srt"});
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(std::move(project));

    CHECK_FALSE(isBlank(*page));
}

TEST_CASE("a project with a translation file is not blank", "[gui][GUI-TABS-01]") {
    Project project;
    project.setSourceFile(Document::Translation, SourceFile{.path = "vide.en.srt"});
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(std::move(project));

    CHECK_FALSE(isBlank(*page));
}

TEST_CASE("a project with a subtitle is not blank", "[gui][GUI-TABS-01]") {
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(oneSubtitle());

    CHECK_FALSE(isBlank(*page));
}

TEST_CASE("a project with a history is not blank, even emptied again", "[gui][GUI-TABS-01]") {
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(Project{});
    const Project& project = page->session->project();
    (void)page->session->apply(
        std::make_unique<subedit::core::InsertCommand>(subedit::core::InsertCommand::blank(
            project, subedit::core::SubtitleIndex::fromValue(0), 1)));
    (void)page->session->undo();
    REQUIRE(page->session->project().count() == 0);

    // What was undone can be redone: closing the page would lose that.
    CHECK_FALSE(isBlank(*page));
}

TEST_CASE("a project with a film is not blank", "[gui][GUI-TABS-01]") {
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(Project{});
    page->session->chooseVideo("film.mkv");

    CHECK_FALSE(isBlank(*page));
}

TEST_CASE("a project marked modified is not blank", "[gui][GUI-TABS-01]") {
    const std::unique_ptr<ProjectPage> page = ProjectPage::make(Project{});
    page->session->markUnsaved(Document::Main);

    CHECK_FALSE(isBlank(*page));
}
