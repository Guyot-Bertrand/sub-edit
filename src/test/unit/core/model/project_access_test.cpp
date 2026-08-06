#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

Project withThreeSubtitles() {
    Project project;
    project.setSubtitles({
        Subtitle{.mainText = "Un."},
        Subtitle{.mainText = "Deux."},
        Subtitle{.mainText = "Trois."},
    });
    return project;
}

} // namespace

TEST_CASE("a project counts its subtitles", "[model][project]") {
    CHECK(Project{}.count() == 0);
    CHECK(withThreeSubtitles().count() == 3);
}

TEST_CASE("a subtitle is reached by its index", "[model][project]") {
    const Project project = withThreeSubtitles();

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
    CHECK(project.subtitleAt(SubtitleIndex::fromNumber(3)).mainText == "Trois.");
}

TEST_CASE("a subtitle reached by its index can be modified", "[model][project]") {
    // What a command needs: change one subtitle without rebuilding the whole
    // vector, which on a file of several thousand would be absurd.
    Project project = withThreeSubtitles();

    project.subtitleAt(SubtitleIndex::fromValue(1)).mainText = "Deux, corrigé.";
    project.subtitleAt(SubtitleIndex::fromValue(1)).end = Timestamp::fromMilliseconds(2000);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(1)).mainText == "Deux, corrigé.");
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(1)).end.milliseconds() == 2000);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(0)).mainText == "Un.");
}

TEST_CASE("an index beyond the last subtitle is refused loudly", "[model][project]") {
    // Louder than undefined behaviour: a wrong index is a programming error,
    // and it should stop the test rather than read whatever lies after the
    // vector.
    Project project = withThreeSubtitles();

    CHECK_THROWS_AS(project.subtitleAt(SubtitleIndex::fromValue(3)), std::out_of_range);
}
