// The tools of phase 23: the alignment cases and their truth, the measure of an answer, the fake
// engine and the contract.
//
// Nothing here aligns anything: this is what the alignment will be held against, and the proof that
// the tools say what they should (issue #803).

#include <subedit/core/transcription/alignment_engine.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "alignment_cases.hpp"
#include "alignment_engine_contract.hpp"
#include "fake_alignment_engine.hpp"
#include "scratch_directory.hpp"

namespace {

using subedit::core::AlignedLine;
using subedit::core::Timestamp;
using subedit::test::AlignmentCase;
using subedit::test::FakeAlignmentEngine;
using subedit::test::FakeAlignmentScript;
using subedit::test::loadAlignmentCase;
using subedit::test::measure;

const std::vector<std::string> kCases{
    "offset-plus-2400", "offset-minus-900", "drift", "partial", "extra-line", "unheard-line"};

/// What a perfect engine answers: the truth, and nothing for a line said nowhere.
std::vector<AlignedLine> truthOf(const AlignmentCase& wanted) {
    std::vector<AlignedLine> answer;
    for (const auto& place : wanted.truth) {
        if (place)
            answer.push_back({.start = place->start, .end = place->end});
        else
            answer.emplace_back();
    }
    return answer;
}

/// What an engine that corrects nothing answers: the times the file already has.
std::vector<AlignedLine> givenOf(const AlignmentCase& wanted) {
    std::vector<AlignedLine> answer;
    for (const auto& place : wanted.given)
        answer.push_back({.start = place.start, .end = place.end});
    return answer;
}

} // namespace

TEST_CASE("every case has a line, a time and a truth for each line of its file",
          "[alignment][cases][ALIGN-TOOLS-01]") {
    for (const std::string& name : kCases) {
        INFO(name);
        const AlignmentCase wanted = loadAlignmentCase(name);
        REQUIRE_FALSE(wanted.lines.empty());
        CHECK(wanted.given.size() == wanted.lines.size());
        CHECK(wanted.truth.size() == wanted.lines.size());
    }
}

TEST_CASE("the truth is what the fault laid down", "[alignment][cases][ALIGN-TOOLS-01]") {
    // A constant offset: every line is given 2.4 s after where it is said.
    const AlignmentCase late = loadAlignmentCase("offset-plus-2400");
    for (std::size_t line = 0; line < late.lines.size(); ++line) {
        REQUIRE(late.truth[line].has_value());
        CHECK((late.given[line].start - late.truth[line]->start).milliseconds() == 2400);
    }

    // A drift: the delay grows from nothing to three seconds.
    const AlignmentCase drifting = loadAlignmentCase("drift");
    CHECK((drifting.given.front().start - drifting.truth.front()->start).milliseconds() == 0);
    CHECK((drifting.given.back().start - drifting.truth.back()->start).milliseconds() == 3000);

    // A line that is said nowhere has no truth, and is in the file.
    const AlignmentCase extra = loadAlignmentCase("extra-line");
    CHECK(extra.lines.size() == 13);
    std::size_t nowhere = 0;
    for (const auto& place : extra.truth)
        if (!place)
            ++nowhere;
    CHECK(nowhere == 1);
    CHECK(extra.lines[5] == "Subtitles by an unknown hand.");
    CHECK_FALSE(extra.truth[5].has_value());
}

TEST_CASE("what an engine would have heard reads as the tool writes it, at the true times",
          "[alignment][cases][ALIGN-TOOLS-01]") {
    for (const std::string& name : kCases) {
        INFO(name);
        const AlignmentCase wanted = loadAlignmentCase(name);
        std::ifstream file{wanted.heard};
        const std::string text{std::istreambuf_iterator<char>{file},
                               std::istreambuf_iterator<char>{}};

        const auto output = subedit::core::readWhisperJson(text);

        REQUIRE(output.has_value());
        const auto& segments = output->transcription.segments;
        // One segment per spoken line, and none for the line the engine did not hear.
        CHECK(segments.size() == (name == "unheard-line" ? 11U : 12U));
        REQUIRE_FALSE(segments.empty());
        CHECK(segments.front().start == Timestamp::fromMilliseconds(1000));
        CHECK_FALSE(segments.front().words.empty());
        CHECK(segments.front().words.front().start == segments.front().start);
    }
}

TEST_CASE("a perfect answer is at no distance, and finds everything the truth places",
          "[alignment][measure][ALIGN-TOOLS-02]") {
    for (const std::string& name : kCases) {
        INFO(name);
        const AlignmentCase wanted = loadAlignmentCase(name);

        const auto quality = measure(truthOf(wanted), wanted);

        CHECK(quality.worst == 0);
        CHECK(quality.median == 0);
        CHECK(quality.missed == 0);
        CHECK(quality.invented == 0);
        CHECK(quality.found == wanted.lines.size() - (name == "extra-line" ? 1U : 0U));
    }
}

TEST_CASE("an engine that corrects nothing is as far as the fault it was not asked to find",
          "[alignment][measure][ALIGN-TOOLS-02]") {
    const AlignmentCase late = loadAlignmentCase("offset-plus-2400");
    CHECK(measure(givenOf(late), late).worst == 2400);
    CHECK(measure(givenOf(late), late).median == 2400);

    const AlignmentCase early = loadAlignmentCase("offset-minus-900");
    CHECK(measure(givenOf(early), early).worst == 900);

    const AlignmentCase drifting = loadAlignmentCase("drift");
    CHECK(measure(givenOf(drifting), drifting).worst == 3000);

    // Half of the file is right, half is two seconds late.
    const AlignmentCase half = loadAlignmentCase("partial");
    const auto quality = measure(givenOf(half), half);
    CHECK(quality.worst == 2000);
    CHECK(quality.errors.front() == 0);
    CHECK(quality.errors.back() == 2000);
}

TEST_CASE("an alignment is judged on what it invents and what it misses",
          "[alignment][measure][ALIGN-TOOLS-02]") {
    // The line said nowhere: giving it a place is the one thing an alignment must not do.
    const AlignmentCase extra = loadAlignmentCase("extra-line");
    CHECK(measure(givenOf(extra), extra).invented == 1);
    CHECK(measure(truthOf(extra), extra).invented == 0);

    // A spoken line the engine did not hear: said not found, and counted, not hidden.
    const AlignmentCase unheard = loadAlignmentCase("unheard-line");
    std::vector<AlignedLine> answer = truthOf(unheard);
    answer[8] = AlignedLine{};
    const auto quality = measure(answer, unheard);
    CHECK(quality.missed == 1);
    CHECK(quality.invented == 0);
    CHECK(quality.errors[8] == std::nullopt);
}

TEST_CASE("the fake alignment engine answers what it was told, and passes the contract",
          "[alignment][contract][ALIGN-TOOLS-03]") {
    const subedit::test::ScratchDirectory scratch;
    const AlignmentCase wanted = loadAlignmentCase("offset-plus-2400");
    auto seen = std::make_shared<subedit::core::AlignmentRequest>();
    const FakeAlignmentEngine engine{FakeAlignmentScript{.answers = truthOf(wanted), .seen = seen}};
    const auto audio = scratch.write("voice.wav", "audio");

    subedit::test::checkAlignmentEngineContract(engine, {.audio = audio, .lines = wanted.lines});

    const auto done = engine.align({.audio = audio, .lines = wanted.lines}, {});
    REQUIRE(done.has_value());
    CHECK(measure(*done, wanted).worst == 0);
    CHECK(seen->lines == wanted.lines);
}

TEST_CASE("an engine that needs a model is held to ask for it before any work",
          "[alignment][contract][ALIGN-TOOLS-03]") {
    const subedit::test::ScratchDirectory scratch;
    const FakeAlignmentEngine engine{FakeAlignmentScript{.capabilities = {.needsModel = true}}};
    const auto audio = scratch.write("voice.wav", "audio");
    const auto model = scratch.write("model.bin", "model");

    subedit::test::checkAlignmentEngineContract(
        engine, {.audio = audio, .model = model, .lines = {"one", "two", "three"}});
}
