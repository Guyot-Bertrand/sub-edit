#include <subedit/cli/destination.hpp>
#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::convertFrameRateAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::StandardFrameRate;

namespace {

const FrameRate kPal{StandardFrameRate::Fps25};
const FrameRate kNtscFilm{StandardFrameRate::Fps23976};

/// Two subtitles, both starting on a position ADR 0013 measured: 1010 ms and
/// 3600017 ms are exactly where going through the frames lands elsewhere.
const std::string kTwo = "1\n"
                         "00:00:01,010 --> 00:00:02,020\n"
                         "First.\n"
                         "\n"
                         "2\n"
                         "01:00:00,017 --> 01:00:02,000\n"
                         "Second.\n"
                         "\n";

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

Run convert(const std::string& content, FrameRate input, FrameRate output) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream errors;

    const ExitCode code = convertFrameRateAll(files,
                                              {"a.srt"},
                                              std::nullopt,
                                              input,
                                              output,
                                              Destination::from("", "out", false, 1).value(),
                                              Reporter{errors, 1});
    return {
        .code = code, .written = files.contentOf("out/a.srt").value_or(""), .errors = errors.str()};
}

} // namespace

TEST_CASE("every position of the file is retimed", "[cli][frame-rate]") {
    // Worked out by hand. The factor is the input over the output:
    // 25 ÷ 24000/1001 = 25025/24000 = 1001/960, so positions come later.
    //      1010 × 1001/960 = 1053.13… -> 1053
    //      2020 × 1001/960 = 2106.27… -> 2106
    //   3600017 × 1001/960 = 3753767.72… -> 3753768  (01:02:33,768)
    //   3602000 × 1001/960 = 3755835.41… -> 3755835  (01:02:35,835)
    const Run run = convert(kTwo, kPal, kNtscFilm);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:01,053 --> 00:00:02,106"));
    CHECK_THAT(run.written, ContainsSubstring("01:02:33,768 --> 01:02:35,835"));
}

TEST_CASE("the rounding happens once, and never on the frame grid", "[cli][frame-rate]") {
    // The whole reason ADR 0013 exists. Going through the frames —
    // fromFrame(toFrame(t, 25), 24000/1001) — quantises each position on the
    // input grid and answers 1043 and 3753750 instead. Both are off by more
    // than the millisecond, and neither may appear here.
    const Run run = convert(kTwo, kPal, kNtscFilm);

    CHECK_THAT(run.written, !ContainsSubstring("00:00:01,043"));
    CHECK_THAT(run.written, !ContainsSubstring("01:02:33,750"));
}

TEST_CASE("converting towards a faster rate brings the positions earlier", "[cli][frame-rate]") {
    // The other direction, and the semantics that go with it: a film mastered
    // at 23.976 and played at 25 runs faster, so its subtitles come earlier.
    //   1010 × (24000/1001) ÷ 25 = 1010 × 960/1001 = 968.63… -> 969
    const Run run = convert(kTwo, kNtscFilm, kPal);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:00,969 --> "));
}

TEST_CASE("converting between one rate and itself leaves every position alone",
          "[cli][frame-rate]") {
    const Run run = convert(kTwo, kPal, kPal);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:01,010 --> 00:00:02,020"));
    CHECK_THAT(run.written, ContainsSubstring("01:00:00,017 --> 01:00:02,000"));
}

TEST_CASE("the format of the file read is kept", "[cli][frame-rate]") {
    InMemoryFileSystem files;
    files.addFile("a.vtt", "WEBVTT\n\n00:01.010 --> 00:02.020\nOnly one.\n");
    std::ostringstream errors;

    CHECK(convertFrameRateAll(files,
                              {"a.vtt"},
                              std::nullopt,
                              kPal,
                              kNtscFilm,
                              Destination::from("", "out", false, 1).value(),
                              Reporter{errors, 0}) == ExitCode::Success);

    CHECK_THAT(files.contentOf("out/a.vtt").value_or(""), ContainsSubstring("WEBVTT"));
}

TEST_CASE("the narration says which two rates were used", "[cli][frame-rate]") {
    // The exact rate, not the word that named it: "23.976" is a label, and the
    // line that reports what happened has to report what happened.
    CHECK_THAT(convert(kTwo, kPal, kNtscFilm).errors,
               ContainsSubstring("a.srt: 2 subtitles retimed from 25 to 24000/1001 fps "
                                 "-> out/a.srt\n"));
}

TEST_CASE("a batch retimes what it can and counts the rest", "[cli][frame-rate]") {
    InMemoryFileSystem files;
    files.addFile("good.srt", kTwo);
    std::ostringstream errors;

    const ExitCode code = convertFrameRateAll(files,
                                              {"absent.srt", "good.srt"},
                                              std::nullopt,
                                              kPal,
                                              kNtscFilm,
                                              Destination::from("", "out", false, 2).value(),
                                              Reporter{errors, 1});

    CHECK(code == ExitCode::SomeFailed);
    CHECK(files.contentOf("out/good.srt").has_value());
}
