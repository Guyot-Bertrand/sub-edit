// How far a transcription is: the lines the tool prints, the time left, the range asked for,
// and the words that say it.

#include <subedit/core/transcription/annotation_notes.hpp>
#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/core/transcription/remaining_time.hpp>
#include <subedit/core/transcription/time_range.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

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

TEST_CASE("an engine that reports once in a long while still gives an estimate",
          "[transcription][progress]") {
    RemainingTime remaining;
    remaining.note(0.0, kOrigin);
    // The first report comes after two minutes, and the next will come two minutes after: nothing
    // happens in the last minute, and the estimate must not depend on what happens in it.
    remaining.note(0.05, kOrigin + 120s);

    // 5 % in 2 minutes: 38 minutes to go, less the 30 seconds since the report.
    CHECK(remaining.left(kOrigin + 150s) == 2250s);
    CHECK(remaining.left(kOrigin + 200s).has_value());
}

TEST_CASE("the same fraction said again adds nothing", "[transcription][progress]") {
    RemainingTime remaining;
    remaining.note(0.0, kOrigin);
    remaining.note(0.10, kOrigin + 100s);
    for (int tick = 1; tick <= 50; ++tick)
        remaining.note(0.10, kOrigin + 100s + std::chrono::seconds{tick * 10});

    // A hundred seconds for a tenth: nine hundred to go, less the five hundred since the report.
    CHECK(remaining.left(kOrigin + 600s) == 400s);
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

TEST_CASE("the time spent is told, and joins the status", "[wording][transcription]") {
    using subedit::core::TranscriptionProgress;
    using subedit::core::TranscriptionStage;
    using subedit::core::transcriptionStatusParts;

    CHECK(subedit::core::elapsedTimeText(312s) == "elapsed: 0:05:12");
    CHECK(transcriptionStatusParts({.stage = TranscriptionStage::Extracting}, std::nullopt, 5s) ==
          std::vector<std::string>{"extracting the audio…", "elapsed: 0:00:05"});
    CHECK(transcriptionStatusParts({.fraction = 0.5}, 60s, 125s) ==
          std::vector<std::string>{"transcribing: 50%", "about 1 min left", "elapsed: 0:02:05"});
}

TEST_CASE("the range errors are told", "[wording][transcription]") {
    CHECK(subedit::core::emptyRange() == "the end of the range must come after its start");
    CHECK(subedit::core::notATimeInRange("abc") ==
          "\"abc\" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds");
}

// ## The language a person types

TEST_CASE("a language is read from a two-letter code, a three-letter one, or a known case",
          "[transcription][language]") {
    using subedit::core::readLanguageCode;

    CHECK(readLanguageCode("fr") == "fr");
    CHECK(readLanguageCode("FR") == "fr");
    CHECK(readLanguageCode("fra") == "fr");
    CHECK(readLanguageCode("fre") == "fr");
    CHECK(readLanguageCode(" eng ") == "en");
    CHECK(readLanguageCode("deu") == "de");
    // Two letters the table lacks are kept.
    CHECK(readLanguageCode("af") == "af");
}

TEST_CASE("what is no language code is nothing", "[transcription][language]") {
    using subedit::core::readLanguageCode;

    CHECK_FALSE(readLanguageCode("").has_value());
    CHECK_FALSE(readLanguageCode("   ").has_value());
    CHECK_FALSE(readLanguageCode("f").has_value());
    CHECK_FALSE(readLanguageCode("f1").has_value());
    CHECK_FALSE(readLanguageCode("french").has_value());
    CHECK_FALSE(readLanguageCode("xyz").has_value());
    CHECK_FALSE(readLanguageCode("fr-FR").has_value());
}

TEST_CASE("the wording says what is wrong with a language", "[wording][transcription]") {
    CHECK(subedit::core::notALanguageCode("french") ==
          "\"french\" is not a language code: write two letters, such as fr, en, de or es");
}

// ## Notes in brackets

TEST_CASE("a piece that is only notes in brackets is recognised", "[transcription][notes]") {
    using subedit::core::isNotesOnly;

    CHECK(isNotesOnly("[MUSIC PLAYING]"));
    CHECK(isNotesOnly("[SPEAKING FRENCH] [MUSIC PLAYING] [SPEAKING FRENCH]"));
    CHECK(isNotesOnly("  (applause) "));
    CHECK_FALSE(isNotesOnly(""));
    CHECK_FALSE(isNotesOnly("   "));
    CHECK_FALSE(isNotesOnly("[]"));
    CHECK_FALSE(isNotesOnly("[MUSIC PLAYING] and then he spoke"));
    CHECK_FALSE(isNotesOnly("He said [no]"));
    CHECK_FALSE(isNotesOnly("[unclosed"));
}

TEST_CASE("a transcription of mostly notes looks like the wrong language",
          "[transcription][notes]") {
    using subedit::core::looksLikeWrongLanguage;
    using subedit::core::NotesShare;

    CHECK(looksLikeWrongLanguage(NotesShare{.notes = 5, .pieces = 9}));
    CHECK(looksLikeWrongLanguage(NotesShare{.notes = 2, .pieces = 6}));
    // Too few, or too small a share.
    CHECK_FALSE(looksLikeWrongLanguage(NotesShare{.notes = 1, .pieces = 1}));
    CHECK_FALSE(looksLikeWrongLanguage(NotesShare{.notes = 2, .pieces = 7}));
    CHECK_FALSE(looksLikeWrongLanguage(NotesShare{}));
}

TEST_CASE("the notes are counted over the pieces", "[transcription][notes]") {
    using subedit::core::TranscribedSegment;

    const std::vector<TranscribedSegment> segments{
        {.text = "[MUSIC PLAYING]", .start = at(0), .end = at(1000)},
        {.text = "Bonjour.", .start = at(1000), .end = at(2000)},
        {.text = "[SPEAKING FRENCH]", .start = at(2000), .end = at(3000)}};

    const auto share = subedit::core::notesShare(segments);

    CHECK(share.notes == 2);
    CHECK(share.pieces == 3);
}

TEST_CASE("the notices say what happened and what to do", "[wording][transcription]") {
    CHECK_THAT(subedit::core::noticeOfNotes(5, 9, "en"),
               Catch::Matchers::ContainsSubstring("5 of the 9 pieces are only notes in brackets"));
    CHECK_THAT(subedit::core::noticeOfNotes(5, 9, "en"),
               Catch::Matchers::ContainsSubstring("Run it again with the language typed"));
    CHECK_THAT(subedit::core::noticeOfEnglishOnlyModel(),
               Catch::Matchers::ContainsSubstring("Choose a multilingual model"));
}
