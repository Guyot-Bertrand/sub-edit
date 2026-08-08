// Reference measurements for the three operations a reader or a writer runs
// once per subtitle. A file of several thousand subtitles goes through each of
// them twice, so they set the floor for the load and save benchmarks of the
// format modules.

#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/ratio.hpp>
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

// The two conversions a position really goes through, measured apart.
//
// `toFrame` and `fromFrame` stay: the frame-based formats of phase 9 and the
// video sync of phase 14 need them, and the round trip they form is exact in
// that direction. What left this file is the *frame-rate conversion* written as
// a round trip — the path ADR 0013 rejects. Scaling by an exact ratio replaced
// it, and is measured next to it so the cost of the decision is visible.
TEST_CASE("position conversions", "[benchmark]") {
    const subedit::core::FrameRate rate{subedit::core::StandardFrameRate::Fps23976};
    const subedit::core::FrameRate pal{subedit::core::StandardFrameRate::Fps25};
    const subedit::core::Timestamp position = subedit::core::Timestamp::fromMilliseconds(3661500);
    const subedit::core::Frame frame = position.toFrame(rate);
    const subedit::core::Ratio factor = pal.conversionTo(rate);

    BENCHMARK("position vers image") {
        return position.toFrame(rate);
    };

    BENCHMARK("image vers position") {
        return subedit::core::Timestamp::fromFrame(frame, rate);
    };

    BENCHMARK("mise à l'échelle par un rationnel exact") {
        return position.scaledBy(factor);
    };
}
