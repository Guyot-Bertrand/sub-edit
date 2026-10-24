// How far a transcription is: the lines the tool prints, the time left, the range asked for,
// and the words that say it.

#include <subedit/core/transcription/remaining_time.hpp>
#include <subedit/core/transcription/time_range.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <optional>

namespace {

using namespace std::chrono_literals;
using subedit::core::RemainingTime;
using subedit::core::Timestamp;

const RemainingTime::Clock::time_point kOrigin{};

[[nodiscard]] Timestamp at(std::int64_t milliseconds) {
    return Timestamp::fromMilliseconds(milliseconds);
}

} // namespace

// ## The lines of the tool

TEST_CASE("a segment line gives its two moments and its text", "[whisper][progress]") {
    const auto line = subedit::core::readWhisperSegmentLine(
        "[00:01:00.000 --> 00:01:17.500]   What your country can do for you");

    REQUIRE(line.has_value());
    CHECK(line.transform([](const auto& read) { return read.start; }) == at(60000));
    CHECK(line.transform([](const auto& read) { return read.end; }) == at(77500));
    CHECK(line.transform([](const auto& read) { return read.text; }) ==
          "What your country can do for you");
}

TEST_CASE("a line that is no segment says nothing", "[whisper][progress]") {
    using subedit::core::readWhisperSegmentLine;

    CHECK_FALSE(readWhisperSegmentLine("").has_value());
    CHECK_FALSE(readWhisperSegmentLine("main: processing 'x.wav'").has_value());
    CHECK_FALSE(readWhisperSegmentLine("[00:00:01.000 --> nonsense]  text").has_value());
    CHECK_FALSE(readWhisperSegmentLine("[00:00:01.000 --> 00:00:02.000 no bracket").has_value());
    CHECK_FALSE(readWhisperSegmentLine("[00:00:01.000]  text").has_value());
    CHECK_FALSE(readWhisperSegmentLine("[00:00:01.000 --> 00:00:0").has_value());
}

TEST_CASE("the length of the audio is read from the line the tool starts with",
          "[whisper][progress]") {
    using subedit::core::readWhisperAudioLength;

    CHECK(readWhisperAudioLength(
              "main: processing 'a.wav' (176000 samples, 11.0 sec), 4 threads") == at(11000));
    // A file name with parentheses and numbers in it does not mislead.
    CHECK(readWhisperAudioLength("main: processing 'a (3 samples, 9 sec).wav' (16000 samples, "
                                 "1.5 sec), 4 threads") == at(1500));
    CHECK_FALSE(readWhisperAudioLength("whisper_init: something (10 samples, 2 sec)").has_value());
    CHECK_FALSE(
        readWhisperAudioLength("main: processing 'a.wav' (176000 samples, x sec)").has_value());
}

// ## The time left

TEST_CASE("nothing is said of the time left before there is enough to go on",
          "[transcription][progress]") {
    RemainingTime remaining;
    CHECK_FALSE(remaining.left(kOrigin).has_value());

    remaining.note(0.0, kOrigin);
    remaining.note(0.01, kOrigin + 2s);
    CHECK_FALSE(remaining.left(kOrigin + 2s).has_value());

    // Enough done, too little time: still a guess.
    remaining.note(0.10, kOrigin + 3s);
    CHECK_FALSE(remaining.left(kOrigin + 3s).has_value());
}

TEST_CASE("the time left follows the speed", "[transcription][progress]") {
    RemainingTime remaining;
    remaining.note(0.0, kOrigin);
    remaining.note(0.25, kOrigin + 25s);

    // A quarter in 25 seconds: 75 seconds more.
    CHECK(remaining.left(kOrigin + 25s) == 75s);
    // And it counts down between two reports.
    CHECK(remaining.left(kOrigin + 35s) == 65s);
    CHECK(remaining.left(kOrigin + 500s) == 0s);
}

TEST_CASE("the speed is the recent one, not the average of the whole run",
          "[transcription][progress]") {
    RemainingTime remaining;
    remaining.note(0.0, kOrigin);
    // A slow start: the model loads.
    remaining.note(0.02, kOrigin + 100s);
    // Then twice as fast: 0.10 in 20 seconds.
    remaining.note(0.12, kOrigin + 120s);
    remaining.note(0.22, kOrigin + 140s);
    remaining.note(0.32, kOrigin + 160s);
    remaining.note(0.42, kOrigin + 180s);

    const auto left = remaining.left(kOrigin + 180s);
    REQUIRE(left.has_value());
    // 58 % to do at 0.10 per 20 s is 116 s; the average of the whole run would say 248 s.
    CHECK(left.value_or(0s) > 100s);
    CHECK(left.value_or(1000s) < 140s);
}

TEST_CASE("no speed, no estimate: a run that has not moved says nothing",
          "[transcription][progress]") {
    RemainingTime remaining;
    remaining.note(0.5, kOrigin);
    remaining.note(0.5, kOrigin + 60s);

    CHECK_FALSE(remaining.left(kOrigin + 60s).has_value());
}

TEST_CASE("a fraction that goes back is ignored", "[transcription][progress]") {
    RemainingTime remaining;
    remaining.note(0.0, kOrigin);
    remaining.note(0.5, kOrigin + 50s);
    remaining.note(0.4, kOrigin + 60s);

    CHECK(remaining.left(kOrigin + 50s) == 50s);
}

// ## The range

TEST_CASE("a bound is read as a clock or as seconds", "[transcription][range]") {
    using subedit::core::readRangeBound;

    CHECK(readRangeBound("2:00") == at(120000));
    CHECK(readRangeBound("1:02:03") == at(3723000));
    CHECK(readRangeBound("2:00.5") == at(120500));
    CHECK(readRangeBound("90") == at(90000));
    CHECK(readRangeBound("90.5") == at(90500));
    CHECK(readRangeBound("  3:30  ") == at(210000));
    CHECK(readRangeBound("0") == at(0));
}

TEST_CASE("text that is no time is no bound", "[transcription][range]") {
    using subedit::core::readRangeBound;

    CHECK_FALSE(readRangeBound("").has_value());
    CHECK_FALSE(readRangeBound("   ").has_value());
    CHECK_FALSE(readRangeBound("abc").has_value());
    CHECK_FALSE(readRangeBound("-5").has_value());
    CHECK_FALSE(readRangeBound("-1:00").has_value());
    CHECK_FALSE(readRangeBound("1,5").has_value());
    CHECK_FALSE(readRangeBound("2:00,5").has_value());
    CHECK_FALSE(readRangeBound("1e3").has_value());
    CHECK_FALSE(readRangeBound("1.2.3").has_value());
    CHECK_FALSE(readRangeBound(".").has_value());
    CHECK_FALSE(readRangeBound("1:75").has_value());
}

TEST_CASE("a range needs its end after its start", "[transcription][range]") {
    using subedit::core::isRangePossible;

    CHECK(isRangePossible(std::nullopt, std::nullopt));
    CHECK(isRangePossible(at(1000), std::nullopt));
    CHECK(isRangePossible(std::nullopt, at(1000)));
    CHECK(isRangePossible(at(1000), at(2000)));
    CHECK_FALSE(isRangePossible(at(2000), at(2000)));
    CHECK_FALSE(isRangePossible(at(3000), at(2000)));
}

// ## The words

TEST_CASE("a position is written as a clock", "[wording][transcription]") {
    CHECK(subedit::core::clockText(at(0)) == "0:00:00");
    CHECK(subedit::core::clockText(at(200000)) == "0:03:20");
    CHECK(subedit::core::clockText(at(7384000)) == "2:03:04");
}

TEST_CASE("the time left is told to the minute", "[wording][transcription]") {
    using subedit::core::remainingTimeText;

    CHECK(remainingTimeText(0s) == "less than a minute left");
    CHECK(remainingTimeText(59s) == "less than a minute left");
    CHECK(remainingTimeText(60s) == "about 1 min left");
    CHECK(remainingTimeText(12min + 40s) == "about 12 min left");
    CHECK(remainingTimeText(60min) == "about 1 h 00 min left");
    CHECK(remainingTimeText(65min) == "about 1 h 05 min left");
    CHECK(remainingTimeText(3h + 25min) == "about 3 h 25 min left");
}

TEST_CASE("the status says what is known and is silent about the rest",
          "[wording][transcription]") {
    using subedit::core::TranscriptionProgress;
    using subedit::core::TranscriptionStage;
    using subedit::core::transcriptionStatusParts;

    CHECK(transcriptionStatusParts({.stage = TranscriptionStage::Extracting}, std::nullopt) ==
          std::vector<std::string>{"extracting the audio…"});
    CHECK(transcriptionStatusParts({}, std::nullopt) == std::vector<std::string>{"transcribing…"});
    CHECK(transcriptionStatusParts({.fraction = 0.5, .position = at(60000), .total = at(120000)},
                                   150s) ==
          std::vector<std::string>{"transcribing: 50%", "0:01:00 of 0:02:00", "about 2 min left"});
    CHECK(transcriptionStatusParts({.fraction = 0.1, .position = at(60000)}, std::nullopt) ==
          std::vector<std::string>{"transcribing: 10%", "0:01:00"});
    CHECK(subedit::core::lastTextLine("hello") == "last text: hello");
}

TEST_CASE("the range errors are told", "[wording][transcription]") {
    CHECK(subedit::core::emptyRange() == "the end of the range must come after its start");
    CHECK(subedit::core::notATimeInRange("abc") ==
          "\"abc\" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds");
}
