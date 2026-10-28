// Putting the found places on a file (issue #812, phase 23).

#include <subedit/core/transcription/alignment_retiming.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

using subedit::core::AlignedLine;
using subedit::core::LineTimes;
using subedit::core::Timestamp;

Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

LineTimes times(std::int64_t start, std::int64_t end) {
    return {.start = at(start), .end = at(end)};
}

AlignedLine place(std::int64_t start, std::int64_t end) {
    return {.start = at(start), .end = at(end)};
}

} // namespace

TEST_CASE("a line found takes its place and a line not found keeps its times",
          "[alignment][retiming]") {
    const std::vector<LineTimes> originals{times(1000, 2000), times(4000, 5000), times(7000, 8000)};
    const std::vector<AlignedLine> places{place(3400, 4400), AlignedLine{}, place(9400, 10400)};

    const auto lines = subedit::core::retime(originals, places);

    REQUIRE(lines.size() == 3);
    CHECK(lines[0].moved);
    // It ran into the next line, which did not move: cut back to where that one begins.
    CHECK(lines[0].times == times(3400, 4000));
    CHECK(lines[0].trimmed);
    CHECK_FALSE(lines[1].moved);
    CHECK(lines[1].times == times(4000, 5000));
    CHECK(lines[2].times == times(9400, 10400));
}

TEST_CASE("a place with a start and no end is not a place", "[alignment][retiming]") {
    const std::vector<LineTimes> originals{times(1000, 2000)};
    const std::vector<AlignedLine> places{{.start = at(5000), .end = std::nullopt}};

    const auto lines = subedit::core::retime(originals, places);

    CHECK_FALSE(lines[0].moved);
    CHECK(lines[0].times == times(1000, 2000));
}

TEST_CASE("a line that would run into the next is cut back to it", "[alignment][retiming]") {
    const std::vector<LineTimes> originals{times(0, 1000), times(2000, 3000)};
    const std::vector<AlignedLine> places{place(1000, 4000), place(3000, 5000)};

    const auto lines = subedit::core::retime(originals, places);

    CHECK(lines[0].trimmed);
    CHECK(lines[0].times == times(1000, 3000));
    CHECK_FALSE(lines[0].overlapping);
    CHECK_FALSE(lines[1].trimmed);
}

TEST_CASE("an overlap that cutting cannot cure is said and left", "[alignment][retiming]") {
    // The second line, not found, keeps a time that is before where the first now begins.
    const std::vector<LineTimes> originals{times(0, 1000), times(1500, 2500)};
    const std::vector<AlignedLine> places{place(4000, 6000), AlignedLine{}};

    const auto lines = subedit::core::retime(originals, places);

    CHECK(lines[0].overlapping);
    CHECK_FALSE(lines[0].trimmed);
    CHECK(lines[0].times == times(4000, 6000));
    CHECK(lines[1].times == times(1500, 2500));
}

TEST_CASE("fewer places than lines leaves the rest alone, and no lines gives none",
          "[alignment][retiming]") {
    const std::vector<LineTimes> originals{times(0, 1000), times(2000, 3000)};

    const auto lines = subedit::core::retime(originals, std::vector<AlignedLine>{place(500, 900)});

    CHECK(lines[0].moved);
    CHECK_FALSE(lines[1].moved);
    CHECK(subedit::core::retime({}, {}).empty());
}
