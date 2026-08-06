// Reference measurements for the three operations a reader or a writer runs
// once per subtitle. A file of several thousand subtitles goes through each of
// them twice, so they set the floor for the load and save benchmarks of the
// format modules.

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("timestamp parsing and formatting", "[benchmark]") {
    const subedit::core::Timestamp position = subedit::core::Timestamp::fromMilliseconds(3661500);

    BENCHMARK("parse") {
        return subedit::core::Timestamp::parse("01:01:01,500");
    };

    BENCHMARK("format") {
        return position.format(subedit::core::DecimalMark::Comma);
    };
}

TEST_CASE("frame rate conversion", "[benchmark]") {
    const subedit::core::FrameRate rate{subedit::core::StandardFrameRate::Fps23976};
    const subedit::core::Timestamp position = subedit::core::Timestamp::fromMilliseconds(3661500);

    BENCHMARK("to frame") {
        return position.toFrame(rate);
    };

    BENCHMARK("round trip through frames") {
        return subedit::core::Timestamp::fromFrame(position.toFrame(rate), rate);
    };
}
