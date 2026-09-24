// Opening and saving, without the window — ADR 0034, issue #460.
//
// `ProjectFiles` reaches the tabs through its `View`; these cases give it a
// double that holds a few pages and records what it was asked. The window's
// own cases are unchanged: what they prove, this does too, one level down.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/gui/project_files.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QCheckBox>
#include <QDialog>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::gui::ProjectFiles;
using subedit::gui::ProjectPage;
using subedit::gui::SaveTarget;
using subedit::gui::UnsavedChoice;
using subedit::gui::UnsavedDocumentsDialog;
using subedit::test::FakePrompts;

constexpr const char* kOne = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n";
constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\nDeux.\n\n";

/// The window, as the files see it.
class Desk final : public ProjectFiles::View {

public:
    std::vector<std::unique_ptr<ProjectPage>> pages;
    int current = 0;
    std::vector<int> shows;
    std::vector<bool> moves;
    std::vector<std::string> announced;
    QWidget parent;

    [[nodiscard]] int projectCount() const override { return static_cast<int>(pages.size()); }

    [[nodiscard]] ProjectPage& project(int index) override {
        return *pages.at(static_cast<std::size_t>(index));
    }

    [[nodiscard]] int shownProject() const override { return current; }

    void show(int index) override {
        current = index;
        shows.push_back(index);
    }

    void saved(ProjectPage& /*page*/, Document /*document*/, bool moved) override {
        moves.push_back(moved);
    }

    void announce(const std::string& message) override { announced.push_back(message); }

    [[nodiscard]] QWidget* dialogParent() override { return &parent; }
};

/// A page read from `path`, as `Open…` would read it.
[[nodiscard]] std::unique_ptr<ProjectPage> pageFrom(const InMemoryFileSystem& files,
                                                    const char* path) {
    auto opened = subedit::core::openProject(files, path);
    REQUIRE(opened.has_value());
    return ProjectPage::make(std::move(opened->project), opened->diagnostics);
}

/// Changes the first text of `page`, one entry of its history.
void touch(ProjectPage& page, const char* text) {
    REQUIRE(page.model->setData(page.model->index(0, 4), QString::fromUtf8(text), Qt::EditRole));
}

[[nodiscard]] InMemoryFileSystem filesystem() {
    InMemoryFileSystem files;
    files.addFile("/films/un.srt", kOne);
    files.addFile("/films/deux.srt", kTwo);
    return files;
}

} // namespace

TEST_CASE("saving writes the file, remembers its directory and says it was not moved",
          "[gui][GUI-SAVE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    touch(*desk.pages.front(), "Un bis.");

    CHECK(projectFiles.save(*desk.pages.front(), Document::Main));

    CHECK(files.contentOf("/films/un.srt").value_or("").find("Un bis.") != std::string::npos);
    CHECK_FALSE(ProjectFiles::isModified(*desk.pages.front(), Document::Main));
    CHECK(desk.moves == std::vector<bool>{false});
    CHECK(projectFiles.lastDirectory() == "/films");
}

TEST_CASE("saving a document with no file asks where, and says it moved", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(ProjectPage::make(subedit::core::Project{}));
    ProjectFiles projectFiles{files, prompts, desk};
    prompts.nextSaveTarget = SaveTarget{.path = "/ailleurs/neuf.srt"};

    CHECK(projectFiles.save(*desk.pages.front(), Document::Main));

    CHECK(prompts.saveTargetAsked == 1);
    CHECK(files.contentOf("/ailleurs/neuf.srt").has_value());
    CHECK(desk.moves == std::vector<bool>{true});
    CHECK(desk.pages.front()->session->project().sourceFile().path == "/ailleurs/neuf.srt");
    CHECK(desk.pages.front()->writeEncoding.has_value());
}

TEST_CASE("a Save As that cannot write leaves the document where it was", "[gui][GUI-SAVE-02]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    prompts.nextSaveTarget = SaveTarget{.path = "/films/autre.srt"};
    files.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);

    CHECK_FALSE(projectFiles.saveAs(*desk.pages.front(), Document::Main));

    CHECK(desk.pages.front()->session->project().sourceFile().path == "/films/un.srt");
    CHECK(desk.moves.empty());
    CHECK(prompts.failures.size() == 1);
}

TEST_CASE("one modified document among the projects asks the plain question, over its tab",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    desk.pages.push_back(pageFrom(files, "/films/deux.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    touch(*desk.pages.at(1), "Deux bis.");
    prompts.nextUnsavedChoice = UnsavedChoice::Save;

    CHECK(projectFiles.mayDiscardAll());

    CHECK(prompts.unsavedAsked == 1);
    CHECK(desk.shows == std::vector<int>{1});
    CHECK(files.contentOf("/films/deux.srt").value_or("").find("Deux bis.") != std::string::npos);
}

TEST_CASE("two modified projects ask through the list, and save what is ticked",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    desk.pages.push_back(pageFrom(files, "/films/deux.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    touch(*desk.pages.at(0), "Un bis.");
    touch(*desk.pages.at(1), "Deux bis.");
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& list = dynamic_cast<UnsavedDocumentsDialog&>(dialog);
        list.boxes().at(0)->setChecked(false);
        list.saveButton()->click();
    };

    CHECK(projectFiles.mayDiscardAll());

    CHECK(files.contentOf("/films/un.srt").value_or("") == kOne);
    CHECK(files.contentOf("/films/deux.srt").value_or("").find("Deux bis.") != std::string::npos);
    CHECK(desk.shows == std::vector<int>{1});
}

TEST_CASE("the question about one project asks nothing of the others", "[gui][GUI-CLOSE-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    desk.pages.push_back(pageFrom(files, "/films/deux.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    touch(*desk.pages.at(1), "Deux bis.");

    CHECK(projectFiles.mayDiscard(0));

    CHECK(prompts.unsavedAsked == 0);
    CHECK(prompts.runAsked == 0);
}

TEST_CASE("Save All writes tab by tab, comes back, and says how many", "[gui][GUI-SAVE-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    desk.pages.push_back(pageFrom(files, "/films/deux.srt"));
    desk.current = 1;
    ProjectFiles projectFiles{files, prompts, desk};
    touch(*desk.pages.at(0), "Un bis.");
    touch(*desk.pages.at(1), "Deux bis.");

    projectFiles.saveAll();

    CHECK(desk.announced == std::vector<std::string>{"2 documents saved"});
    CHECK(desk.current == 1);
    CHECK(files.contentOf("/films/un.srt").value_or("").find("Un bis.") != std::string::npos);
}

TEST_CASE("Save All stops at a Save As given up, and says what was written", "[gui][GUI-SAVE-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    desk.pages.push_back(ProjectPage::make(subedit::core::Project{}));
    desk.pages.push_back(pageFrom(files, "/films/deux.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    touch(*desk.pages.at(0), "Un bis.");
    desk.pages.at(1)->session->markUnsaved(Document::Main);
    touch(*desk.pages.at(2), "Deux bis.");

    projectFiles.saveAll();

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() == "Save All stopped: 1 of 3 documents saved");
    CHECK(files.contentOf("/films/deux.srt").value_or("") == kTwo);
    CHECK(desk.announced.empty());
}

TEST_CASE("a file is found open however its path is spelled", "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    desk.pages.push_back(pageFrom(files, "/films/deux.srt"));
    ProjectFiles projectFiles{files, prompts, desk};

    CHECK(projectFiles.indexOf("/films/./autres/../deux.srt") == 1);
    CHECK_FALSE(projectFiles.indexOf("/films/trois.srt").has_value());
}

TEST_CASE("reading remembers the directory only of a file that opens", "[gui][GUI-OPEN-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    ProjectFiles projectFiles{files, prompts, desk};
    projectFiles.setLastDirectory("/avant");

    const auto missing = projectFiles.read("/ailleurs/absent.srt");
    REQUIRE_FALSE(missing.has_value());
    CHECK(missing.error() == "/ailleurs/absent.srt: does not exist");
    CHECK(projectFiles.lastDirectory() == "/avant");

    CHECK(projectFiles.read("/films/un.srt").has_value());
    CHECK(projectFiles.lastDirectory() == "/films");
}

TEST_CASE("a drop is sorted by name: films apart, subtitles in the order they came",
          "[gui][GUI-TABS-04]") {
    const std::vector<std::filesystem::path> paths{"b.srt", "film.MKV", "a.txt", "autre.mp4"};

    const ProjectFiles::Dropped dropped = ProjectFiles::sort(paths);

    CHECK(dropped.subtitles == std::vector<std::filesystem::path>{"b.srt", "a.txt"});
    CHECK(dropped.films == std::vector<std::filesystem::path>{"film.MKV", "autre.mp4"});
}

TEST_CASE("choosing a translation stops at a file that will not open, and says why",
          "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    prompts.nextFileToOpen = "/films/absent.srt";

    CHECK_FALSE(projectFiles.chooseTranslation(*desk.pages.front()).has_value());

    CHECK(prompts.failures.size() == 1);
    CHECK(prompts.runAsked == 0);
}

TEST_CASE("choosing a translation reads it and asks how to align it", "[gui][GUI-TRANS-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Desk desk;
    desk.pages.push_back(pageFrom(files, "/films/un.srt"));
    ProjectFiles projectFiles{files, prompts, desk};
    prompts.nextFileToOpen = "/films/deux.srt";
    prompts.nextRun = true;

    const auto chosen = projectFiles.chooseTranslation(*desk.pages.front());

    // `value_or` rather than a dereference after `REQUIRE`: the static
    // analysis does not read a Catch2 macro as a guard.
    CHECK(chosen.has_value());
    CHECK(chosen.transform([](const auto& one) { return one.read.lines.size(); }).value_or(0) == 1);
    CHECK(prompts.runAsked == 1);
    CHECK(projectFiles.lastDirectory() == "/films");
}
