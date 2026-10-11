#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/join_repeats.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::CommandKind;
using subedit::core::joinRepeats;
using subedit::core::JoinReport;
using subedit::core::Project;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::core::WebVttExtras;

[[nodiscard]] Subtitle subtitle(std::int64_t start, std::int64_t end, std::string text) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end),
                    .mainText = std::move(text)};
}

[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

struct Row {
    std::int64_t start;
    std::int64_t end;
    std::string text;

    friend bool operator==(const Row&, const Row&) = default;
};

[[nodiscard]] std::vector<Row> rowsOf(const Project& project) {
    std::vector<Row> rows;
    for (const Subtitle& each : project.subtitles()) {
        rows.push_back(Row{.start = each.start.milliseconds(),
                           .end = each.end.milliseconds(),
                           .text = each.mainText});
    }
    return rows;
}

/// Joins the repeats of `project` and applies the command, if any.
JoinReport join(Project& project) {
    auto joined = joinRepeats(project);
    if (joined.command != nullptr)
        joined.command->apply(project);
    return joined.report;
}

} // namespace

TEST_CASE("a text cut in pieces that follow each other is one subtitle again", "[edit][join]") {
    Project project = projectOf({subtitle(1000, 2000, "Alpha"),
                                 subtitle(2000, 4000, "Alpha"),
                                 subtitle(4000, 6000, "Alpha")});

    const JoinReport report = join(project);

    CHECK(rowsOf(project) == std::vector<Row>{{1000, 6000, "Alpha"}});
    CHECK(report.chains == 1);
    CHECK(report.joined == 2);
}

TEST_CASE("it does not depend on the order of the file, and keeps the order it finds",
          "[edit][join]") {
    // The pieces of one text, with another text over the whole range, given
    // out of order: the third starts before the second.
    Project project = projectOf({subtitle(47240, 47520, "Alpha"),
                                 subtitle(47520, 50080, "Alpha"),
                                 subtitle(47240, 52000, "Beta"),
                                 subtitle(50080, 52000, "Alpha")});

    join(project);

    CHECK(rowsOf(project) == std::vector<Row>{{47240, 52000, "Alpha"}, {47240, 52000, "Beta"}});
    CHECK(project.isInOrder());
}

TEST_CASE("pieces that overlap are joined up to the furthest end", "[edit][join]") {
    Project project = projectOf({subtitle(0, 5000, "Alpha"), subtitle(3000, 4000, "Alpha")});

    join(project);

    CHECK(rowsOf(project) == std::vector<Row>{{0, 5000, "Alpha"}});
}

TEST_CASE("a gap, another text, another translation or other extras keep subtitles apart",
          "[edit][join]") {
    Subtitle translated = subtitle(2000, 4000, "Alpha");
    translated.translationText = "Un";
    Subtitle styled = subtitle(4000, 6000, "Alpha");
    styled.extras = WebVttExtras{.settings = "line:83%"};
    const Project project = projectOf({subtitle(0, 1000, "Alpha"),
                                       subtitle(1500, 2000, "Alpha"),
                                       translated,
                                       styled,
                                       subtitle(6000, 7000, "Beta")});

    const auto joined = joinRepeats(project);

    CHECK(joined.command == nullptr);
    CHECK(joined.report == JoinReport{});
}

TEST_CASE("an empty or backwards subtitle is never joined", "[edit][join]") {
    const Project project = projectOf({subtitle(0, 1000, ""),
                                       subtitle(1000, 2000, ""),
                                       subtitle(3000, 2000, "Alpha"),
                                       subtitle(3000, 4000, "Alpha")});

    CHECK(joinRepeats(project).command == nullptr);
}

TEST_CASE("a run keeps the subtitle that starts first and its extras", "[edit][join]") {
    Subtitle first = subtitle(0, 1000, "Alpha");
    first.extras = WebVttExtras{.settings = "line:95%"};
    Subtitle second = subtitle(1000, 2000, "Alpha");
    second.extras = WebVttExtras{.settings = "line:95%"};
    Project project = projectOf({second, first});

    join(project);

    REQUIRE(project.count() == 1);
    CHECK(project.subtitles()[0].start.milliseconds() == 0);
    CHECK(project.subtitles()[0].end.milliseconds() == 2000);
}

TEST_CASE("a second run finds nothing, and an undo gives the file back", "[edit][join]") {
    const std::vector<Subtitle> original{
        subtitle(0, 1000, "Alpha"), subtitle(1000, 2000, "Alpha"), subtitle(5000, 6000, "Beta")};
    Project project = projectOf(original);
    auto joined = joinRepeats(project);
    REQUIRE(joined.command != nullptr);
    CHECK(joined.command->kind() == CommandKind::JoinRepeats);

    joined.command->apply(project);
    CHECK(joinRepeats(project).command == nullptr);

    joined.command->revert(project);
    REQUIRE(project.count() == 3);
    CHECK(project.subtitles()[1] == original[1]);
    CHECK(project.subtitles()[0].end.milliseconds() == 1000);
}
