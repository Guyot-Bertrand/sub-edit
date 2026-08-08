#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

/// Builds a subtitle whose text names it, so that a test can say which one
/// moved without comparing positions.
[[nodiscard]] Subtitle named(std::string_view text) {
    return Subtitle{.mainText = std::string{text}};
}

/// Builds a subtitle starting at `start`, one second long.
[[nodiscard]] Subtitle startingAt(std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000)};
}

[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

[[nodiscard]] std::vector<std::string> textsOf(const Project& project) {
    std::vector<std::string> texts;
    for (const Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.mainText);
    return texts;
}

[[nodiscard]] std::vector<std::string> textsOf(const std::vector<Subtitle>& subtitles) {
    std::vector<std::string> texts;
    texts.reserve(subtitles.size());
    for (const Subtitle& subtitle : subtitles)
        texts.push_back(subtitle.mainText);
    return texts;
}

[[nodiscard]] std::vector<std::size_t> valuesOf(const std::vector<SubtitleIndex>& indices) {
    std::vector<std::size_t> values;
    values.reserve(indices.size());
    for (const SubtitleIndex index : indices)
        values.push_back(index.value());
    return values;
}

} // namespace

TEST_CASE("subtitles insert before the index they are given", "[model][project]") {
    Project project = projectOf({named("a"), named("d")});
    const std::array<Subtitle, 2> inserted = {named("b"), named("c")};

    project.insert(SubtitleIndex::fromValue(1), inserted);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "b", "c", "d"});
}

TEST_CASE("subtitles insert at the end when given the count", "[model][project]") {
    // One past the last is the append position, as it is for every sequence.
    // Refusing it would make appending the one case a caller has to special.
    Project project = projectOf({named("a")});
    const std::array<Subtitle, 1> inserted = {named("b")};

    project.insert(SubtitleIndex::fromValue(1), inserted);

    CHECK(textsOf(project) == std::vector<std::string>{"a", "b"});
}

TEST_CASE("inserting past the end is refused", "[model][project]") {
    Project project = projectOf({named("a")});
    const std::array<Subtitle, 1> inserted = {named("b")};

    CHECK_THROWS_AS(project.insert(SubtitleIndex::fromValue(2), inserted), std::out_of_range);
    CHECK(project.count() == 1);
}

TEST_CASE("inserting nothing leaves the project alone", "[model][project]") {
    Project project = projectOf({named("a")});

    project.insert(SubtitleIndex::fromValue(0), {});

    CHECK(textsOf(project) == std::vector<std::string>{"a"});
}

TEST_CASE("a discontinuous selection is removed without shifting indices", "[model][project]") {
    // The case that makes removal worth testing: taking out 0 and 2 must not
    // let the removal of the first shift the second onto its neighbour.
    Project project = projectOf({named("a"), named("b"), named("c"), named("d")});
    const std::array<SubtitleIndex, 2> targets = {SubtitleIndex::fromValue(0),
                                                  SubtitleIndex::fromValue(2)};

    const std::vector<Subtitle> removed = project.remove(Selection::of(targets));

    CHECK(textsOf(project) == std::vector<std::string>{"b", "d"});
    CHECK(textsOf(removed) == std::vector<std::string>{"a", "c"});
}

TEST_CASE("removal hands back the subtitles in index order", "[model][project]") {
    // Whatever order the caller wrote its indices in: that is what the inverse
    // command has to put back, and it puts it back by ascending index.
    Project project = projectOf({named("a"), named("b"), named("c")});
    const std::array<SubtitleIndex, 3> targets = {
        SubtitleIndex::fromValue(2), SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(1)};

    const std::vector<Subtitle> removed = project.remove(Selection::of(targets));

    CHECK(project.count() == 0);
    CHECK(textsOf(removed) == std::vector<std::string>{"a", "b", "c"});
}

TEST_CASE("removing an empty selection changes nothing", "[model][project]") {
    Project project = projectOf({named("a"), named("b")});

    const std::vector<Subtitle> removed = project.remove(Selection::of({}));

    CHECK(removed.empty());
    CHECK(textsOf(project) == std::vector<std::string>{"a", "b"});
}

TEST_CASE("removing an index the project does not have is refused", "[model][project]") {
    Project project = projectOf({named("a")});
    const std::array<SubtitleIndex, 1> targets = {SubtitleIndex::fromValue(1)};

    CHECK_THROWS_AS(project.remove(Selection::of(targets)), std::out_of_range);
    CHECK(project.count() == 1);
}

TEST_CASE("what removal hands back goes back where it came from", "[model][project]") {
    // The property the inverse command rests on, checked on the model alone:
    // re-inserting each subtitle at its original index restores the project.
    Project project = projectOf({named("a"), named("b"), named("c"), named("d")});
    const std::vector<std::string> before = textsOf(project);
    const std::array<SubtitleIndex, 2> targets = {SubtitleIndex::fromValue(1),
                                                  SubtitleIndex::fromValue(3)};

    const std::vector<Subtitle> removed = project.remove(Selection::of(targets));
    for (std::size_t position = 0; position < targets.size(); ++position)
        project.insert(targets[position], std::span{&removed[position], 1});

    CHECK(textsOf(project) == before);
}

TEST_CASE("a project in order reports no disorder", "[model][project]") {
    const Project project = projectOf({startingAt(0), startingAt(2000), startingAt(4000)});

    CHECK(project.outOfOrder().empty());
}

TEST_CASE("two subtitles starting at the same moment are in order", "[model][project]") {
    // Equal starts are odd, not disordered: neither precedes the other, and
    // reporting them would flood the list on files that stack captions.
    const Project project = projectOf({startingAt(1000), startingAt(1000)});

    CHECK(project.outOfOrder().empty());
}

TEST_CASE("a project names the subtitles that start too early", "[model][project]") {
    // The index reported is the one that breaks the order, not the one before
    // it: that is the line the interface has to show.
    const Project project =
        projectOf({startingAt(0), startingAt(4000), startingAt(2000), startingAt(6000)});

    CHECK(valuesOf(project.outOfOrder()) == std::vector<std::size_t>{2});
}

TEST_CASE("a reversed project reports every subtitle but the first", "[model][project]") {
    const Project project = projectOf({startingAt(4000), startingAt(2000), startingAt(0)});

    CHECK(valuesOf(project.outOfOrder()) == std::vector<std::size_t>{1, 2});
}

TEST_CASE("an empty project reports no disorder", "[model][project]") {
    CHECK(projectOf({}).outOfOrder().empty());
}
