// The adapter around `whisper-vad-speech-segments`, against stand-ins for the program (issue #820,
// phase 23).

#include <subedit/core/transcription/speech_detector.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "scratch_directory.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::SpeechSpan;
using subedit::core::Timestamp;
using subedit::core::TranscriptionErrorKind;
using subedit::core::WhisperVadDetector;

/// An executable that runs `body` as a shell script.
std::filesystem::path program(const subedit::test::ScratchDirectory& scratch,
                              const std::string& body) {
    const std::filesystem::path script = scratch.write("vad-tool", "#!/bin/sh\n" + body + "\n");
    std::filesystem::permissions(
        script, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    return script;
}

} // namespace

TEST_CASE("the detector reads the stretches the program prints", "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    const WhisperVadDetector detector{
        program(scratch,
                "echo 'Detected 1 speech segments:'\n"
                "echo 'Speech segment 0: start = 493.00, end = 880.00'\n"
                "echo 'whisper_vad: noise' >&2")};
    const auto audio = scratch.write("voice.wav", "audio");
    const auto model = scratch.write("vad.bin", "model");

    const auto done = detector.detect(audio, model, {});

    REQUIRE(done.has_value());
    REQUIRE(done->size() == 1);
    CHECK(done->front() == SpeechSpan{.start = Timestamp::fromMilliseconds(4930),
                                      .end = Timestamp::fromMilliseconds(8800)});
}

TEST_CASE("the detector refuses a missing program, model or audio before running anything",
          "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    const auto audio = scratch.write("voice.wav", "audio");
    const auto model = scratch.write("vad.bin", "model");
    const WhisperVadDetector detector{program(scratch, "exit 0")};

    const WhisperVadDetector nowhere{scratch.file("nowhere")};
    const auto noProgram = nowhere.detect(audio, model, {});
    REQUIRE_FALSE(noProgram.has_value());
    CHECK(noProgram.error().kind == TranscriptionErrorKind::EngineMissing);

    const auto noModelNamed = detector.detect(audio, std::nullopt, {});
    REQUIRE_FALSE(noModelNamed.has_value());
    CHECK(noModelNamed.error().kind == TranscriptionErrorKind::ModelMissing);

    const auto noModelThere = detector.detect(audio, scratch.file("nowhere.bin"), {});
    REQUIRE_FALSE(noModelThere.has_value());
    CHECK(noModelThere.error().kind == TranscriptionErrorKind::ModelMissing);

    const auto noAudio = detector.detect(scratch.file("nowhere.wav"), model, {});
    REQUIRE_FALSE(noAudio.has_value());
    CHECK(noAudio.error().kind == TranscriptionErrorKind::AudioUnreadable);
}

TEST_CASE("the detector says what the program said when it fails, and when it cannot start",
          "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    const auto audio = scratch.write("voice.wav", "audio");
    const auto model = scratch.write("vad.bin", "model");

    const WhisperVadDetector failing{program(scratch, "echo 'error: no way' >&2\nexit 3")};
    const auto failed = failing.detect(audio, model, {});
    REQUIRE_FALSE(failed.has_value());
    CHECK(failed.error().kind == TranscriptionErrorKind::Failed);
    CHECK_THAT(failed.error().detail, ContainsSubstring("no way"));

    // A file that is there and cannot be run.
    const WhisperVadDetector notRunnable{scratch.write("plain-file", "not a program")};
    const auto unlaunched = notRunnable.detect(audio, model, {});
    REQUIRE_FALSE(unlaunched.has_value());
    CHECK(unlaunched.error().kind == TranscriptionErrorKind::Failed);
}

TEST_CASE("the detector stops the program when asked to", "[alignment][speech]") {
    const subedit::test::ScratchDirectory scratch;
    const auto audio = scratch.write("voice.wav", "audio");
    const auto model = scratch.write("vad.bin", "model");
    const WhisperVadDetector slow{program(scratch, "sleep 30")};
    const subedit::core::TranscriptionObserver impatient{.cancelled = [] { return true; }};

    const auto stopped = slow.detect(audio, model, impatient);

    REQUIRE_FALSE(stopped.has_value());
    CHECK(stopped.error().kind == TranscriptionErrorKind::Cancelled);
}
