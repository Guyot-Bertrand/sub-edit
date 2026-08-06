#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>
#include <vector>

namespace {

using subedit::core::FrameRate;
using subedit::core::Newline;
using subedit::core::Project;
using subedit::core::SourceFile;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

std::vector<Subtitle> twoSubtitles() {
    return {
        Subtitle{
            .start = Timestamp::fromMilliseconds(1000),
            .end = Timestamp::fromMilliseconds(3000),
            .mainText = "Premier.",
        },
        Subtitle{
            .start = Timestamp::fromMilliseconds(4000),
            .end = Timestamp::fromMilliseconds(6000),
            .mainText = "Second.",
        },
    };
}

} // namespace

TEST_CASE("a new project is empty", "[model][project]") {
    const Project project;

    CHECK(project.subtitles().empty());
}

TEST_CASE("a new project runs at 24000/1001, as Gaupol does", "[model][project]") {
    const Project project;

    CHECK(project.frameRate() == FrameRate{StandardFrameRate::Fps23976});
}

TEST_CASE("a project holds the subtitles it is given", "[model][project]") {
    Project project;

    project.setSubtitles(twoSubtitles());

    REQUIRE(project.subtitles().size() == 2);
    CHECK(project.subtitles()[0].mainText == "Premier.");
    CHECK(project.subtitles()[1].mainText == "Second.");
}

TEST_CASE("the frame rate of a project can be changed", "[model][project]") {
    Project project;

    project.setFrameRate(FrameRate{StandardFrameRate::Fps25});

    CHECK(project.frameRate() == FrameRate{StandardFrameRate::Fps25});
}

TEST_CASE("a project that came from nowhere has no source file", "[model][project]") {
    const Project project;

    CHECK_FALSE(project.sourceFile().path.has_value());
    CHECK(project.sourceFile().newline == Newline::Lf);
    CHECK_FALSE(project.sourceFile().hadUtf8Bom);
    CHECK(project.sourceFile().header.empty());
}

TEST_CASE("a project keeps what the file it came from looked like", "[model][project]") {
    // These are not decorations: writing back a file that arrived with a BOM
    // and CRLF endings without them would rewrite every line of the diff the
    // user sees in their version control.
    Project project;

    project.setSourceFile(SourceFile{
        .path = std::filesystem::path{"/films/dialogue.vtt"},
        .newline = Newline::CrLf,
        .hadUtf8Bom = true,
        .header = "WEBVTT - Dialogue",
    });

    const std::optional<std::filesystem::path> path = project.sourceFile().path;
    if (!path.has_value()) {
        FAIL("the project lost the path of the file it came from");
        return;
    }

    CHECK(path->filename() == "dialogue.vtt");
    CHECK(project.sourceFile().newline == Newline::CrLf);
    CHECK(project.sourceFile().hadUtf8Bom);
    CHECK(project.sourceFile().header == "WEBVTT - Dialogue");
}

TEST_CASE("a project holds an incoherent subtitle without complaint", "[model][project]") {
    // ADR 0008 again, at the level of the collection this time: opening a file
    // must not fail because one of its subtitles ends before it starts.
    Project project;

    project.setSubtitles({Subtitle{
        .start = Timestamp::fromMilliseconds(9000),
        .end = Timestamp::fromMilliseconds(1000),
    }});

    REQUIRE(project.subtitles().size() == 1);
    CHECK(project.subtitles()[0].duration().milliseconds() == -8000);
}

TEST_CASE("copying a project copies its subtitles", "[model][project]") {
    Project project;
    project.setSubtitles(twoSubtitles());

    Project copy = project;
    copy.setSubtitles({});

    CHECK(copy.subtitles().empty());
    CHECK(project.subtitles().size() == 2);
}
