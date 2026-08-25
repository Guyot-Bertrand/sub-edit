// Choosing a film, and the one the naming convention offers — issue #175.
//
// The chooser is a modal dialog of Qt, so it goes through the `Prompts` seam
// like every other: what is tested is not the dialog but what the window makes
// of the answer, including when the answer is « no ».

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/prompts.hpp>

#include <QAction>
#include <QLabel>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
using subedit::gui::SaveTarget;
using subedit::test::FakePrompts;

constexpr const char* kOne = "1\n"
                             "00:00:01,000 --> 00:00:02,000\n"
                             "Un.\n"
                             "\n";

/// A directory holding a subtitle file, and whatever else the case needs.
[[nodiscard]] InMemoryFileSystem directoryHolding(std::initializer_list<const char*> names) {
    InMemoryFileSystem files;
    for (const char* name : names)
        files.addFile(std::filesystem::path{"/films"} / name, "");
    files.addFile("/films/film.fr.srt", kOne);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string statusOf(const MainWindow& window) {
    return window.videoStatus()->text().toStdString();
}

} // namespace

TEST_CASE("choosing a video associates it, and the window names it", "[gui][GUI-VIDEO-01]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    prompts.nextVideoToOpen = "/ailleurs/autre-montage.mkv";
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();

    window.selectVideoAction()->trigger();

    CHECK(prompts.videoAsked == 1);
    CHECK(statusOf(window) == "Video: autre-montage.mkv");
}

// The chooser opens where the film almost always is, and where the convention
// has just looked: beside the subtitle file.
TEST_CASE("the chooser opens on the directory of the subtitle file", "[gui][GUI-VIDEO-01]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();

    window.selectVideoAction()->trigger();

    CHECK(prompts.lastVideoDirectory == std::filesystem::path{"/films"});
}

TEST_CASE("a chooser nobody answered changes nothing", "[gui][GUI-VIDEO-01]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();

    window.selectVideoAction()->trigger();

    CHECK(prompts.videoAsked == 1);
    CHECK(statusOf(window) == "No video");
}

TEST_CASE("opening a subtitle file proposes the film beside it", "[gui][GUI-VIDEO-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();

    // Nobody was asked anything: the convention speaks on its own.
    CHECK(prompts.videoAsked == 0);
    CHECK(statusOf(window) == "Video: film.mkv");
}

TEST_CASE("a directory with no film proposes none", "[gui][GUI-VIDEO-02]") {
    InMemoryFileSystem files = directoryHolding({"notes.txt"});
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();

    CHECK(statusOf(window) == "No video");
}

TEST_CASE("opening another file proposes the film beside that one", "[gui][GUI-VIDEO-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    files.addFile("/autre/serie.mkv", "");
    files.addFile("/autre/serie.srt", kOne);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();
    REQUIRE(statusOf(window) == "Video: film.mkv");

    prompts.nextFileToOpen = "/autre/serie.srt";
    window.openAction()->trigger();

    CHECK(statusOf(window) == "Video: serie.mkv");
}

// D5, and the reason the origin of the path is remembered at all. A « save as »
// makes the file answer to another name, so the convention speaks again — and
// what the user said stands.
TEST_CASE("a chosen film survives a save under another name", "[gui][GUI-VIDEO-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    prompts.nextVideoToOpen = "/ailleurs/le-bon-montage.mkv";
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts};
    window.show();
    window.selectVideoAction()->trigger();
    REQUIRE(statusOf(window) == "Video: le-bon-montage.mkv");

    prompts.nextSaveTarget =
        SaveTarget{.path = "/films/film.en.srt", .format = SubtitleFormat::SubRip};
    window.saveAsAction()->trigger();

    // `film.mkv` lies right beside the new name, and the convention would have
    // proposed it. It does not get to.
    CHECK(statusOf(window) == "Video: le-bon-montage.mkv");
}
