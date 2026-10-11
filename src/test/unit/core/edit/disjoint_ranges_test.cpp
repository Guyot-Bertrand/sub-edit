#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/disjoint_ranges.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::AnomalyKind;
using subedit::core::CommandKind;
using subedit::core::DisjointRefusal;
using subedit::core::DisjointReport;
using subedit::core::DisjointSettings;
using subedit::core::makeDisjoint;
using subedit::core::Project;
using subedit::core::scanAnomalies;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

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

/// What the settings say when there is nothing to absorb and nothing to cut in
/// windows: the cuts alone.
[[nodiscard]] DisjointSettings plain() {
    return DisjointSettings{
        .shortest = subedit::core::Duration::fromMilliseconds(0), .mostLines = 0, .dashes = false};
}

/// Makes `project` disjoint, and returns what was asked of it.
DisjointReport run(Project& project, const DisjointSettings& settings) {
    auto made = makeDisjoint(project, settings);
    REQUIRE(made.has_value());
    if (made->command != nullptr)
        made->command->apply(project);
    return made->report;
}

struct Row {
    std::int64_t start;
    std::int64_t end;
    std::string text;

    friend bool operator==(const Row&, const Row&) = default;
};

[[nodiscard]] std::vector<Row> rowsOf(const Project& project) {
    std::vector<Row> rows;
    for (const Subtitle& subtitle : project.subtitles()) {
        rows.push_back(Row{.start = subtitle.start.milliseconds(),
                           .end = subtitle.end.milliseconds(),
                           .text = subtitle.mainText});
    }
    return rows;
}

[[nodiscard]] bool anyOverlap(const Project& project) {
    return std::ranges::any_of(scanAnomalies(project), [](const auto& anomaly) {
        return anomaly.kind == AnomalyKind::OverlappingSubtitles;
    });
}

} // namespace

TEST_CASE("two overlapping subtitles become three disjoint ones", "[edit][disjoint]") {
    // A [0, 5] and B [3, 8]: nothing is shortened or extended.
    Project project = projectOf({subtitle(0, 5000, "A"), subtitle(3000, 8000, "B")});

    const DisjointReport report = run(project, plain());

    CHECK(rowsOf(project) ==
          std::vector<Row>{{0, 3000, "A"}, {3000, 5000, "A\nB"}, {5000, 8000, "B"}});
    CHECK(report.groups == 1);
    CHECK(report.replaced == 2);
    CHECK(report.produced == 3);
    CHECK(report.split == 2);
    CHECK(report.merged == 1);
    CHECK_FALSE(anyOverlap(project));
}

TEST_CASE("subtitles that overlap nothing are left alone", "[edit][disjoint]") {
    const Project project =
        projectOf({subtitle(0, 1000, "A"), subtitle(1000, 2000, "B"), subtitle(3000, 4000, "C")});

    const auto made = makeDisjoint(project, plain());

    REQUIRE(made.has_value());
    CHECK(made->command == nullptr);
    CHECK(made->report == DisjointReport{});
}

TEST_CASE("only the groups are replaced, the rest keeps its place", "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 1000, "before"),
                                 subtitle(2000, 5000, "A"),
                                 subtitle(4000, 6000, "B"),
                                 subtitle(7000, 8000, "between"),
                                 subtitle(9000, 12000, "C"),
                                 subtitle(10000, 11000, "D"),
                                 subtitle(13000, 14000, "after")});

    const DisjointReport report = run(project, plain());

    CHECK(report.groups == 2);
    CHECK(rowsOf(project) == std::vector<Row>{{0, 1000, "before"},
                                              {2000, 4000, "A"},
                                              {4000, 5000, "A\nB"},
                                              {5000, 6000, "B"},
                                              {7000, 8000, "between"},
                                              {9000, 10000, "C"},
                                              {10000, 11000, "C\nD"},
                                              {11000, 12000, "C"},
                                              {13000, 14000, "after"}});
}

TEST_CASE("a second run finds nothing to do, and an undo gives the file back", "[edit][disjoint]") {
    const std::vector<Subtitle> original{subtitle(0, 5000, "A"), subtitle(3000, 8000, "B")};
    Project project = projectOf(original);
    auto made = makeDisjoint(project, plain());
    REQUIRE(made.has_value());
    REQUIRE(made->command != nullptr);
    CHECK(made->command->kind() == CommandKind::MakeDisjoint);

    made->command->apply(project);
    const auto again = makeDisjoint(project, plain());
    REQUIRE(again.has_value());
    CHECK(again->command == nullptr);

    made->command->revert(project);
    CHECK(project.subtitles().size() == 2);
    CHECK(std::ranges::equal(project.subtitles(), original));
}

TEST_CASE("a line is never duplicated", "[edit][disjoint]") {
    // The same text on both: one line in the overlap, and the neighbouring pieces
    // that end up the same are one subtitle again.
    Project project = projectOf({subtitle(0, 5000, "Same"), subtitle(3000, 8000, "Same")});

    run(project, plain());

    CHECK(rowsOf(project) == std::vector<Row>{{0, 8000, "Same"}});
}

TEST_CASE("a subtitle inside another cuts it in three", "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 10000, "Long"), subtitle(3000, 5000, "Inside")});

    const DisjointReport report = run(project, plain());

    CHECK(rowsOf(project) ==
          std::vector<Row>{{0, 3000, "Long"}, {3000, 5000, "Long\nInside"}, {5000, 10000, "Long"}});
    CHECK(report.split == 1);
}

TEST_CASE("equal starts are merged line by line, in the order of the file", "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 5000, "First"), subtitle(0, 3000, "Second")});

    run(project, plain());

    CHECK(rowsOf(project) == std::vector<Row>{{0, 3000, "First\nSecond"}, {3000, 5000, "First"}});
}

TEST_CASE("three subtitles overlapping each other", "[edit][disjoint]") {
    Project project =
        projectOf({subtitle(0, 6000, "A"), subtitle(2000, 8000, "B"), subtitle(4000, 10000, "C")});

    run(project, plain());

    CHECK(rowsOf(project) == std::vector<Row>{{0, 2000, "A"},
                                              {2000, 4000, "A\nB"},
                                              {4000, 6000, "A\nB\nC"},
                                              {6000, 8000, "B\nC"},
                                              {8000, 10000, "C"}});
}

TEST_CASE("a very short interval is taken over by the neighbour that holds it",
          "[edit][disjoint]") {
    // [2000, 2050] shows A and B; the interval after it shows A, B and D, so it
    // is the one that stretches back over it.
    Project project = projectOf(
        {subtitle(0, 10000, "A"), subtitle(2000, 10000, "B"), subtitle(2050, 10000, "D")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(80), .mostLines = 0, .dashes = false};

    const DisjointReport report = run(project, settings);

    CHECK(rowsOf(project) == std::vector<Row>{{0, 2000, "A"}, {2000, 10000, "A\nB\nD"}});
    CHECK(report.absorbed == 1);
}

TEST_CASE("a very short interval no neighbour holds goes to the longer one", "[edit][disjoint]") {
    // [4950, 5000] shows A and B, which neither neighbour shows.
    Project project = projectOf({subtitle(0, 5000, "A"), subtitle(4950, 8000, "B")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(80), .mostLines = 0, .dashes = false};

    const DisjointReport report = run(project, settings);

    CHECK(rowsOf(project) == std::vector<Row>{{0, 5000, "A"}, {5000, 8000, "B"}});
    CHECK(report.absorbed == 1);
    CHECK_FALSE(anyOverlap(project));
}

TEST_CASE("the first and the last interval are absorbed by their only neighbour",
          "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 5000, "A"), subtitle(40, 9000, "B")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(80), .mostLines = 0, .dashes = false};

    run(project, settings);
    CHECK(rowsOf(project) == std::vector<Row>{{0, 5000, "A\nB"}, {5000, 9000, "B"}});

    Project last = projectOf({subtitle(0, 9000, "A"), subtitle(4000, 9040, "B")});
    run(last, settings);
    CHECK(rowsOf(last) == std::vector<Row>{{0, 4000, "A"}, {4000, 9040, "A\nB"}});
}

TEST_CASE("a whole subtitle shorter than the limit is never absorbed", "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 5000, "A"), subtitle(1000, 1040, "Blink")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(80), .mostLines = 0, .dashes = false};

    const DisjointReport report = run(project, settings);

    CHECK(rowsOf(project) ==
          std::vector<Row>{{0, 1000, "A"}, {1000, 1040, "A\nBlink"}, {1040, 5000, "A"}});
    CHECK(report.tooShort == 1);
    CHECK(report.absorbed == 0);
}

TEST_CASE("more lines than allowed are shown in windows", "[edit][disjoint]") {
    // Three lines on a three-second interval: the first two for the first half,
    // the last two for the second.
    Project project =
        projectOf({subtitle(0, 3000, "A"), subtitle(0, 3000, "B"), subtitle(0, 3000, "C")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(0), .mostLines = 2, .dashes = false};

    const DisjointReport report = run(project, settings);

    CHECK(rowsOf(project) == std::vector<Row>{{0, 1500, "A\nB"}, {1500, 3000, "B\nC"}});
    CHECK(report.windowed == 1);
    CHECK(report.merged == 1);
}

TEST_CASE("windows slide one line at a time over four lines", "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 3000, "A"),
                                 subtitle(0, 3000, "B"),
                                 subtitle(0, 3000, "C"),
                                 subtitle(0, 3000, "D")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(0), .mostLines = 2, .dashes = false};

    run(project, settings);

    CHECK(rowsOf(project) ==
          std::vector<Row>{{0, 1000, "A\nB"}, {1000, 2000, "B\nC"}, {2000, 3000, "C\nD"}});
}

TEST_CASE("the windows share the milliseconds the division leaves", "[edit][disjoint]") {
    Project project =
        projectOf({subtitle(0, 1000, "A"), subtitle(0, 1000, "B"), subtitle(0, 1000, "C")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(0), .mostLines = 2, .dashes = false};
    run(project, settings);
    CHECK(rowsOf(project) == std::vector<Row>{{0, 500, "A\nB"}, {500, 1000, "B\nC"}});

    Project odd =
        projectOf({subtitle(0, 1001, "A"), subtitle(0, 1001, "B"), subtitle(0, 1001, "C")});
    run(odd, settings);
    CHECK(rowsOf(odd) == std::vector<Row>{{0, 500, "A\nB"}, {500, 1001, "B\nC"}});
}

TEST_CASE("dashes mark the lines of a merged subtitle", "[edit][disjoint]") {
    Project project = projectOf({subtitle(0, 5000, "A"), subtitle(3000, 8000, "- B")});
    const DisjointSettings settings{
        .shortest = subedit::core::Duration::fromMilliseconds(0), .mostLines = 0, .dashes = true};

    run(project, settings);

    CHECK(rowsOf(project) ==
          std::vector<Row>{{0, 3000, "A"}, {3000, 5000, "- A\n- B"}, {5000, 8000, "- B"}});
}

TEST_CASE("the translation follows the same cuts", "[edit][disjoint]") {
    Subtitle first = subtitle(0, 5000, "A");
    first.translationText = "a";
    Subtitle second = subtitle(3000, 8000, "B");
    second.translationText = "b";
    Project project = projectOf({first, second});

    run(project, plain());

    REQUIRE(project.subtitles().size() == 3);
    CHECK(project.subtitles()[0].translationText == "a");
    CHECK(project.subtitles()[1].translationText == "a\nb");
    CHECK(project.subtitles()[2].translationText == "b");
}

TEST_CASE("a subtitle ending before it starts is reported and left", "[edit][disjoint]") {
    const Project project = projectOf(
        {subtitle(0, 5000, "A"), subtitle(6000, 2000, "Backwards"), subtitle(7000, 8000, "B")});

    const auto made = makeDisjoint(project, plain());

    REQUIRE(made.has_value());
    CHECK(made->command == nullptr);
    CHECK(made->report.endingBeforeStart == 1);
}

TEST_CASE("subtitles out of order are refused", "[edit][disjoint]") {
    const Project project = projectOf({subtitle(5000, 6000, "B"), subtitle(0, 1000, "A")});

    const auto made = makeDisjoint(project, plain());

    REQUIRE_FALSE(made.has_value());
    CHECK(made.error() == DisjointRefusal::NotInOrder);
}

TEST_CASE("the texts shown at every instant are the same before and after", "[edit][disjoint]") {
    // A property, on overlaps made by a small generator: whatever the starts and
    // ends, the lines on screen at any instant are the same, said once each.
    std::uint32_t seed = 12345;
    const auto next = [&seed](std::uint32_t bound) {
        seed = (seed * 1664525U) + 1013904223U;
        return (seed >> 8U) % bound;
    };

    for (int round = 0; round < 60; ++round) {
        std::vector<Subtitle> subtitles;
        std::int64_t start = 0;
        const std::uint32_t count = 2 + next(7);
        for (std::uint32_t at = 0; at < count; ++at) {
            start += static_cast<std::int64_t>(next(4)) * 500;
            const std::int64_t length = 500 + (static_cast<std::int64_t>(next(8)) * 500);
            subtitles.push_back(subtitle(start, start + length, std::string{char('A' + next(4))}));
        }
        Project project = projectOf(subtitles);
        INFO("round " << round);

        const auto shown = [](const std::vector<Subtitle>& rows, std::int64_t instant) {
            std::vector<std::string> lines;
            for (const Subtitle& row : rows) {
                if (row.start.milliseconds() > instant || row.end.milliseconds() <= instant)
                    continue;
                if (std::ranges::find(lines, row.mainText) == lines.end() &&
                    !row.mainText.empty()) {
                    // A merged text holds several lines already: split them.
                    std::size_t from = 0;
                    while (true) {
                        const std::size_t at = row.mainText.find('\n', from);
                        const std::string line = row.mainText.substr(from, at - from);
                        if (std::ranges::find(lines, line) == lines.end())
                            lines.push_back(line);
                        if (at == std::string::npos)
                            break;
                        from = at + 1;
                    }
                }
            }
            return lines;
        };

        run(project, plain());

        CHECK_FALSE(anyOverlap(project));
        const std::vector<Subtitle> after(project.subtitles().begin(), project.subtitles().end());
        for (std::int64_t instant = 0; instant < start + 5000; instant += 250)
            CHECK(shown(after, instant) == shown(subtitles, instant));

        const auto again = makeDisjoint(project, plain());
        REQUIRE(again.has_value());
        CHECK(again->command == nullptr);
    }
}

TEST_CASE("the rows written are told, as they stand once the operation is applied",
          "[edit][disjoint]") {
    // A group of two becomes three, which moves the second group down by one.
    Project project = projectOf({subtitle(0, 1000, "keep"),
                                 subtitle(2000, 5000, "A"),
                                 subtitle(4000, 6000, "B"),
                                 subtitle(7000, 8000, "keep"),
                                 subtitle(9000, 12000, "C"),
                                 subtitle(10000, 11000, "D")});
    auto made = makeDisjoint(project, plain());
    REQUIRE(made.has_value());
    const subedit::core::IndexRange nowhere{.first = subedit::core::SubtitleIndex::fromValue(0),
                                            .last = subedit::core::SubtitleIndex::fromValue(0)};
    REQUIRE(made->touched.has_value());
    const subedit::core::IndexRange touched = made->touched.value_or(nowhere);
    made->command->apply(project);

    CHECK(touched.first.value() == 1);
    // Two groups of three rows each: rows 1 to 3, then 5 to 7.
    CHECK(touched.last.value() == 7);
    CHECK(project.subtitles()[7].mainText == "C");
    CHECK(project.count() == 8);

    const Project clean = projectOf({subtitle(0, 1000, "A")});
    const auto none = makeDisjoint(clean, plain());
    REQUIRE(none.has_value());
    CHECK_FALSE(none->touched.has_value());
}

TEST_CASE("a window that shows what its neighbour shows is one subtitle with it",
          "[edit][disjoint]") {
    // Three lines at 3.5 s to 4 s are shown in two windows; the first one shows
    // what the interval before it showed, the second what the one after it shows.
    Project project = projectOf({subtitle(1000, 4000, "Hello."),
                                 subtitle(3000, 6000, "Hi."),
                                 subtitle(3500, 5000, "Yes?")});
    const DisjointSettings defaults{};

    const DisjointReport report = run(project, defaults);

    CHECK(rowsOf(project) == std::vector<Row>{{1000, 3000, "Hello."},
                                              {3000, 3750, "Hello.\nHi."},
                                              {3750, 5000, "Hi.\nYes?"},
                                              {5000, 6000, "Hi."}});
    CHECK(report.windowed == 1);
    CHECK(report.produced == 4);
    CHECK(report.split == 3);
}
