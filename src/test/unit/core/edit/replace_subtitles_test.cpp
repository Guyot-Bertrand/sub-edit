// Replacing every subtitle of a project at once: what a transcription does to it.

#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/replace_subtitles.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::CommandKind;
using subedit::core::Project;
using subedit::core::replaceAllSubtitles;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle named(std::string_view text, std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text},
                    .translationText = "t-" + std::string{text}};
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

} // namespace

TEST_CASE("every subtitle is replaced, in the order given, and one undo gives them back",
          "[edit][transcribe]") {
    Project project;
    project.setSubtitles({named("a", 0), named("b", 2000), named("c", 4000)});
    const std::vector<Subtitle> before{project.subtitles().begin(), project.subtitles().end()};

    auto command = replaceAllSubtitles(project, {named("x", 100), named("y", 3000)});
    REQUIRE(command != nullptr);
    CHECK(command->kind() == CommandKind::Transcribe);

    command->apply(project);
    CHECK(textsOf(project) == std::vector<std::string>{"x", "y"});

    command->revert(project);
    CHECK(std::vector<Subtitle>{project.subtitles().begin(), project.subtitles().end()} == before);
}

TEST_CASE("an empty project is given the subtitles, and an undo empties it again",
          "[edit][transcribe]") {
    Project project;

    auto command = replaceAllSubtitles(project, {named("x", 0)});
    REQUIRE(command != nullptr);
    command->apply(project);
    CHECK(textsOf(project) == std::vector<std::string>{"x"});

    command->revert(project);
    CHECK(project.count() == 0);
}

TEST_CASE("replacing everything by nothing empties the project", "[edit][transcribe]") {
    Project project;
    project.setSubtitles({named("a", 0)});

    auto command = replaceAllSubtitles(project, {});
    REQUIRE(command != nullptr);
    command->apply(project);

    CHECK(project.count() == 0);
}

TEST_CASE("nothing to remove and nothing to put is no command", "[edit][transcribe]") {
    const Project project;

    CHECK(replaceAllSubtitles(project, {}) == nullptr);
}
