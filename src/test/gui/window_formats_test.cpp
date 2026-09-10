// The nine formats, seen from the window — issue #352.
//
// **The same promises the command line keeps, on the other surface.** What is
// asserted here is what the window makes of a corpus file and of an answer: the
// nine open, the nine can be written, and a format that will not carry
// everything says so while the answer can still be « no ».
//
// The bytes come from `formats/`, the versioned corpus of one scene in nine
// renderings, and are handed to an `InMemoryFileSystem` — a real corpus for the
// content, a fake disk for the window, which is what every other window test
// does.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/prompts.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QLabel>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <expected>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::SaveTarget;
using subedit::test::FakePrompts;

/// One rendering of the scene: the corpus file, and the format it is in.
struct Rendering {
    std::string_view file;
    SubtitleFormat format;
};

constexpr std::array<Rendering, 9> kNine = {
    Rendering{.file = "scene.srt", .format = SubtitleFormat::SubRip},
    Rendering{.file = "scene.vtt", .format = SubtitleFormat::WebVtt},
    Rendering{.file = "scene.subviewer2.sub", .format = SubtitleFormat::SubViewer2},
    Rendering{.file = "scene.ssa", .format = SubtitleFormat::SubStationAlpha},
    Rendering{.file = "scene.ass", .format = SubtitleFormat::AdvancedSubStationAlpha},
    Rendering{.file = "scene.microdvd.sub", .format = SubtitleFormat::MicroDvd},
    Rendering{.file = "scene.mpl2.txt", .format = SubtitleFormat::Mpl2},
    Rendering{.file = "scene.tmplayer.txt", .format = SubtitleFormat::TMPlayer},
    Rendering{.file = "scene.lrc", .format = SubtitleFormat::Lrc},
};

/// The bytes of one corpus file, or a failed test.
[[nodiscard]] std::string corpus(std::string_view name) {
    const std::filesystem::path path =
        std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "formats" / std::string{name};
    const std::ifstream file{path, std::ios::binary};
    if (!file) {
        FAIL("le corpus versionné est introuvable : " << path.string());
        return {};
    }

    std::ostringstream bytes;
    bytes << file.rdbuf();
    return bytes.str();
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const std::string& path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// Whatever the diagnostics panel is showing, as one string.
[[nodiscard]] std::string panelText(const MainWindow& window) {
    std::string text;
    for (int row = 0; row < window.diagnostics()->count(); ++row)
        text += window.diagnostics()->lineAt(row).toStdString() + "\n";
    return text;
}

} // namespace

TEST_CASE("the window opens each of the nine formats", "[gui][GUI-FORMAT-01]") {
    for (const Rendering& rendering : kNine) {
        INFO("fichier : " << rendering.file);
        InMemoryFileSystem files;
        files.addFile(std::string{rendering.file}, corpus(rendering.file));

        FakePrompts prompts;
        prompts.nextFileToOpen = std::string{rendering.file};
        MainWindow window{files, OpenedFile{}, prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(window.table()->model()->rowCount() == 4);
    }
}

TEST_CASE("saving as writes each of the nine, and they read back as themselves",
          "[gui][GUI-FORMAT-02]") {
    // **The other direction of the same promise**, and the reading is what
    // judges it: a file written as MPL2 that came back SubRip would pass any
    // assertion made on its bytes alone.
    for (const Rendering& rendering : kNine) {
        INFO("format : " << subedit::core::nameOf(rendering.format));
        InMemoryFileSystem files;
        files.addFile("scene.srt", corpus("scene.srt"));

        FakePrompts prompts;
        const std::string written =
            "written" + std::string{subedit::core::extensionOf(rendering.format)};
        prompts.nextSaveTarget = SaveTarget{.path = written, .format = rendering.format};
        MainWindow window{files, fileIn(files, "scene.srt"), prompts};
        window.show();

        window.saveAsAction()->trigger();

        const auto reopened = openProject(files, written);
        REQUIRE(reopened.has_value());
        CHECK(reopened->project.sourceFile().format == rendering.format);
        CHECK(reopened->project.count() == 4);
    }
}

TEST_CASE("saving into a format that will not carry everything says so first",
          "[gui][GUI-FORMAT-03]") {
    // **Before the writing and not after**, which is the whole difference
    // between a warning and a report. LRC states no end, joins the lines of a
    // subtitle, and has no vocabulary for the italic the scene wears.
    InMemoryFileSystem files;
    files.addFile("scene.srt", corpus("scene.srt"));

    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "scene.lrc", .format = SubtitleFormat::Lrc};
    MainWindow window{files, fileIn(files, "scene.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    REQUIRE(prompts.losses.size() == 1);
    CHECK_THAT(prompts.losses.front(), ContainsSubstring("ends are not carried by LRC"));
    CHECK_THAT(prompts.losses.front(), ContainsSubstring("line breaks were joined"));
    CHECK_THAT(prompts.losses.front(), ContainsSubstring("tag dropped"));
    CHECK(files.contentOf("scene.lrc").has_value());
}

TEST_CASE("refusing the warning writes nothing and moves nothing", "[gui][GUI-FORMAT-03]") {
    // A warning whose « no » wrote the file anyway would be worse than none:
    // it would teach the user that the question is decoration.
    InMemoryFileSystem files;
    files.addFile("scene.srt", corpus("scene.srt"));

    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "scene.lrc", .format = SubtitleFormat::Lrc};
    prompts.nextLossAccepted = false;
    MainWindow window{files, fileIn(files, "scene.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK(prompts.losses.size() == 1);
    CHECK_FALSE(files.contentOf("scene.lrc").has_value());
    CHECK(window.windowTitle().contains(QStringLiteral("scene.srt")));
}

TEST_CASE("a conversion that loses nothing asks nothing", "[gui][GUI-FORMAT-03]") {
    // **The half of the policy that is harder to keep.** A box that says
    // « nothing will be lost » teaches a user to dismiss the one that matters,
    // so the question is not asked at all.
    InMemoryFileSystem files;
    files.addFile("scene.srt", corpus("scene.srt"));

    FakePrompts prompts;
    prompts.nextSaveTarget =
        SaveTarget{.path = "scene.ass", .format = SubtitleFormat::AdvancedSubStationAlpha};
    MainWindow window{files, fileIn(files, "scene.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK(prompts.losses.empty());
    CHECK(files.contentOf("scene.ass").has_value());
}

TEST_CASE("the tags are translated on the way out, not copied", "[gui][GUI-FORMAT-03]") {
    // ADR 0031, on this surface: an Advanced SSA that still said `<i>` would be
    // a file the format never interprets — text the user sees, which is worse
    // than a loss. And the document itself has become that file, so the table
    // shows what the file holds.
    InMemoryFileSystem files;
    files.addFile("scene.srt", corpus("scene.srt"));

    FakePrompts prompts;
    prompts.nextSaveTarget =
        SaveTarget{.path = "scene.ass", .format = SubtitleFormat::AdvancedSubStationAlpha};
    MainWindow window{files, fileIn(files, "scene.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    const std::string written = files.contentOf("scene.ass").value_or("");
    CHECK_THAT(written, ContainsSubstring(R"({\i1})"));
    CHECK_FALSE(written.contains("<i>"));
}

TEST_CASE("a document counted in frames says its rate where a grid would be",
          "[gui][GUI-FRAMES-01]") {
    // **The rate replaces the grid rather than joining it.** Deducing one from
    // positions that were computed from frames at that very rate would answer
    // with the number it was given — the same choice `inspect` makes.
    InMemoryFileSystem files;
    files.addFile("scene.sub", corpus("scene.microdvd.sub"));

    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "scene.sub"), prompts};
    window.show();

    CHECK(window.gridStatus()->text() == QStringLiteral("Frames: 24000/1001 fps"));
    CHECK_FALSE(window.gridStatus()->text().contains(QStringLiteral("Grid")));
}

TEST_CASE("opening a file counted in frames announces the rate nobody chose",
          "[gui][GUI-FRAMES-01]") {
    // The file states none — no MicroDVD file does — so the reading leaves a
    // diagnostic behind, and the panel is where the window shows it.
    InMemoryFileSystem files;
    files.addFile("scene.sub", corpus("scene.microdvd.sub"));

    FakePrompts prompts;
    prompts.nextFileToOpen = "scene.sub";
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();

    window.openAction()->trigger();

    CHECK_THAT(panelText(window), ContainsSubstring("counts in frames and states no rate"));
}

TEST_CASE("a document counted in time keeps the grid line", "[gui][GUI-FRAMES-01]") {
    // The other side of the same choice: eight formats of nine count in time,
    // and none of them loses the line the deduction is there to write.
    InMemoryFileSystem files;
    files.addFile("scene.srt", corpus("scene.srt"));

    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "scene.srt"), prompts};
    window.show();

    CHECK_FALSE(window.gridStatus()->text().contains(QStringLiteral("Frames")));
}

TEST_CASE("a file counted in frames comes back with the same frames", "[gui][GUI-FRAMES-01]") {
    // **The rate the file was read at is the rate it is written back at**, and
    // that is what makes the round trip exact: the document works against it
    // from the moment it was read, and nothing in between changed it.
    InMemoryFileSystem files;
    files.addFile("scene.sub", corpus("scene.microdvd.sub"));

    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "again.sub", .format = SubtitleFormat::MicroDvd};
    MainWindow window{files, fileIn(files, "scene.sub"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK(files.contentOf("again.sub").value_or("") == corpus("scene.microdvd.sub"));
}

TEST_CASE("a document saved into frames starts counting in them", "[gui][GUI-FRAMES-01]") {
    // The document has become that file: it counts in frames now, so the status
    // bar drops the grid — which would only find the rate it was just given —
    // and a reading of what was written lands back on the same positions.
    InMemoryFileSystem files;
    files.addFile("scene.srt", corpus("scene.srt"));

    FakePrompts prompts;
    prompts.nextSaveTarget = SaveTarget{.path = "scene.sub", .format = SubtitleFormat::MicroDvd};
    MainWindow window{files, fileIn(files, "scene.srt"), prompts};
    window.show();

    window.saveAsAction()->trigger();

    CHECK_THAT(window.gridStatus()->text().toStdString(), ContainsSubstring("Frames:"));

    // One second, counted at 24000/1001: the twenty-fourth frame. Written by
    // the rate the document works against, and by no other.
    CHECK_THAT(files.contentOf("scene.sub").value_or(""), ContainsSubstring("{24}"));
}
