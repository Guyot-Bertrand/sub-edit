// What a shift cannot do — issue #132.
//
// The rule lived in the command line, in the loop of `shiftAll`. The window
// needs the same one, and two copies of one rule drift apart — one has just
// been paid for with the vocabulary.

#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

using subedit::core::Duration;
using subedit::core::firstBeforeOrigin;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle at(std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = "x"};
}

/// Starts at one, three and five seconds.
[[nodiscard]] Project three() {
    Project project;
    project.setSubtitles({at(1000), at(3000), at(5000)});
    return project;
}

[[nodiscard]] Selection only(std::size_t value) {
    const SubtitleIndex index = SubtitleIndex::fromValue(value);
    return Selection::range(index, index);
}

} // namespace

TEST_CASE("a shift that keeps everything on the timeline is allowed", "[edit][shift]") {
    const Project project = three();

    CHECK_FALSE(
        firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(-1000))
            .has_value());
}

TEST_CASE("a shift that would take a subtitle before the origin names the first", "[edit][shift]") {
    // The first, and not any of them: it is the one the user has to look at to
    // understand by how much they were wrong.
    const Project project = three();

    // The whole option rather than its content: clang-tidy does not recognise
    // Catch2's REQUIRE as a check.
    CHECK(firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(-2000)) ==
          SubtitleIndex::fromNumber(1));
}

TEST_CASE("only the selected subtitles are looked at", "[edit][shift]") {
    // Shifting the third back by four seconds leaves it at one second; the
    // first would pass before the origin, but it is not being shifted.
    const Project project = three();

    CHECK_FALSE(firstBeforeOrigin(project, only(2), Duration::fromMilliseconds(-4000)).has_value());
    CHECK(firstBeforeOrigin(project, only(0), Duration::fromMilliseconds(-4000)).has_value());
}

TEST_CASE("landing exactly on the origin is allowed", "[edit][shift]") {
    // Zero is a position, and refusing it would turn the bound into a ban.
    const Project project = three();

    CHECK_FALSE(
        firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(-1000))
            .has_value());
}

TEST_CASE("a shift forward is never refused", "[edit][shift]") {
    const Project project = three();

    CHECK_FALSE(
        firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(3600000))
            .has_value());
}

TEST_CASE("an empty selection has nothing to refuse", "[edit][shift]") {
    const Project project = three();

    CHECK_FALSE(firstBeforeOrigin(project, Selection::of({}), Duration::fromMilliseconds(-99999))
                    .has_value());
}
