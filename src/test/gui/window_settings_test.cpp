// What the window finds again from one session to the next — issue #240.
//
// **The whole round trip is the criterion**, and not each half taken apart. A
// window that says its geometry and another that knows how to lay one down
// prove nothing together for as long as what comes out of the first has not
// gone into the second by way of a file.
//
// Nothing here touches a real location: the file is in memory, and its path is
// given. It is the seam of ADR 0022 and the harness of #238.

#include <subedit/core/config/editor_settings.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/manual_path.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/subtitle_editor.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/theme.hpp>
#include <subedit/platform/locations.hpp>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QImage>
#include <QRect>
#include <QSpinBox>
#include <QTest>
#include <Qt>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::EditorSettings;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::LengthUnit;
using subedit::core::openProject;
using subedit::core::Settings;
using subedit::core::Theme;
using subedit::core::WindowGeometry;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr const char* kThree = "1\n"
                               "00:00:01,000 --> 00:00:02,000\n"
                               "Un.\n"
                               "\n"
                               "2\n"
                               "00:00:03,000 --> 00:00:04,000\n"
                               "Deux.\n"
                               "\n";

constexpr const char* kPath = "/config/subedit/settings.conf";

/// "Dark", third in the list — the order of the dialog is system, light, dark,
/// the one that does nothing first since it is the default.
constexpr int kDarkIndex = 2;

/// A window on a document, shown: a window never shown has no real geometry,
/// and so nothing to keep.
class Windowed {
public:
    Windowed() {
        m_files.addFile("film.srt", kThree);
        m_window = std::make_unique<MainWindow>(
            m_files, openProject(m_files, "film.srt").value(), m_prompts);
    }

    [[nodiscard]] MainWindow& window() { return *m_window; }

    [[nodiscard]] InMemoryFileSystem& files() { return m_files; }

    [[nodiscard]] FakePrompts& prompts() { return m_prompts; }

private:
    InMemoryFileSystem m_files;
    FakePrompts m_prompts;
    std::unique_ptr<MainWindow> m_window;
};

} // namespace

TEST_CASE("the window reports where it is and what its columns do",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();
    window.setGeometry(30, 50, 1000, 700);

    const Settings said = window.settings();

    // `value_or` rather than a dereference after `REQUIRE`: the static
    // analysis does not read a Catch2 macro as a guard, and a default would
    // answer zero here, which the comparison refuses just as much.
    CHECK(said.geometry.value_or(WindowGeometry{}).width == 1000);
    CHECK(said.geometry.value_or(WindowGeometry{}).height == 700);
    CHECK_FALSE(said.maximised);
    CHECK(said.columnWidths.size() == subedit::core::kColumnWidthCount);
}

TEST_CASE("a geometry that was set is the one the window takes", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();

    window.applySettings(
        Settings{.geometry = WindowGeometry{.x = 20, .y = 40, .width = 900, .height = 640}});
    window.show();

    CHECK(window.width() == 900);
    CHECK(window.height() == 640);
}

TEST_CASE("column widths that were set are the ones the table takes",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();

    window.applySettings(Settings{.columnWidths = {40, 130, 130, 130}});

    CHECK(window.table()->columnWidth(0) == 40);
    CHECK(window.table()->columnWidth(1) == 130);
}

// **The criterion of the requirement, and the whole round trip**: what one
// session leaves, the next finds again, by way of the file.
TEST_CASE("a session finds the geometry and columns of the previous one",
          "[gui][config][GUI-CONFIG-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    {
        Windowed first;
        first.window().show();
        first.window().setGeometry(15, 25, 1100, 720);
        first.window().applySettings(Settings{.columnWidths = {45, 125, 125, 125}});

        subedit::gui::writeUserSettings(files, kPath, first.window().settings(), errors);
    }

    Windowed second;
    second.window().applySettings(subedit::gui::readUserSettings(files, kPath, errors));
    second.window().show();

    CHECK(second.window().width() == 1100);
    CHECK(second.window().height() == 720);
    CHECK(second.window().table()->columnWidth(0) == 45);
    CHECK(second.window().table()->columnWidth(3) == 125);
    CHECK(errors.str().empty());
}

TEST_CASE("a window left maximised reopens maximised", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();

    window.applySettings(Settings{.maximised = true});
    window.show();

    CHECK((window.windowState() & Qt::WindowMaximized) != 0);
}

// **A missing geometry leaves the window to size itself**, and that is what
// makes the first launch identical to the one from before the preferences: wide
// enough to read the table — #211.
TEST_CASE("with no settings, the window opens as it always has", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();

    window.applySettings(Settings{});
    window.show();

    CHECK(window.width() >= 1200);
    CHECK(window.height() >= 800);
}

// ## The handle — #254

TEST_CASE("the share given to the table is set and read back", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.setGeometry(0, 0, 1000, 800);
    window.show();

    window.applySettings(Settings{.tableShare = 40});
    const Settings once = window.settings();

    // **Stable rather than exact**, and that is the right promise. The band at
    // the top has a minimum height, so a share too small is brought back to
    // what the splitter accepts — which is right, and no defect. What has to
    // hold is that a share read back and laid down again does not move: without
    // that, the handle would drift from one launch to the next.
    REQUIRE(once.tableShare.has_value());
    window.applySettings(once);
    CHECK(window.settings().tableShare == once.tableShare);
}

TEST_CASE("a share read back and laid down again does not creep, whatever the height",
          "[gui][config][GUI-CONFIG-01]") {
    // Reading rounds down, so laying down at the pixel below made the share come back
    // one per cent lower unless the height happened to be a multiple of a hundred:
    // the handle crept up the window at every launch, and the test above passed only
    // for heights where it could not. Any height must hold.
    for (const int height : {700, 713, 727, 741, 768, 777, 803, 839}) {
        INFO("height " << height);
        Windowed fixture;
        MainWindow& window = fixture.window();
        window.setGeometry(0, 0, 1000, height);
        window.show();

        window.applySettings(Settings{.tableShare = 45});
        const Settings once = window.settings();
        window.applySettings(once);
        const Settings twice = window.settings();
        window.applySettings(twice);

        CHECK(twice.tableShare == once.tableShare);
        CHECK(window.settings().tableShare == once.tableShare);
    }
}

TEST_CASE("a larger share gives a taller table", "[gui][config][GUI-CONFIG-01]") {
    // The other half: a stable share that meant nothing would be stable for
    // nothing.
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.setGeometry(0, 0, 1000, 800);
    window.show();

    window.applySettings(Settings{.tableShare = 30});
    const int narrow = window.settings().tableShare.value_or(0);
    window.applySettings(Settings{.tableShare = 80});

    CHECK(window.settings().tableShare.value_or(0) > narrow);
}

TEST_CASE("with no share saved, the handle stays where the window puts it",
          "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();
    const Settings before = window.settings();

    window.applySettings(Settings{});

    CHECK(window.settings().tableShare == before.tableShare);
}

// ## The last directory — #254

TEST_CASE("the open dialog starts in the last file's directory", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.lastDirectory = std::filesystem::path{"/films/quai"}});
    window.show();

    window.openAction()->trigger();

    CHECK(fixture.prompts().lastOpenDirectory == std::filesystem::path{"/films/quai"});
}

TEST_CASE("a cancelled dialog does not move the remembered directory",
          "[gui][config][GUI-CONFIG-01]") {
    // What counts is where the user works, not where they looked.
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.lastDirectory = std::filesystem::path{"/films/quai"}});
    window.show();
    fixture.prompts().nextFileToOpen = {};

    window.openAction()->trigger();

    CHECK(window.settings().lastDirectory == std::filesystem::path{"/films/quai"});
}

TEST_CASE("opening a file remembers its directory", "[gui][config][GUI-CONFIG-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    fixture.files().addFile("/ailleurs/autre.srt", kThree);
    fixture.prompts().nextFileToOpen = "/ailleurs/autre.srt";
    window.show();

    window.openAction()->trigger();

    CHECK(window.settings().lastDirectory == std::filesystem::path{"/ailleurs"});
}

// ## The theme — #241

TEST_CASE("the theme chosen in the preferences is the one the window renders",
          "[gui][config][GUI-THEME-01]") {
    const subedit::gui::PreferencesDialog probe{Theme::System};
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();

    // The fake fills the dialog in and then answers "accepted", which is what
    // a human does by picking from the list before clicking.
    fixture.prompts().fill = [](QDialog& dialog) {
        auto* preferences = dynamic_cast<subedit::gui::PreferencesDialog*>(&dialog);
        if (preferences != nullptr)
            preferences->themeBox()->setCurrentIndex(kDarkIndex);
    };
    fixture.prompts().nextRun = true;

    window.preferencesAction()->trigger();

    CHECK(window.settings().theme == Theme::Dark);
    CHECK(probe.theme() == Theme::System);
}

TEST_CASE("cancelled preferences change nothing", "[gui][config][GUI-THEME-01]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.theme = Theme::Light});
    window.show();

    fixture.prompts().fill = [](QDialog& dialog) {
        auto* preferences = dynamic_cast<subedit::gui::PreferencesDialog*>(&dialog);
        if (preferences != nullptr)
            preferences->themeBox()->setCurrentIndex(kDarkIndex);
    };
    fixture.prompts().nextRun = false;

    window.preferencesAction()->trigger();

    CHECK(window.settings().theme == Theme::Light);
}

TEST_CASE("a saved theme is the one the window reopens with", "[gui][config][GUI-THEME-01]") {
    Windowed fixture;

    fixture.window().applySettings(Settings{.theme = Theme::Light});

    CHECK(fixture.window().settings().theme == Theme::Light);
}

// ## What the wiring writes on the error output

TEST_CASE("an unreadable value is named, with what was written", "[gui][config][GUI-CONFIG-02]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.geometry = plus tard\n");
    std::ostringstream errors;

    const Settings read = subedit::gui::readUserSettings(files, kPath, errors);

    CHECK_FALSE(read.geometry.has_value());
    CHECK_THAT(errors.str(), ContainsSubstring("window.geometry"));
    // The offending value is quoted: without it, the user has to hunt.
    CHECK_THAT(errors.str(), ContainsSubstring("\"plus tard\""));
    CHECK_THAT(errors.str(), ContainsSubstring("keeping the default"));
}

TEST_CASE("a missing file draws not a word", "[gui][config][GUI-CONFIG-02]") {
    const InMemoryFileSystem files;
    std::ostringstream errors;

    CHECK(subedit::gui::readUserSettings(files, kPath, errors) == Settings{});
    CHECK(errors.str().empty());
}

TEST_CASE("an unreadable file says so, and the window starts on the defaults",
          "[gui][config][GUI-CONFIG-02]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "window.maximised = true\n");
    files.failNextRead(FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    CHECK(subedit::gui::readUserSettings(files, kPath, errors) == Settings{});
    CHECK_THAT(errors.str(), ContainsSubstring("permission denied"));
}

// **What writing an option commented out buys**, seen from the file the
// program really writes: an option never touched is not frozen at the value of
// the day it was written, so a default one improves reaches everybody. Without
// it, improving one would reach nobody at all.
TEST_CASE("the written file comments out what stayed at its default",
          "[gui][config][GUI-CONFIG-03]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    subedit::gui::writeUserSettings(files, kPath, Settings{}, errors);

    const std::string written = files.contentOf(kPath).value_or("");
    CHECK_THAT(written, ContainsSubstring("#window.maximised = false"));
    CHECK_THAT(written, ContainsSubstring("#table.columns = "));
    CHECK(errors.str().empty());
}

TEST_CASE("an option that was set is written without a comment", "[gui][config][GUI-CONFIG-03]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    subedit::gui::writeUserSettings(files, kPath, Settings{.maximised = true}, errors);

    CHECK_THAT(files.contentOf(kPath).value_or(""),
               ContainsSubstring("\nwindow.maximised = true\n"));
}

// **The two overloads that resolve the location answer each other.** That is
// all there is to prove of them: that writing and then reading back, without
// ever saying where, gives back what was written — so that the two speak of the
// same place.
//
// **Nothing reaches a real file**, and not only because the harness moves
// `XDG_CONFIG_HOME`: the file system is in memory, so the resolved path is used
// as a key and nothing else.
TEST_CASE("writing then reading back without saying where returns what was written",
          "[gui][config][GUI-CONFIG-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    const Settings chosen{.maximised = true, .columnWidths = {41, 121, 121, 121}};

    subedit::gui::writeUserSettings(files, chosen, errors);

    CHECK(subedit::gui::readUserSettings(files, errors) == chosen);
    CHECK(errors.str().empty());
}

TEST_CASE("a refused write says so, and nothing more", "[gui][config][GUI-CONFIG-02]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    subedit::gui::writeUserSettings(files, kPath, Settings{}, errors);

    CHECK_THAT(errors.str(), ContainsSubstring("settings could not be written"));
}

// **`main.cpp`'s own three lines, composed in one call** — issue #505: moved
// out of the entry point once a fourth setter would have sent it over the
// budget `check-architecture.sh` holds it to. Nothing here reaches a real
// location, for the same reason the round trip above does not: the file
// system is in memory, so every resolved path is used as a key and nothing
// else — each of the three keys is computed here exactly as
// `configureFromEnvironment` computes it itself, and something is placed
// there beforehand, so each of its three effects is read back directly
// rather than trusted on the word of an empty `errors`.
TEST_CASE("configureFromEnvironment applies settings, the manual path and the "
          "pattern catalogue in one call",
          "[gui][config]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    InMemoryFileSystem& files = fixture.files();

    // The settings half: a theme distinct from the window's own starting
    // default (`Theme::System`), written at the location the resolving
    // overload of `readUserSettings` will look for it.
    std::ostringstream setup;
    subedit::gui::writeUserSettings(files, Settings{.theme = Theme::Dark}, setup);

    // The manual half: `setManualPath` lights `manualAction()` the moment it
    // finds an `index.md` beside the directory it was given — nothing lights
    // it on its own, every action starts disabled (`buildAction`).
    files.addFile(subedit::gui::installedManualPath() / "index.md", "# Manual\n");
    REQUIRE_FALSE(window.manualAction()->isEnabled());

    // The pattern-catalogue half: one real record, at the exact cascade
    // `Common Errors` reads by default — `Zyyy`, `CorrectionSettings{}`'s own
    // starting code.
    files.addFile(subedit::platform::installedPatternsPath() / "Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Letter I\nClasses=Human;\nPattern=a\n");

    std::ostringstream errors;
    subedit::gui::configureFromEnvironment(window, files, errors);

    CHECK(errors.str().empty());
    CHECK(window.settings().theme == Theme::Dark);
    CHECK(window.manualAction()->isEnabled());

    // Read back through the real wizard rather than a catalogue accessor —
    // `MainWindow` exposes none — the same way `window_correction_test.cpp`
    // already drives the assistant to a task page and reads its pattern list.
    window.show();
    bool patternSeen = false;
    FakePrompts& prompts = fixture.prompts();
    prompts.nextRun = false; // Cancel: reading the task page is all this needs
    prompts.fill = [&patternSeen](QDialog& dialog) {
        auto& wizard = dynamic_cast<subedit::gui::CorrectionWizard&>(dialog);
        wizard.show();
        while (wizard.currentId() != subedit::gui::CorrectionWizard::CommonErrorsId)
            wizard.next();
        patternSeen =
            wizard.commonErrorsPage().findChild<QCheckBox*>(QStringLiteral("Letter I")) != nullptr;
    };
    window.correctTextsAction()->trigger();

    CHECK(patternSeen);
}

// ## The length of each line — issue #526

TEST_CASE("GUI-EDIT-04: the preferences offer the unit and the two switches",
          "[gui][config][GUI-EDIT-04]") {
    const subedit::gui::PreferencesDialog defaults{Theme::System};
    CHECK(defaults.editor() == EditorSettings{});
    CHECK(defaults.lengthUnitBox()->currentIndex() == 0);
    CHECK(defaults.showLengthsInCellsBox()->isChecked());
    CHECK(defaults.showLengthsInEditorBox()->isChecked());

    const subedit::gui::PreferencesDialog chosen{Theme::System,
                                                 {.lengthUnit = LengthUnit::Characters,
                                                  .showLengthsInCells = false,
                                                  .showLengthsInEditor = true}};
    CHECK(chosen.lengthUnitBox()->currentIndex() == 1);
    CHECK(chosen.editor().lengthUnit == LengthUnit::Characters);
    CHECK_FALSE(chosen.editor().showLengthsInCells);
    CHECK(chosen.editor().showLengthsInEditor);
}

TEST_CASE("GUI-EDIT-04: the unit is greyed when no length is shown", "[gui][config][GUI-EDIT-04]") {
    const subedit::gui::PreferencesDialog dialog{Theme::System};
    CHECK(dialog.lengthUnitBox()->isEnabled());

    dialog.showLengthsInCellsBox()->setChecked(false);
    CHECK(dialog.lengthUnitBox()->isEnabled()); // the editor still shows them

    dialog.showLengthsInEditorBox()->setChecked(false);
    CHECK_FALSE(dialog.lengthUnitBox()->isEnabled());

    dialog.showLengthsInCellsBox()->setChecked(true);
    CHECK(dialog.lengthUnitBox()->isEnabled());
}

TEST_CASE("GUI-EDIT-04: accepted preferences set the editor's settings, cancelled ones do not",
          "[gui][config][GUI-EDIT-04]") {
    Windowed fixture;
    const MainWindow& window = fixture.window();
    CHECK(window.settings().editor == EditorSettings{});

    fixture.prompts().fill = [](QDialog& dialog) {
        auto* preferences = dynamic_cast<subedit::gui::PreferencesDialog*>(&dialog);
        if (preferences == nullptr)
            return;
        preferences->lengthUnitBox()->setCurrentIndex(1);
        preferences->showLengthsInEditorBox()->setChecked(false);
    };

    fixture.prompts().nextRun = false;
    window.preferencesAction()->trigger();
    CHECK(window.settings().editor == EditorSettings{});

    fixture.prompts().nextRun = true;
    window.preferencesAction()->trigger();
    CHECK(window.settings().editor == EditorSettings{.lengthUnit = LengthUnit::Characters,
                                                     .showLengthsInCells = true,
                                                     .showLengthsInEditor = false});
}

TEST_CASE("GUI-EDIT-04: the settings of the editor are the ones the window reopens with",
          "[gui][config][GUI-EDIT-04]") {
    Windowed fixture;
    const EditorSettings kept{.lengthUnit = LengthUnit::Characters,
                              .showLengthsInCells = false,
                              .showLengthsInEditor = false};

    fixture.window().applySettings(Settings{.editor = kept});

    CHECK(fixture.window().settings().editor == kept);
}

TEST_CASE("GUI-EDIT-04: changing the setting repaints the cells", "[gui][config][GUI-EDIT-04]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();
    QCoreApplication::processEvents();

    const QImage shown = window.table()->viewport()->grab().toImage();

    window.applySettings(Settings{.editor = {.showLengthsInCells = false}});
    QCoreApplication::processEvents();
    const QImage hidden = window.table()->viewport()->grab().toImage();
    CHECK(shown != hidden);

    window.applySettings(Settings{});
    QCoreApplication::processEvents();
    CHECK(window.table()->viewport()->grab().toImage() == shown);
}

TEST_CASE("GUI-EDIT-04: the editor the window opens carries the margin when the setting is on",
          "[gui][config][GUI-EDIT-04]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.show();
    window.activateWindow();
    QApplication::setActiveWindow(&window);
    REQUIRE(QTest::qWaitForWindowActive(&window));

    const QModelIndex cell = window.table()->model()->index(0, 4);

    SECTION("on") {
        // In characters: a length in ems depends on the font of the machine.
        window.applySettings(Settings{.editor = {.lengthUnit = LengthUnit::Characters}});
        window.table()->setCurrentIndex(cell);
        window.table()->edit(cell);
        auto* editor = window.table()->findChild<subedit::gui::SubtitleEditor*>();
        REQUIRE(editor != nullptr);
        CHECK(editor->gutterLengths() == std::vector<int>{3}); // "Un."
        CHECK(editor->gutterWidth() > 0);
    }

    SECTION("off") {
        window.applySettings(Settings{.editor = {.showLengthsInEditor = false}});
        window.table()->setCurrentIndex(cell);
        window.table()->edit(cell);
        auto* editor = window.table()->findChild<subedit::gui::SubtitleEditor*>();
        REQUIRE(editor != nullptr);
        CHECK(editor->gutterWidth() == 0);
    }
}

// ## How the video is driven — issue #615

TEST_CASE("GUI-SEEK-02: the preferences offer the jump and the lead-in",
          "[gui][config][GUI-SEEK-02]") {
    const subedit::gui::PreferencesDialog defaults{Theme::System};
    CHECK(defaults.seekLengthBox()->value() == 30);
    CHECK(defaults.contextLengthBox()->value() == 1.0);
    CHECK(defaults.video().seekLengthSeconds == 30);
    CHECK(defaults.video().contextLengthMilliseconds == 1000);

    const subedit::gui::PreferencesDialog chosen{
        Theme::System,
        {},
        {.seekLengthSeconds = 10, .contextLengthMilliseconds = 2500, .volume = 40}};
    CHECK(chosen.seekLengthBox()->value() == 10);
    CHECK(chosen.contextLengthBox()->value() == 2.5);
    CHECK(chosen.video().seekLengthSeconds == 10);
    CHECK(chosen.video().contextLengthMilliseconds == 2500);
}

TEST_CASE("GUI-SEEK-02: accepted preferences set the jump and keep the volume as it stands",
          "[gui][config][GUI-SEEK-02]") {
    Windowed fixture;
    MainWindow& window = fixture.window();
    window.applySettings(Settings{.video = {.volume = 40}});

    fixture.prompts().fill = [](QDialog& dialog) {
        auto* preferences = dynamic_cast<subedit::gui::PreferencesDialog*>(&dialog);
        if (preferences == nullptr)
            return;
        preferences->seekLengthBox()->setValue(12);
        preferences->contextLengthBox()->setValue(0.5);
    };

    fixture.prompts().nextRun = false;
    window.preferencesAction()->trigger();
    CHECK(window.settings().video.seekLengthSeconds == 30);

    fixture.prompts().nextRun = true;
    window.preferencesAction()->trigger();

    CHECK(window.settings().video.seekLengthSeconds == 12);
    CHECK(window.settings().video.contextLengthMilliseconds == 500);
    // The dialog does not hold the volume: the bar and the gestures set it, and it stays.
    CHECK(window.settings().video.volume == 40);
}

TEST_CASE("GUI-VOLUME-01: the settings the window reopens with carry the volume",
          "[gui][config][GUI-VOLUME-01]") {
    Windowed fixture;

    fixture.window().applySettings(Settings{.video = {.seekLengthSeconds = 20, .volume = 35}});

    CHECK(fixture.window().settings().video.volume == 35);
    CHECK(fixture.window().settings().video.seekLengthSeconds == 20);
}
