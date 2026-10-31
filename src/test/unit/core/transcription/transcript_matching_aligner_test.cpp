// The first alignment engine: a transcription engine listens, the text is matched to what it
// heard (issue #812, phase 23).

#include <subedit/core/config/alignment_settings.hpp>
#include <subedit/core/transcription/transcript_matching_aligner.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include "alignment_cases.hpp"
#include "alignment_engine_contract.hpp"
#include "fake_transcription_engine.hpp"
#include "scratch_directory.hpp"

namespace {

using subedit::core::TranscriptMatchingAligner;
using subedit::test::AlignmentCase;
using subedit::test::FakeEngineScript;
using subedit::test::FakeTranscriptionEngine;

const subedit::core::EngineCapabilities kListening{
    .timedWords = true, .languageDetection = true, .needsModel = false};

/// What the case's engine would have heard, as a fake transcription engine answers it.
FakeEngineScript hearing(const AlignmentCase& wanted) {
    const std::string text = subedit::test::detail::readText(wanted.heard);
    const auto output =
        subedit::core::readWhisperJson(text).value_or(subedit::core::WhisperOutput{});
    return {.capabilities = kListening, .segments = output.transcription.segments};
}

} // namespace

TEST_CASE("the engine places the lines where the transcription engine heard them",
          "[alignment][aligner]") {
    const subedit::test::ScratchDirectory scratch;
    const AlignmentCase wanted = subedit::test::loadAlignmentCase("offset-plus-2400");
    const FakeTranscriptionEngine hearer{hearing(wanted)};
    const TranscriptMatchingAligner aligner{hearer};
    const auto audio = scratch.write("voice.wav", "audio");

    const auto done = aligner.align({.audio = audio, .lines = wanted.lines}, {});

    REQUIRE(done.has_value());
    const auto quality = subedit::test::measure(*done, wanted);
    CHECK(quality.worst <= 500);
    CHECK(quality.missed == 0);
    CHECK(quality.invented == 0);
}

TEST_CASE("the engine hands its audio, language, model, voice model and range to the listener",
          "[alignment][aligner]") {
    const subedit::test::ScratchDirectory scratch;
    const AlignmentCase wanted = subedit::test::loadAlignmentCase("drift");
    FakeEngineScript script = hearing(wanted);
    script.seen = std::make_shared<subedit::core::TranscriptionRequest>();
    const auto seen = script.seen;
    const FakeTranscriptionEngine hearer{std::move(script)};
    const TranscriptMatchingAligner aligner{hearer};
    const auto audio = scratch.write("voice.wav", "audio");
    const auto vad = scratch.write("vad.bin", "vad");

    const auto done = aligner.align({.audio = audio,
                                     .lines = wanted.lines,
                                     .language = "en",
                                     .vadModel = vad,
                                     .start = subedit::core::Timestamp::fromMilliseconds(1000),
                                     .end = subedit::core::Timestamp::fromMilliseconds(90000)},
                                    {});

    REQUIRE(done.has_value());
    CHECK(seen->audio == audio);
    CHECK(seen->language == "en");
    CHECK(seen->vadModel == vad);
    CHECK(seen->start == subedit::core::Timestamp::fromMilliseconds(1000));
    CHECK(seen->end == subedit::core::Timestamp::fromMilliseconds(90000));
}

TEST_CASE("the counts the doubt is judged on come with the places", "[alignment][aligner]") {
    const subedit::test::ScratchDirectory scratch;
    const AlignmentCase wanted = subedit::test::loadAlignmentCase("extra-line");
    const FakeTranscriptionEngine hearer{hearing(wanted)};
    const TranscriptMatchingAligner aligner{hearer};
    const auto audio = scratch.write("voice.wav", "audio");

    const auto matches = aligner.match({.audio = audio, .lines = wanted.lines}, {});

    REQUIRE(matches.has_value());
    REQUIRE(matches->size() == wanted.lines.size());
    CHECK((*matches)[0].matched == (*matches)[0].words);
    CHECK_FALSE((*matches)[5].place.start.has_value());
}

TEST_CASE("an engine that gives no timed words cannot align", "[alignment][aligner]") {
    const subedit::test::ScratchDirectory scratch;
    const FakeTranscriptionEngine hearer{
        FakeEngineScript{.capabilities = {.timedWords = false, .needsModel = false}}};
    const TranscriptMatchingAligner aligner{hearer};
    const auto audio = scratch.write("voice.wav", "audio");

    const auto done = aligner.align({.audio = audio, .lines = {"One", "Two"}}, {});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == subedit::core::TranscriptionErrorKind::Failed);
}

TEST_CASE("what the listener cannot say, the aligner cannot either", "[alignment][aligner]") {
    const FakeTranscriptionEngine hearer{
        FakeEngineScript{.capabilitiesError = subedit::core::TranscriptionError{
                             .kind = subedit::core::TranscriptionErrorKind::EngineMissing}}};
    const TranscriptMatchingAligner aligner{hearer};

    const auto capabilities = aligner.capabilitiesFor(std::nullopt);
    REQUIRE_FALSE(capabilities.has_value());
    CHECK(capabilities.error().kind == subedit::core::TranscriptionErrorKind::EngineMissing);

    const auto done = aligner.align({.audio = "voice.wav", .lines = {"One"}}, {});
    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == subedit::core::TranscriptionErrorKind::EngineMissing);
}

TEST_CASE("no text, no entry, and no work", "[alignment][aligner]") {
    const FakeTranscriptionEngine hearer{FakeEngineScript{.capabilities = kListening}};
    const TranscriptMatchingAligner aligner{hearer};

    const auto done = aligner.align({.audio = "absent.wav", .lines = {}}, {});

    REQUIRE(done.has_value());
    CHECK(done->empty());
}

TEST_CASE("the engine passes the contract of the port", "[alignment][aligner][contract]") {
    const subedit::test::ScratchDirectory scratch;
    const AlignmentCase wanted = subedit::test::loadAlignmentCase("offset-minus-900");
    const auto audio = scratch.write("voice.wav", "audio");

    const FakeTranscriptionEngine hearer{hearing(wanted)};
    const TranscriptMatchingAligner aligner{hearer};
    subedit::test::checkAlignmentEngineContract(aligner, {.audio = audio, .lines = wanted.lines});

    // An engine that needs a model is held to ask for it before any work.
    FakeEngineScript needing = hearing(wanted);
    needing.capabilities.needsModel = true;
    const FakeTranscriptionEngine needyHearer{std::move(needing)};
    const TranscriptMatchingAligner needy{needyHearer};
    const auto model = scratch.write("model.bin", "model");
    subedit::test::checkAlignmentEngineContract(
        needy, {.audio = audio, .model = model, .lines = wanted.lines});
}

TEST_CASE("the factory knows the engine by its name", "[alignment][aligner]") {
    const FakeTranscriptionEngine hearer{FakeEngineScript{.capabilities = kListening}};

    const auto engine = subedit::core::makeAlignmentEngine("transcript-matching", hearer);
    REQUIRE(engine != nullptr);
    CHECK(engine->name() == subedit::core::kDefaultAlignmentEngine);

    CHECK(subedit::core::makeAlignmentEngine("no-such-engine", hearer) == nullptr);
}

TEST_CASE("the settings start at the defaults of the spec", "[alignment][aligner]") {
    const subedit::core::AlignmentSettings settings;

    CHECK(settings.engine == "transcript-matching");
    CHECK(settings.minimumMatchPercent == 50);
    CHECK(settings.minimumMatch() == subedit::core::kDefaultMinimumMatch);
    CHECK(settings.doubtGapMilliseconds == 2000);
}
