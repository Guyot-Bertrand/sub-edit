// The other bound of the timeline — issue #174.
//
// The core has known the bound at zero since #132: no subtitle file can hold a
// negative position, so a shift that would make one is refused. This one is
// the opposite in every way. It is not a refusal but a notice (D4): a
// subtitle after the closing credits may be exactly what the user meant, and a
// refusal that is wrong costs more than a warning that is ignored.

#include <subedit/core/edit/video_bounds.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace {

using subedit::core::BeyondEnd;
using subedit::core::beyondEnd;
using subedit::core::Duration;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle from(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = "x"};
}

[[nodiscard]] Project holding(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

[[nodiscard]] Duration seconds(std::int64_t count) {
    return Duration::fromMilliseconds(count * 1000);
}

} // namespace

// The promise the whole phase rests on: without `ffprobe`, without a video,
// without a duration, nothing changes and nothing is said. The absence of a
// player is the ordinary state of the program, not a degraded one.
TEST_CASE("with no duration known, nothing is said", "[video][bounds]") {
    const Project project = holding({from(1000, 2000), from(3000, 90000)});

    CHECK_FALSE(beyondEnd(project, Selection::all(project), std::nullopt).has_value());
}

TEST_CASE("a document that fits inside the video says nothing", "[video][bounds]") {
    const Project project = holding({from(1000, 2000), from(3000, 4000)});

    CHECK_FALSE(beyondEnd(project, Selection::all(project), seconds(10)).has_value());
}

// The bound is the end of the video, and landing on it is landing inside.
// Refusing the exact end would turn a bound into a prohibition — the same
// reading the bound at zero already takes.
TEST_CASE("a subtitle ending exactly with the video is inside it", "[video][bounds]") {
    const Project project = holding({from(8000, 10000)});

    CHECK_FALSE(beyondEnd(project, Selection::all(project), seconds(10)).has_value());
}

TEST_CASE("one subtitle past the end is counted, and measured", "[video][bounds]") {
    const Project project = holding({from(1000, 2000), from(9000, 12500)});

    CHECK(beyondEnd(project, Selection::all(project), seconds(10)) ==
          BeyondEnd{.count = 1, .overshoot = Duration::fromMilliseconds(2500)});
}

// « How many overshoot, and by how much the last one does » — and the last is
// the one that ends latest, not the one that comes last in the file. A file
// out of order is an ordinary thing to be handed, and what the user needs to
// know is how far past the end the document reaches.
TEST_CASE("several past the end are counted, and the furthest is measured", "[video][bounds]") {
    const Project project = holding({from(9000, 11000), from(30000, 40000), from(9500, 12000)});

    CHECK(beyondEnd(project, Selection::all(project), seconds(10)) ==
          BeyondEnd{.count = 3, .overshoot = seconds(30)});
}

// The notice belongs to the operation that was just applied, so it looks at
// what that operation touched. A subtitle nobody moved, already past the end
// because the wrong film is associated, is not this operation's doing.
TEST_CASE("only the subtitles the operation touched are looked at", "[video][bounds]") {
    const Project project = holding({from(1000, 2000), from(30000, 40000)});

    const std::vector<SubtitleIndex> only{SubtitleIndex::fromNumber(1)};
    const Selection first = Selection::of(only);
    CHECK_FALSE(beyondEnd(project, first, seconds(10)).has_value());
}

// A video shorter than nothing is not a video. It says so rather than
// answering for a film of negative length.
TEST_CASE("a duration of zero or less is no duration at all", "[video][bounds]") {
    const Project project = holding({from(1000, 2000)});

    CHECK_FALSE(beyondEnd(project, Selection::all(project), Duration::zero()).has_value());
    CHECK_FALSE(beyondEnd(project, Selection::all(project), seconds(-5)).has_value());
}
