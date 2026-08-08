#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::Project;
using subedit::core::RemoveCommand;
using subedit::core::Selection;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle named(std::string_view text, std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = std::string{text}};
}

[[nodiscard]] Project fourSubtitles() {
    Project project;
    project.setSubtitles({named("a", 0), named("b", 2000), named("c", 4000), named("d", 6000)});
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

[[nodiscard]] Selection discontinuous() {
    const std::array<SubtitleIndex, 2> indices = {SubtitleIndex::fromValue(1),
                                                  SubtitleIndex::fromValue(3)};
    return Selection::of(indices);
}

} // namespace

TEST_CASE("a removal takes the selected subtitles out", "[edit][remove]") {
    Project project = fourSubtitles();
    RemoveCommand command{discontinuous()};

    command.apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "c"});
}

TEST_CASE("undoing a removal puts the subtitles back at their own indices", "[edit][remove]") {
    // The criterion of the issue, and the reason `Project::remove` hands back
    // what it took: a discontinuous removal cannot be undone by appending.
    Project project = fourSubtitles();
    const std::vector<std::string> before = textsOf(project);
    RemoveCommand command{discontinuous()};

    command.apply(project);
    command.revert(project);

    CHECK(textsOf(project) == before);
}

TEST_CASE("undoing a removal restores the subtitles themselves, not copies of a neighbour",
          "[edit][remove]") {
    Project project = fourSubtitles();
    RemoveCommand command{discontinuous()};

    command.apply(project);
    command.revert(project);

    CHECK(project.subtitleAt(SubtitleIndex::fromValue(1)).start.milliseconds() == 2000);
    CHECK(project.subtitleAt(SubtitleIndex::fromValue(3)).start.milliseconds() == 6000);
}

TEST_CASE("removing everything and undoing it restores the whole project", "[edit][remove]") {
    Project project = fourSubtitles();
    const std::vector<std::string> before = textsOf(project);
    RemoveCommand command{Selection::all(project)};

    command.apply(project);
    CHECK(project.count() == 0);

    command.revert(project);
    CHECK(textsOf(project) == before);
}

TEST_CASE("removing nothing is not a special case", "[edit][remove]") {
    Project project = fourSubtitles();
    RemoveCommand command{Selection::of({})};

    command.apply(project);
    command.revert(project);

    CHECK(project.count() == 4);
}

TEST_CASE("a removal reports the indices it emptied", "[edit][remove]") {
    const RemoveCommand command{discontinuous()};

    const std::vector<subedit::core::Change> changes = command.describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Removal);
    CHECK(changes[0].indices ==
          std::vector<SubtitleIndex>{SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(3)});
}

TEST_CASE("a removal says what it is", "[edit][remove]") {
    const RemoveCommand command{discontinuous()};

    CHECK(command.kind() == CommandKind::Remove);
}

TEST_CASE("a removal marks both documents", "[edit][remove]") {
    Session session{fourSubtitles()};

    session.apply(std::make_unique<RemoveCommand>(discontinuous()));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 1);
}

TEST_CASE("undoing then redoing a removal restores the exact state", "[edit][remove]") {
    Session session{fourSubtitles()};

    session.apply(std::make_unique<RemoveCommand>(discontinuous()));
    session.undo();
    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "b", "c", "d"});

    session.redo();
    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "c"});
}
