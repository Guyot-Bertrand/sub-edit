#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/sort_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::ChangeKind;
using subedit::core::CommandKind;
using subedit::core::Project;
using subedit::core::SortCommand;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle at(std::int64_t start, std::string_view text) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
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

} // namespace

TEST_CASE("sorting puts the subtitles in order of their start", "[edit][sort]") {
    Project project = projectOf({at(4000, "c"), at(0, "a"), at(2000, "b")});
    SortCommand command;

    command.apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "b", "c"});
    CHECK(project.outOfOrder().empty());
}

TEST_CASE("sorting is stable on equal starts", "[edit][sort]") {
    // Neither of two subtitles starting at the same moment precedes the other,
    // so moving them would be a decision nobody asked for.
    //
    // Enough of them to actually catch an unstable sort: on a handful, an
    // unstable implementation happens to keep the order — the standard library
    // falls back to insertion sort on short ranges — and the test would pass
    // on a wrong sort. Forty is past that threshold.
    constexpr std::size_t kEqualStarts = 40;
    std::vector<Subtitle> subtitles;
    subtitles.reserve(kEqualStarts + 1);
    subtitles.push_back(at(2000, "dernier"));
    for (std::size_t rank = 0; rank < kEqualStarts; ++rank)
        subtitles.push_back(at(1000, std::to_string(rank)));

    Project project = projectOf(std::move(subtitles));
    SortCommand command;

    command.apply(project);

    std::vector<std::string> expected;
    expected.reserve(kEqualStarts + 1);
    for (std::size_t rank = 0; rank < kEqualStarts; ++rank)
        expected.push_back(std::to_string(rank));
    expected.emplace_back("dernier");

    CHECK(textsOf(project) == expected);
}

TEST_CASE("undoing a sort restores the order the file had", "[edit][sort]") {
    Project project = projectOf({at(4000, "c"), at(0, "a"), at(2000, "b")});
    const std::vector<std::string> before = textsOf(project);
    SortCommand command;

    command.apply(project);
    command.revert(project);

    CHECK(textsOf(project) == before);
}

TEST_CASE("sorting an ordered project moves nothing and says nothing", "[edit][sort]") {
    // A strict policy appends this command after every operation that could
    // have broken the order, and most of them will not have. Reporting a
    // change there would mark a document as differing from its file for no
    // reason.
    Project project = projectOf({at(0, "a"), at(2000, "b")});
    SortCommand command;

    command.apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "b"});
    CHECK(command.describe().empty());
}

TEST_CASE("a sort reports the rows that moved", "[edit][sort]") {
    Project project = projectOf({at(4000, "c"), at(0, "a"), at(2000, "b")});
    SortCommand command;

    command.apply(project);

    const std::vector<subedit::core::Change> changes = command.describe();
    REQUIRE(changes.size() == 1);
    CHECK(changes[0].kind == ChangeKind::Reordering);
    CHECK(changes[0].subtitles.count() == 3);
}

TEST_CASE("sorting an empty project is not a special case", "[edit][sort]") {
    Project project = projectOf({});
    SortCommand command;

    command.apply(project);
    command.revert(project);

    CHECK(project.count() == 0);
    CHECK(command.describe().empty());
}

TEST_CASE("redoing a sort puts the order back", "[edit][sort]") {
    // The spec asks it of each of the eight operations: applying then undoing
    // restores the exact state, and redoing reproduces it. The sort was the
    // one covered only through a composite, where a failure would have been
    // read as the composite's.
    Project project = projectOf({at(4000, "c"), at(0, "a"), at(2000, "b")});
    SortCommand command;

    command.apply(project);
    command.revert(project);
    command.apply(project);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("a sort says what it is", "[edit][sort]") {
    CHECK(SortCommand{}.kind() == CommandKind::Sort);
}
