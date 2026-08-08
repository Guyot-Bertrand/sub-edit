#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;

[[nodiscard]] Project projectOf(std::size_t count) {
    Project project;
    project.setSubtitles(std::vector<Subtitle>(count));
    return project;
}

[[nodiscard]] std::vector<std::size_t> valuesOf(const Selection& selection) {
    std::vector<std::size_t> values;
    for (const SubtitleIndex index : selection.indices())
        values.push_back(index.value());
    return values;
}

} // namespace

TEST_CASE("a selection of everything covers the project in order", "[model][selection]") {
    const Project project = projectOf(3);

    const Selection selection = Selection::all(project);

    CHECK(selection.count() == 3);
    CHECK(valuesOf(selection) == std::vector<std::size_t>{0, 1, 2});
}

TEST_CASE("a selection of everything in an empty project is empty", "[model][selection]") {
    // Not a special case to guard against at each call site: the empty
    // selection is a selection, and an operation over it changes nothing.
    const Selection selection = Selection::all(projectOf(0));

    CHECK(selection.isEmpty());
    CHECK(selection.count() == 0);
}

TEST_CASE("a selection sorts and deduplicates what it is given", "[model][selection]") {
    // By construction, which is what insertion and removal need to be right:
    // neither has to wonder what order it was handed.
    const std::array<SubtitleIndex, 5> given = {
        SubtitleIndex::fromValue(4),
        SubtitleIndex::fromValue(1),
        SubtitleIndex::fromValue(4),
        SubtitleIndex::fromValue(0),
        SubtitleIndex::fromValue(1),
    };

    const Selection selection = Selection::of(given);

    CHECK(valuesOf(selection) == std::vector<std::size_t>{0, 1, 4});
}

TEST_CASE("a range covers both of its bounds", "[model][selection]") {
    const Selection selection =
        Selection::range(SubtitleIndex::fromValue(2), SubtitleIndex::fromValue(5));

    CHECK(valuesOf(selection) == std::vector<std::size_t>{2, 3, 4, 5});
}

TEST_CASE("a range of one index holds that index", "[model][selection]") {
    const Selection selection =
        Selection::range(SubtitleIndex::fromValue(3), SubtitleIndex::fromValue(3));

    CHECK(valuesOf(selection) == std::vector<std::size_t>{3});
}

TEST_CASE("a range whose bounds are the wrong way round is empty", "[model][selection]") {
    // Empty rather than reversed or refused: the caller asked for what lies
    // between two bounds, and nothing does.
    const Selection selection =
        Selection::range(SubtitleIndex::fromValue(5), SubtitleIndex::fromValue(2));

    CHECK(selection.isEmpty());
}

TEST_CASE("a selection says whether it holds an index", "[model][selection]") {
    const std::array<SubtitleIndex, 2> given = {SubtitleIndex::fromValue(1),
                                                SubtitleIndex::fromValue(4)};

    const Selection selection = Selection::of(given);

    CHECK(selection.contains(SubtitleIndex::fromValue(1)));
    CHECK(selection.contains(SubtitleIndex::fromValue(4)));
    CHECK_FALSE(selection.contains(SubtitleIndex::fromValue(0)));
    CHECK_FALSE(selection.contains(SubtitleIndex::fromValue(2)));
}

TEST_CASE("an empty selection holds nothing", "[model][selection]") {
    const Selection selection = Selection::of({});

    CHECK(selection.isEmpty());
    CHECK_FALSE(selection.contains(SubtitleIndex::fromValue(0)));
}
