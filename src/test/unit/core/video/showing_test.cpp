// Which subtitle is on screen at a given moment — the question the window asks
// ten times a second while a film plays.

#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/showing.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <vector>

namespace {

using subedit::core::Project;
using subedit::core::showingAt;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle from(int start, int end, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(start),
        .end = Timestamp::fromMilliseconds(end),
        .mainText = std::move(text),
    };
}

/// Three subtitles with a gap between the second and the third.
[[nodiscard]] Project three() {
    Project project;
    project.setSubtitles({
        from(1000, 2000, "Un."),
        from(2000, 3000, "Deux."),
        from(5000, 6000, "Trois."),
    });
    return project;
}

[[nodiscard]] std::optional<std::size_t> numberShowingAt(const Project& project, int milliseconds) {
    const std::optional<SubtitleIndex> showing =
        showingAt(project, Timestamp::fromMilliseconds(milliseconds));
    return showing.has_value() ? std::optional{showing->number()} : std::nullopt;
}

} // namespace

TEST_CASE("a position inside a subtitle names it", "[video][showing]") {
    const Project project = three();

    CHECK(numberShowingAt(project, 1500) == 1U);
    CHECK(numberShowingAt(project, 2500) == 2U);
    CHECK(numberShowingAt(project, 5500) == 3U);
}

// The same reading as `beyondEnd`, which counts a subtitle ending exactly with
// the video as inside it. A bound that excluded its own edge would make the
// last millisecond of every subtitle a hole.
TEST_CASE("both ends of a subtitle are inside it", "[video][showing]") {
    Project project;
    project.setSubtitles({from(1000, 2000, "Un.")});

    CHECK(numberShowingAt(project, 1000) == 1U);
    CHECK(numberShowingAt(project, 2000) == 1U);
}

TEST_CASE("a position between two subtitles names none", "[video][showing]") {
    const Project project = three();

    CHECK_FALSE(numberShowingAt(project, 4000).has_value());
    CHECK_FALSE(numberShowingAt(project, 500).has_value());
    CHECK_FALSE(numberShowingAt(project, 9000).has_value());
}

TEST_CASE("a document with nothing in it shows nothing", "[video][showing]") {
    const Project project;

    CHECK_FALSE(numberShowingAt(project, 1000).has_value());
}

// Two speakers, or a document nobody has cleaned. The one written last is the
// one a player draws over the other, which is how Gaupol reads it too.
TEST_CASE("where two subtitles overlap, the later one shows", "[video][showing]") {
    Project project;
    project.setSubtitles({
        from(1000, 4000, "Un."),
        from(2000, 3000, "Deux."),
    });

    CHECK(numberShowingAt(project, 2500) == 2U);
    // Outside the second, the first is back on its own.
    CHECK(numberShowingAt(project, 3500) == 1U);
}

// A project out of order is a state this model holds on purpose — ADR 0008 —
// and the answer must not depend on the order the file happened to be in.
TEST_CASE("a document out of order is read all the same", "[video][showing]") {
    Project project;
    project.setSubtitles({
        from(5000, 6000, "Trois."),
        from(1000, 2000, "Un."),
    });

    CHECK(numberShowingAt(project, 1500) == 2U);
    CHECK(numberShowingAt(project, 5500) == 1U);
}

// Nothing forbids a subtitle whose end precedes its start — ADR 0008 again —
// and nothing is ever inside one.
TEST_CASE("a subtitle that ends before it starts shows never", "[video][showing]") {
    Project project;
    project.setSubtitles({from(3000, 1000, "À l'envers.")});

    CHECK_FALSE(numberShowingAt(project, 2000).has_value());
}
