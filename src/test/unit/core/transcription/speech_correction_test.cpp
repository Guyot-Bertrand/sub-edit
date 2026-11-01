// Laying a subtitle file on the sound of a film from when there is speech, whatever the language
// (issue #820, phase 23).

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/align_on_speech.hpp>
#include <subedit/core/transcription/speech_correction.hpp>
#include <subedit/core/transcription/speech_detector.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "fake_speech_detector.hpp"
#include "scratch_directory.hpp"

namespace {

using subedit::core::LineTimes;
using subedit::core::SpeechCorrection;
using subedit::core::SpeechSpan;
using subedit::core::Timestamp;

Timestamp at(double seconds) {
    return Timestamp::fromMilliseconds(std::llround(seconds * 1000.0));
}

/// A film twenty minutes long: stretches of speech of one to four seconds, a pause of half a second
/// to three between them, from a fixed sequence so that the test says the same thing every time.
std::vector<SpeechSpan> aFilm(double length = 1200.0) {
    std::vector<SpeechSpan> film;
    std::uint32_t state = 12345;
    const auto next = [&state](double low, double high) {
        state = state * 1664525U + 1013904223U;
        return low + ((high - low) * static_cast<double>(state >> 8U) / 16777216.0);
    };
    for (double now = 5.0; now < length - 6.0;) {
        const double speech = next(1.0, 4.0);
        film.push_back({.start = at(now), .end = at(now + speech)});
        now += speech + next(0.5, 3.0);
    }
    return film;
}

/// The subtitles of that film: each stretch shown a little after it begins and a little after it
/// ends, as a person writes them — and a title card, shown over a silence, before anyone speaks.
std::vector<LineTimes> subtitlesOf(const std::vector<SpeechSpan>& film) {
    std::vector<LineTimes> lines{{.start = at(0.8), .end = at(2.3)}};
    for (const SpeechSpan& span : film)
        lines.push_back({.start = Timestamp::fromMilliseconds(span.start.milliseconds() + 100),
                         .end = Timestamp::fromMilliseconds(span.end.milliseconds() + 150)});
    return lines;
}

std::vector<LineTimes> faulty(const std::vector<LineTimes>& lines,
                              const std::function<double(double)>& fault) {
    std::vector<LineTimes> result;
    result.reserve(lines.size());
    for (const LineTimes& line : lines)
        result.push_back(
            {.start = at(fault(static_cast<double>(line.start.milliseconds()) / 1000.0)),
             .end = at(fault(static_cast<double>(line.end.milliseconds()) / 1000.0))});
    return result;
}

/// How many of the starts end up within half a second of where they were before the fault.
double within500(const SpeechCorrection& correction,
                 const std::vector<LineTimes>& wrong,
                 const std::vector<LineTimes>& right) {
    std::size_t good = 0;
    for (std::size_t line = 0; line < wrong.size(); ++line) {
        const auto error = std::abs(correction.corrected(wrong[line].start).milliseconds() -
                                    right[line].start.milliseconds());
        if (error <= 500)
            ++good;
    }
    return static_cast<double>(good) / static_cast<double>(wrong.size());
}

constexpr double kMostlyRight = 0.95;

} // namespace

TEST_CASE("the output of the voice detector is read in hundredths of a second",
          "[alignment][speech]") {
    const auto spans =
        subedit::core::parseSpeechSegments("Detected 3 speech segments:\n"
                                           "Speech segment 0: start = 493.00, end = 880.00\n"
                                           "Speech segment 1: start = 922.50, end = 1088.00\n"
                                           "whisper_vad_segments_from_probs: VAD segment 1: start "
                                           "= 9.22, end = 10.88 (duration: 1.66)\n"
                                           "Speech segment 2: start = 90.00, end = 80.00\n");

    REQUIRE(spans.size() == 2);
    CHECK(spans[0] == SpeechSpan{.start = at(4.93), .end = at(8.8)});
    CHECK(spans[1] == SpeechSpan{.start = at(9.225), .end = at(10.88)});
}

TEST_CASE("a file that is late, early, drifting or half late is put back on the sound",
          "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    const std::vector<LineTimes> right = subtitlesOf(film);

    const auto [name, fault] = GENERATE(
        std::pair<const char*, std::function<double(double)>>{"late",
                                                              [](double t) { return t + 2.4; }},
        std::pair<const char*, std::function<double(double)>>{"early",
                                                              [](double t) { return t - 7.0; }},
        std::pair<const char*, std::function<double(double)>>{
            "drift", [](double t) { return t + (12.0 * t / 1200.0); }},
        std::pair<const char*, std::function<double(double)>>{
            "second half late", [](double t) { return t + (t > 600.0 ? 5.0 : 0.0); }});
    CAPTURE(name);
    const std::vector<LineTimes> wrong = faulty(right, fault);

    const SpeechCorrection correction = subedit::core::estimateSpeechCorrection(film, wrong);

    REQUIRE(correction.fitted());
    CHECK(correction.ratio == 1.0);
    CHECK(within500(correction, wrong, right) >= kMostlyRight);
}

TEST_CASE("a file timed for another frame rate is put back, with or without a shift on top",
          "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    const std::vector<LineTimes> right = subtitlesOf(film);
    constexpr double kRatio = 25.0 / 23.976;

    const SpeechCorrection plain = subedit::core::estimateSpeechCorrection(
        film, faulty(right, [](double t) { return t * kRatio; }));
    const SpeechCorrection shifted = subedit::core::estimateSpeechCorrection(
        film, faulty(right, [](double t) { return (t * kRatio) + 3.0; }));

    CHECK(std::abs(plain.ratio - kRatio) < 1e-9);
    CHECK(std::abs(shifted.ratio - kRatio) < 1e-9);
    CHECK(within500(plain, faulty(right, [](double t) { return t * kRatio; }), right) >=
          kMostlyRight);
    CHECK(within500(shifted, faulty(right, [](double t) { return (t * kRatio) + 3.0; }), right) >=
          kMostlyRight);
}

TEST_CASE("a file that is right is left where it is", "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    const std::vector<LineTimes> right = subtitlesOf(film);

    const SpeechCorrection correction = subedit::core::estimateSpeechCorrection(film, right);

    REQUIRE(correction.fitted());
    CHECK(correction.ratio == 1.0);
    CHECK(within500(correction, right, right) >= kMostlyRight);
}

TEST_CASE("a title card nobody says does not pull the file off the speech", "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    std::vector<LineTimes> right = subtitlesOf(film);
    // Three more cards over silences, at the start and the end.
    right.insert(right.begin(), {.start = at(0.2), .end = at(0.7)});
    right.push_back({.start = at(1190.0), .end = at(1193.0)});
    const std::vector<LineTimes> wrong = faulty(right, [](double t) { return t + 4.0; });

    const SpeechCorrection correction = subedit::core::estimateSpeechCorrection(film, wrong);

    REQUIRE(correction.fitted());
    CHECK(within500(correction, wrong, right) >= kMostlyRight);
}

TEST_CASE("a file with nothing in common with the sound agrees little", "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    // Another film's subtitles: same kind of pattern, from another sequence.
    std::vector<SpeechSpan> other;
    std::uint32_t state = 987654321;
    for (double now = 3.0; now < 1190.0;) {
        state = state * 1664525U + 1013904223U;
        const double speech = 1.0 + (3.0 * static_cast<double>(state >> 8U) / 16777216.0);
        state = state * 1664525U + 1013904223U;
        other.push_back({.start = at(now), .end = at(now + speech)});
        now += speech + 0.5 + 2.5 * static_cast<double>(state >> 8U) / 16777216.0;
    }

    const SpeechCorrection same = subedit::core::estimateSpeechCorrection(film, subtitlesOf(film));
    const SpeechCorrection unrelated =
        subedit::core::estimateSpeechCorrection(film, subtitlesOf(other));

    CHECK(same.agreement > 0.5);
    CHECK(unrelated.agreement < 0.5 * same.agreement);
}

TEST_CASE("without speech or without lines there is nothing to correct", "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();

    CHECK_FALSE(subedit::core::estimateSpeechCorrection({}, subtitlesOf(film)).fitted());
    CHECK_FALSE(subedit::core::estimateSpeechCorrection(film, {}).fitted());
    CHECK(SpeechCorrection{}.corrected(at(12.0)) == at(12.0));
}

TEST_CASE("a few lines are put right on the speech around them", "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    const std::vector<LineTimes> all = subtitlesOf(film);
    // The lines from two to three minutes, nothing before or after.
    std::vector<LineTimes> right;
    std::vector<SpeechSpan> heard;
    for (const LineTimes& line : all)
        if (line.start >= at(120.0) && line.end <= at(180.0))
            right.push_back(line);
    for (const SpeechSpan& span : film)
        if (span.end >= at(60.0) && span.start <= at(240.0))
            heard.push_back(span);
    const std::vector<LineTimes> wrong = faulty(right, [](double t) { return t + 3.0; });

    const SpeechCorrection correction = subedit::core::estimateSpeechCorrection(heard, wrong);

    REQUIRE(correction.fitted());
    CHECK(within500(correction, wrong, right) >= kMostlyRight);
}

TEST_CASE("the lines of a media file are put on the speech the detector finds",
          "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    subedit::core::RealFileSystem files;
    const std::vector<SpeechSpan> film = aFilm();
    const std::vector<LineTimes> right = subtitlesOf(film);
    const std::vector<LineTimes> wrong = faulty(right, [](double t) { return t + 2.4; });
    const subedit::test::FakeSpeechDetector detector{film};
    const auto audio = scratch.write("voice.wav", "audio");
    const auto model = scratch.write("vad.bin", "vad");

    const auto done = subedit::core::alignMediaOnSpeech(
        files, detector, "", {.media = {.input = audio, .vadModel = model}, .lines = wrong});

    REQUIRE(done.has_value());
    CHECK(detector.audio() == audio);
    CHECK(detector.model() == model);
    const auto& matches = done.value().matches;
    REQUIRE(matches.size() == wrong.size());
    std::size_t good = 0;
    for (std::size_t line = 0; line < wrong.size(); ++line) {
        const auto& place = matches[line].place;
        REQUIRE(place.start.has_value());
        const auto error = std::abs(place.start.value_or(Timestamp::origin()).milliseconds() -
                                    right[line].start.milliseconds());
        if (error <= 500)
            ++good;
        CHECK(matches[line].words == matches[line].matched);
    }
    CHECK(static_cast<double>(good) / static_cast<double>(wrong.size()) >= kMostlyRight);
}

TEST_CASE("when the sound and the file agree too little nothing is placed", "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    subedit::core::RealFileSystem files;
    // A film with a single stretch of speech and a file of lines that cover it half and half.
    const subedit::test::FakeSpeechDetector detector{{{.start = at(10.0), .end = at(11.0)}}};
    const auto audio = scratch.write("voice.wav", "audio");
    std::vector<LineTimes> lines;
    lines.reserve(200);
    for (int line = 0; line < 200; ++line)
        lines.push_back({.start = at(2.0 * line), .end = at((2.0 * line) + 1.0)});

    const auto done = subedit::core::alignMediaOnSpeech(
        files, detector, "", {.media = {.input = audio}, .lines = lines});

    REQUIRE(done.has_value());
    CHECK_FALSE(done->correction.fitted());
    CHECK(std::ranges::none_of(done->matches, [](const subedit::core::LineMatch& match) {
        return match.place.start.has_value();
    }));
}

TEST_CASE("a time is carried by the shifts around it, and by the nearest one outside them",
          "[alignment][speech]") {
    SpeechCorrection correction;
    correction.shifts = {{.at = at(10.0), .shiftMilliseconds = 1000},
                         {.at = at(20.0), .shiftMilliseconds = 3000}};

    CHECK(correction.corrected(at(5.0)) == at(6.0));
    CHECK(correction.corrected(at(15.0)) == at(17.0));
    CHECK(correction.corrected(at(30.0)) == at(33.0));
}

TEST_CASE("speech everywhere, or a film too short for a stretch, gives nothing to correct",
          "[alignment][speech]") {
    const std::vector<LineTimes> lines{{.start = at(10.0), .end = at(12.0)},
                                       {.start = at(100.0), .end = at(102.0)}};

    // Speech from the first second to the last: no moment is told from another.
    const std::vector<SpeechSpan> always{{.start = at(0.0), .end = at(300.0)}};
    CHECK(subedit::core::estimateSpeechCorrection(always, lines).agreement < 0.01);

    // A few seconds of film: shorter than the least stretch a shift is found for.
    const std::vector<SpeechSpan> brief{{.start = at(0.0), .end = at(2.0)}};
    CHECK_FALSE(subedit::core::estimateSpeechCorrection(
                    brief, std::vector<LineTimes>{{.start = at(0.0), .end = at(2.0)}})
                    .fitted());

    // Speech in one place and lines in another, with no stretch that holds both.
    const std::vector<SpeechSpan> early{{.start = at(0.0), .end = at(60.0)}};
    const std::vector<LineTimes> late{{.start = at(900.0), .end = at(960.0)}};
    CHECK_FALSE(subedit::core::estimateSpeechCorrection(early, late).fitted());
}

TEST_CASE(
    "the voice detector's output is read line by line, and a line without its numbers is skipped",
    "[alignment][speech]") {
    const auto spans =
        subedit::core::parseSpeechSegments("Speech segment 0: end = 90.00\n"
                                           "Speech segment 1: start = soon, end = 90.00\n"
                                           "Speech segment 2: start = 10.00, end = 20.00\n"
                                           "Speech segment 3: start = 10.00, end = 1e300\n"
                                           "Speech segment 4: start = 10.00, end = inf\n"
                                           "Speech segment 5: start = nan, end = 20.00");

    REQUIRE(spans.size() == 1);
    CHECK(spans.front() == SpeechSpan{.start = at(0.1), .end = at(0.2)});
}

TEST_CASE("a detector that fails fails the alignment, and the reason is kept",
          "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    subedit::core::RealFileSystem files;
    const subedit::test::FakeSpeechDetector detector{subedit::core::TranscriptionError{
        .kind = subedit::core::TranscriptionErrorKind::Failed, .detail = "no way"}};
    const auto audio = scratch.write("voice.wav", "audio");

    const auto done = subedit::core::alignMediaOnSpeech(
        files,
        detector,
        "",
        {.media = {.input = audio}, .lines = {{.start = at(1.0), .end = at(2.0)}}});

    REQUIRE_FALSE(done.has_value());
    const auto* reason = std::get_if<subedit::core::TranscriptionError>(&done.error().cause);
    REQUIRE(reason != nullptr);
    CHECK(reason->detail == "no way");
}

TEST_CASE("a time far beyond the film does not decide how much is looked at",
          "[alignment][speech]") {
    const std::vector<SpeechSpan> film = aFilm();
    std::vector<LineTimes> lines = subtitlesOf(film);
    // A file that says one of its subtitles ends after thirty-five thousand hours.
    lines.push_back({.start = at(125000000.0), .end = at(126000000.0)});

    const SpeechCorrection correction = subedit::core::estimateSpeechCorrection(film, lines);

    CHECK(correction.fitted());
    CHECK(correction.ratio == 1.0);
}
