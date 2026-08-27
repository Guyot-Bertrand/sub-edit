#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

using subedit::core::Anomaly;
using subedit::core::AnomalyKind;
using subedit::core::Project;
using subedit::core::scanAnomalies;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle spanning(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(end)};
}

[[nodiscard]] Project projectOf(std::vector<Subtitle> subtitles) {
    Project project;
    project.setSubtitles(std::move(subtitles));
    return project;
}

[[nodiscard]] Anomaly at(AnomalyKind kind, std::size_t value) {
    return Anomaly{.kind = kind, .index = SubtitleIndex::fromValue(value)};
}

/// Builds a subtitle starting at `start`, one second long.
[[nodiscard]] Subtitle startingAt(std::int64_t start) {
    return spanning(start, start + 1000);
}

/// The subtitles reported out of order, and them alone.
///
/// Ordering tests read better without the overlaps that follow from moving a
/// subtitle before its predecessor — the overlap is real and reported, it is
/// simply not what these cases are about.
[[nodiscard]] std::vector<std::size_t> disorderedIn(const Project& project) {
    std::vector<std::size_t> values;
    for (const Anomaly& anomaly : scanAnomalies(project)) {
        if (anomaly.kind == AnomalyKind::OutOfOrder)
            values.push_back(anomaly.index.value());
    }
    return values;
}

} // namespace

TEST_CASE("a sound project has no anomaly", "[model][anomaly]") {
    const Project project = projectOf({spanning(0, 1000), spanning(2000, 3000)});

    CHECK(scanAnomalies(project).empty());
}

TEST_CASE("a subtitle that ends before it starts is an anomaly", "[model][anomaly]") {
    // The one kind that needs no predecessor: the first subtitle can carry it.
    const Project project = projectOf({spanning(2000, 1000)});

    CHECK(scanAnomalies(project) == std::vector<Anomaly>{at(AnomalyKind::EndBeforeStart, 0)});
}

TEST_CASE("a subtitle starting before the previous one ends overlaps it", "[model][anomaly]") {
    const Project project = projectOf({spanning(0, 2000), spanning(1000, 3000)});

    CHECK(scanAnomalies(project) == std::vector<Anomaly>{at(AnomalyKind::OverlappingSubtitles, 1)});
}

TEST_CASE("a subtitle starting before the previous one starts breaks the order",
          "[model][anomaly]") {
    const Project project = projectOf({spanning(5000, 6000), spanning(1000, 2000)});

    CHECK(scanAnomalies(project) == std::vector<Anomaly>{at(AnomalyKind::OverlappingSubtitles, 1),
                                                         at(AnomalyKind::OutOfOrder, 1)});
}

TEST_CASE("equal starts are disorder under neither reading", "[model][anomaly]") {
    // Neither precedes the other, so neither is out of place. The overlap is
    // reported all the same, because it is one.
    const Project project = projectOf({spanning(1000, 3000), spanning(1000, 4000)});

    CHECK(scanAnomalies(project) == std::vector<Anomaly>{at(AnomalyKind::OverlappingSubtitles, 1)});
}

TEST_CASE("anomalies come in the order of the subtitles they name", "[model][anomaly]") {
    // What a report and a table both need: walking the list once follows the
    // file, and no caller has to sort it.
    const Project project =
        projectOf({spanning(0, 1000), spanning(4000, 3000), spanning(2000, 5000)});

    const std::vector<Anomaly> found = scanAnomalies(project);

    REQUIRE(found.size() == 3);
    CHECK(found[0] == at(AnomalyKind::EndBeforeStart, 1));
    CHECK(found[1] == at(AnomalyKind::OverlappingSubtitles, 2));
    CHECK(found[2] == at(AnomalyKind::OutOfOrder, 2));
}

TEST_CASE("an empty project has no anomaly", "[model][anomaly]") {
    CHECK(scanAnomalies(Project{}).empty());
}

TEST_CASE("the subtitle named is the one out of place", "[model][anomaly]") {
    // Not the one it is out of place against: that is the row an interface has
    // to mark, and the subtitle a report has to name.
    const Project project =
        projectOf({startingAt(0), startingAt(4000), startingAt(2000), startingAt(6000)});

    CHECK(disorderedIn(project) == std::vector<std::size_t>{2});
}

TEST_CASE("a reversed project names every subtitle but the first", "[model][anomaly]") {
    const Project project = projectOf({startingAt(4000), startingAt(2000), startingAt(0)});

    CHECK(disorderedIn(project) == std::vector<std::size_t>{1, 2});
}

TEST_CASE("a subtitle that recovers the order breaks nothing", "[model][anomaly]") {
    // 3000 follows 2000, so it breaks nothing — though it does start before the
    // 4000 already seen. **This is where the two readings of disorder parted**,
    // and the retained one is this: name what breaks the order, because that is
    // the subtitle there is something to do about. The other named lines that
    // are already in their place. The corpus could not settle it — none of its
    // files is out of order at all — so it was settled by reasoning, and the
    // phase-5 spec says so rather than claim a measurement.
    const Project project =
        projectOf({startingAt(4000), startingAt(2000), startingAt(3000), startingAt(5000)});

    CHECK(disorderedIn(project) == std::vector<std::size_t>{1});
}
