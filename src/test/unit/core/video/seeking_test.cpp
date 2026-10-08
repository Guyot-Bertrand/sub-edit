// Where the player is sent to, and what is selected or inserted from where it is — issue #644.
//
// These rules lived in `VideoPane` and `MpvPlayer`, where only a window or a film could try
// them. Here they take positions and give positions.

#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/seeking.hpp>
#include <subedit/core/wording/video.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace {

using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::Timestamp;

[[nodiscard]] Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

[[nodiscard]] Subtitle subtitle(std::int64_t start, std::int64_t end) {
    return Subtitle{.start = at(start), .end = at(end)};
}

/// Three subtitles, **out of order** on purpose: the second one starts first.
[[nodiscard]] std::vector<Subtitle> outOfOrder() {
    return {subtitle(5000, 6000), subtitle(1000, 2000), subtitle(9000, 10000)};
}

constexpr Duration kLength = Duration::fromMilliseconds(60000);

} // namespace

TEST_CASE("the next neighbour is the smallest start after the position, in any order",
          "[core][video][GUI-SEEK-03]") {
    const std::vector<Subtitle> subtitles = outOfOrder();

    // Not the first met after the position, which is the one at 9000.
    CHECK(subedit::core::neighbourStart(subtitles, at(3000), true) == at(5000));
    CHECK(subedit::core::neighbourStart(subtitles, at(9500), true) == std::nullopt);
}

TEST_CASE("the previous neighbour is the start of the last subtitle that ended before",
          "[core][video][GUI-SEEK-03]") {
    const std::vector<Subtitle> subtitles = outOfOrder();

    CHECK(subedit::core::neighbourStart(subtitles, at(7000), false) == at(5000));
    CHECK(subedit::core::neighbourStart(subtitles, at(500), false) == std::nullopt);
}

TEST_CASE("a position that is exactly a start is not its own neighbour",
          "[core][video][GUI-SEEK-03]") {
    const std::vector<Subtitle> subtitles = outOfOrder();

    // The margin is one millisecond, on both sides: at the start, the next is the one after.
    CHECK(subedit::core::neighbourStart(subtitles, at(5000), true) == at(9000));
    CHECK(subedit::core::neighbourStart(subtitles, at(4999), true) == at(9000));
    CHECK(subedit::core::neighbourStart(subtitles, at(4998), true) == at(5000));
    // And at an end, the previous is not the subtitle that ends there.
    CHECK(subedit::core::neighbourStart(subtitles, at(2000), false) == std::nullopt);
    CHECK(subedit::core::neighbourStart(subtitles, at(2002), false) == at(1000));
}

TEST_CASE("selecting from the position goes by the order of the file",
          "[core][video][GUI-MARK-04]") {
    const std::vector<Subtitle> subtitles = outOfOrder();

    // The first, in file order, that starts after 3000 is row 0 (5000), not the smallest start.
    CHECK(subedit::core::rowFrom(subtitles, at(3000), true) == std::size_t{0});
    // The last, in file order, that started before 6000 is row 1.
    CHECK(subedit::core::rowFrom(subtitles, at(6000), false) == std::size_t{1});
}

TEST_CASE("with none on that side, the end of the file on that side is selected",
          "[core][video][GUI-MARK-04]") {
    const std::vector<Subtitle> subtitles = outOfOrder();

    CHECK(subedit::core::rowFrom(subtitles, at(50000), true) == subtitles.size() - 1);
    CHECK(subedit::core::rowFrom(subtitles, at(0), false) == std::size_t{0});
    CHECK(subedit::core::rowFrom({}, at(0), true) == std::nullopt);
}

TEST_CASE(
    "an insertion goes after every subtitle that starts at or before, and ends in three seconds",
    "[core][video][GUI-MARK-03]") {
    const std::vector<Subtitle> subtitles = {subtitle(1000, 2000), subtitle(9000, 10000)};

    const subedit::core::Insertion plain = subedit::core::insertionAt(subtitles, at(3000));
    CHECK(plain.rank == 1);
    CHECK(plain.end == at(6000));

    // At the very start of one, it goes after it: « at or before ».
    CHECK(subedit::core::insertionAt(subtitles, at(1000)).rank == 1);
}

TEST_CASE("an insertion stops where the next subtitle starts", "[core][video][GUI-MARK-03]") {
    const std::vector<Subtitle> subtitles = {subtitle(1000, 2000), subtitle(4000, 5000)};

    const subedit::core::Insertion cut = subedit::core::insertionAt(subtitles, at(3000));
    CHECK(cut.rank == 1);
    CHECK(cut.end == at(4000));

    // None after: three seconds.
    CHECK(subedit::core::insertionAt(subtitles, at(7000)).end == at(10000));
    CHECK(subedit::core::insertionAt({}, at(0)).rank == 0);
}

TEST_CASE("a jump is kept inside the film", "[core][video][GUI-SEEK-01]") {
    CHECK(subedit::core::jumpedBy(at(40000), kLength, 30, 1) == at(60000));
    CHECK(subedit::core::jumpedBy(at(10000), kLength, 30, -1) == at(0));
    CHECK(subedit::core::jumpedBy(at(10000), kLength, 30, 1) == at(40000));
    CHECK(subedit::core::jumpedBy(at(40000), kLength, 30, -1) == at(10000));
}

TEST_CASE("an edge is taken back by the context length, and not before the film",
          "[core][video][GUI-SEEK-02]") {
    CHECK(subedit::core::withLeadIn(at(5000), 1500) == at(3500));
    CHECK(subedit::core::withLeadIn(at(1000), 1500) == at(0));
}

TEST_CASE("a step is exact, and stops at the last frame", "[core][video][GUI-STEP-04]") {
    const FrameRate fps25{StandardFrameRate::Fps25};
    const FrameRate fps24{StandardFrameRate::Fps23976};

    CHECK(subedit::core::steppedTo(at(1000), kLength, fps25, 1) == at(1040));
    CHECK(subedit::core::steppedTo(at(1000), kLength, fps25, -1) == at(960));
    // Never before the origin.
    CHECK(subedit::core::steppedTo(at(20), kLength, fps25, -1) == at(0));
    // The last frame of a film that is 60 s long starts one frame before: 59 960 ms.
    CHECK(subedit::core::steppedTo(at(59960), kLength, fps25, 1) == at(59960));
    CHECK(subedit::core::steppedTo(at(59000), kLength, fps25, 100) == at(59960));
    // At 23.976 a frame is 41.708 ms, and N of them are counted from the rational once.
    CHECK(subedit::core::steppedTo(at(0), kLength, fps24, 1000) == at(41708));
    // A film shorter than a frame has no later frame than its origin.
    CHECK(subedit::core::steppedTo(at(0), Duration::fromMilliseconds(10), fps25, 3) == at(0));
}

TEST_CASE("a reported rate is a standard one when it is near enough, else read to the thousandth",
          "[core][video][GUI-STEP-04]") {
    CHECK(subedit::core::frameRateNear(23.976023976) == FrameRate{StandardFrameRate::Fps23976});
    CHECK(subedit::core::frameRateNear(25.0) == FrameRate{StandardFrameRate::Fps25});
    CHECK(subedit::core::frameRateNear(59.94005994) == FrameRate{StandardFrameRate::Fps59940});

    // 12.5 is no standard rate: it is read to the thousandth, and that is 25 over 2.
    CHECK(subedit::core::frameRateNear(12.5) == FrameRate::create(25, 2));
}

TEST_CASE("a rate that is none is nothing", "[core][video][GUI-STEP-04]") {
    CHECK_FALSE(subedit::core::frameRateNear(0.0).has_value());
    CHECK_FALSE(subedit::core::frameRateNear(-25.0).has_value());
    CHECK_FALSE(subedit::core::frameRateNear(std::numeric_limits<double>::infinity()).has_value());
    CHECK_FALSE(subedit::core::frameRateNear(std::numeric_limits<double>::quiet_NaN()).has_value());
}

TEST_CASE("the number of a frame is rounded once, and the same for the bar and the table",
          "[core][video][GUI-FRAMES-02]") {
    const FrameRate fps25{StandardFrameRate::Fps25};
    CHECK(subedit::core::frameNumberText(at(2000), fps25) == "50");
    CHECK(subedit::core::frameNumberText(at(600000), FrameRate{StandardFrameRate::Fps23976}) ==
          "14386");
}
