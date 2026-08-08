#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::InsertCommand;
using subedit::core::Project;
using subedit::core::Session;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, std::string_view text = "") {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = std::string{text}};
}

[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

/// The start and end of each subtitle, in milliseconds.
[[nodiscard]] std::vector<std::int64_t> boundsOf(const Project& project) {
    std::vector<std::int64_t> bounds;
    bounds.reserve(project.count() * 2);
    for (const Subtitle& subtitle : project.subtitles()) {
        bounds.push_back(subtitle.start.milliseconds());
        bounds.push_back(subtitle.end.milliseconds());
    }
    return bounds;
}

} // namespace

TEST_CASE("subtitles given are inserted where they were asked for", "[edit][insert]") {
    Project project = projectOf({at(0, 1000, "a"), at(5000, 6000, "d")});
    InsertCommand command{SubtitleIndex::fromValue(1), {at(2000, 3000, "b"), at(3000, 4000, "c")}};

    command.apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "b", "c", "d"});
}

TEST_CASE("undoing an insertion takes back exactly what it added", "[edit][insert]") {
    Project project = projectOf({at(0, 1000, "a"), at(5000, 6000, "d")});
    InsertCommand command{SubtitleIndex::fromValue(1), {at(2000, 3000, "b")}};

    command.apply(project);
    command.revert(project);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "d"});
    CHECK(boundsOf(project) == std::vector<std::int64_t>{0, 1000, 5000, 6000});
}

TEST_CASE("an insertion reports the indices it filled", "[edit][insert]") {
    const InsertCommand command{SubtitleIndex::fromValue(1), {at(2000, 3000), at(3000, 4000)}};

    const std::vector<subedit::core::Change> changes = command.describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Insertion);
    CHECK(changes[0].indices ==
          std::vector<SubtitleIndex>{SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(2)});
}

TEST_CASE("an insertion says what it is", "[edit][insert]") {
    const InsertCommand command{SubtitleIndex::fromValue(0), {at(0, 1000)}};

    CHECK(command.kind() == CommandKind::Insert);
}

TEST_CASE("blank subtitles added at the end last three seconds", "[edit][insert]") {
    // Gaupol's rule: with no subtitle after them, there is no window to share,
    // so they take a duration that is merely reasonable.
    Project project = projectOf({at(0, 7000)});
    InsertCommand command = InsertCommand::blank(project, SubtitleIndex::fromValue(1), 2);

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{0, 7000, 7000, 10000, 10000, 13000});
}

TEST_CASE("blank subtitles added to an empty project start at the origin", "[edit][insert]") {
    Project project = projectOf({});
    InsertCommand command = InsertCommand::blank(project, SubtitleIndex::fromValue(0), 1);

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{0, 3000});
}

TEST_CASE("blank subtitles share the window before the next one", "[edit][insert]") {
    // Four seconds of room between the end of the first and the start of the
    // next, split in two.
    Project project = projectOf({at(0, 1000), at(5000, 6000)});
    InsertCommand command = InsertCommand::blank(project, SubtitleIndex::fromValue(1), 2);

    command.apply(project);

    CHECK(boundsOf(project) ==
          std::vector<std::int64_t>{0, 1000, 1000, 3000, 3000, 5000, 5000, 6000});
}

TEST_CASE("blank subtitles inserted first start at the origin", "[edit][insert]") {
    Project project = projectOf({at(5000, 6000)});
    InsertCommand command = InsertCommand::blank(project, SubtitleIndex::fromValue(0), 2);

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{0, 2500, 2500, 5000, 5000, 6000});
}

TEST_CASE("blank subtitles inserted before a file that starts before the origin fit in front",
          "[edit][insert]") {
    // A file whose first subtitle sits before zero leaves no room in front of
    // it. Three seconds are taken from further back rather than overlapping.
    Project project = projectOf({at(-5000, -4000)});
    InsertCommand command = InsertCommand::blank(project, SubtitleIndex::fromValue(0), 1);

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{-8000, -5000, -5000, -4000});
}

TEST_CASE("blank subtitles with no room to share are empty of duration", "[edit][insert]") {
    // The window is negative when the file overlaps itself at that point.
    // Nothing sensible can be shared, and inventing a duration would push the
    // new subtitles over their neighbour.
    Project project = projectOf({at(0, 9000), at(5000, 6000)});
    InsertCommand command = InsertCommand::blank(project, SubtitleIndex::fromValue(1), 1);

    command.apply(project);

    CHECK(boundsOf(project) == std::vector<std::int64_t>{0, 9000, 9000, 9000, 5000, 6000});
}

TEST_CASE("an insertion marks both documents", "[edit][insert]") {
    Session session{projectOf({at(0, 1000)})};

    session.apply(std::make_unique<InsertCommand>(SubtitleIndex::fromValue(1),
                                                  std::vector<Subtitle>{at(2000, 3000)}));

    CHECK(session.modificationCount(Document::Main) == 1);
    CHECK(session.modificationCount(Document::Translation) == 1);
}

TEST_CASE("undoing then redoing an insertion restores the exact state", "[edit][insert]") {
    Session session{projectOf({at(0, 1000, "a"), at(5000, 6000, "d")})};

    session.apply(std::make_unique<InsertCommand>(SubtitleIndex::fromValue(1),
                                                  std::vector<Subtitle>{at(2000, 3000, "b")}));
    session.undo();
    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "d"});

    session.redo();
    CHECK(textsOf(session.project()) == std::vector<std::string>{"a", "b", "d"});
}
