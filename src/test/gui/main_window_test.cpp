#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>

#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <string>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::Project;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::SubtitleFormat;
using subedit::gui::MainWindow;
using subedit::gui::openProject;

constexpr const char* kThree = "1\n"
                               "00:00:01,000 --> 00:00:02,000\n"
                               "Un.\n"
                               "\n"
                               "2\n"
                               "00:00:03,000 --> 00:00:04,000\n"
                               "Deux.\n"
                               "\n"
                               "3\n"
                               "00:00:05,000 --> 00:00:06,000\n"
                               "Trois.\n";

[[nodiscard]] InMemoryFileSystem withFile(const std::string& path, const std::string& content) {
    InMemoryFileSystem files;
    files.addFile(path, content);
    return files;
}

} // namespace

TEST_CASE("opening a file gives a project that remembers where it came from",
          "[gui][GUI-OPEN-01]") {
    const InMemoryFileSystem files = withFile("film.srt", kThree);

    const std::expected<Project, ReadError> project = openProject(files, "film.srt");

    REQUIRE(project.has_value());
    CHECK(project->count() == 3);
    CHECK(project->sourceFile().format == SubtitleFormat::SubRip);
    // L'option entière plutôt que son contenu : clang-tidy ne reconnaît pas le
    // REQUIRE de Catch2 comme une vérification.
    CHECK(project->sourceFile().path == std::filesystem::path{"film.srt"});
}

TEST_CASE("opening what is not a subtitle file fails, and says why", "[gui][GUI-OPEN-02]") {
    const InMemoryFileSystem files = withFile("film.srt", "rien de reconnaissable\n");

    const std::expected<Project, ReadError> project = openProject(files, "film.srt");

    REQUIRE_FALSE(project.has_value());
    CHECK(project.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("opening a file that is not there fails", "[gui][GUI-OPEN-02]") {
    const InMemoryFileSystem files;

    CHECK_FALSE(openProject(files, "absent.srt").has_value());
}

TEST_CASE("the window shows the subtitles of the project it was given", "[gui][GUI-OPEN-01]") {
    const InMemoryFileSystem files = withFile("film.srt", kThree);
    const std::expected<Project, ReadError> project = openProject(files, "film.srt");
    REQUIRE(project.has_value());

    const MainWindow window{*project};

    REQUIRE(window.table() != nullptr);
    REQUIRE(window.table()->model() != nullptr);
    CHECK(window.table()->model()->rowCount({}) == 3);
    CHECK(window.table()->model()->columnCount({}) == 5);
}

TEST_CASE("a window with no file shows an empty table", "[gui][GUI-OPEN-01]") {
    // Not a case to guard against: an empty project is a project, and the table
    // over it is empty rather than absent.
    const MainWindow window{Project{}};

    CHECK(window.table()->model()->rowCount({}) == 0);
}

TEST_CASE("the window names the file it holds", "[gui][GUI-OPEN-01]") {
    const InMemoryFileSystem files = withFile("film.srt", kThree);
    const std::expected<Project, ReadError> project = openProject(files, "film.srt");
    REQUIRE(project.has_value());

    const MainWindow window{*project};

    CHECK(window.windowTitle().toStdString().find("film.srt") != std::string::npos);
}
