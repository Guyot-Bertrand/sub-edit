// What the window looks like before anyone has done anything to it — issue #211.
//
// None of these change what the program computes; all of them change whether
// someone can find it. They are gathered here rather than spread through the
// tests of the features they touch, because what they have in common is being
// about the window itself.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/version.hpp>
#include <subedit/gui/about_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAction>
#include <QBrush>
#include <QDialog>
#include <QStringList>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <grid_fixtures.hpp>
#include <string>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::AboutDialog;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

[[nodiscard]] MainWindow windowOn(InMemoryFileSystem& files, FakePrompts& prompts) {
    files.addFile("a.srt", subedit::test::gridBytes("grille-24.srt"));
    return MainWindow{files, openProject(files, "a.srt").value(), prompts, {}, {}};
}

} // namespace

TEST_CASE("the window opens wide enough to read the table", "[gui][appearance]") {
    // Qt sizes a window that never asks to its layout's hints, and a table's
    // hint knows nothing of how many rows there are: the result was a window
    // showing a handful of subtitles and a truncated text column.
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn(files, prompts);
    window.show();

    CHECK(window.width() >= 1200);
    CHECK(window.height() >= 800);
}

TEST_CASE("the menus read in the order a user meets them", "[gui][appearance]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window = windowOn(files, prompts);

    // The document, what one does to it, what accompanies it, what inspects it,
    // what explains it.
    CHECK(window.menuTitles() == QStringList{QStringLiteral("&File"),
                                             QStringLiteral("&Edit"),
                                             QStringLiteral("&Video"),
                                             QStringLiteral("&Tools"),
                                             QStringLiteral("&Help")});
}

TEST_CASE("with no film the window offers a way to choose one", "[gui][appearance]") {
    // An absence a user cannot act on is worse than an empty band: hiding the
    // picture left nothing at all where one would go, and the only way in was a
    // menu one had to know about.
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn(files, prompts);
    window.show();

    CHECK(window.noVideoBanner()->isVisible());
    CHECK_FALSE(window.videoView()->isVisible());
}

TEST_CASE("the about entry says which version is running", "[gui][appearance]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window = windowOn(files, prompts);
    window.show();

    std::string said;
    prompts.fill = [&said](QDialog& dialog) {
        if (auto* about = dynamic_cast<AboutDialog*>(&dialog))
            said = about->text().toStdString();
    };

    window.aboutAction()->trigger();

    CHECK_THAT(said, ContainsSubstring(std::string{subedit::core::versionString()}));
    CHECK_THAT(said, ContainsSubstring("GPL"));
}

TEST_CASE("the manual entry is there, and out until it has one", "[gui][appearance]") {
    // Phase 7 brings the complete user manual and the packaging that decides
    // where it lives. The entry is here so the shape of the menu is settled
    // now; an entry that opened nothing would be worse than one that says it is
    // not ready.
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window = windowOn(files, prompts);

    REQUIRE(window.manualAction() != nullptr);
    CHECK_FALSE(window.manualAction()->isEnabled());
}
