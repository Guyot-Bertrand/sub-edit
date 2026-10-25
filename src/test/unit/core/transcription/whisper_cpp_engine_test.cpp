// The whisper.cpp adapter, against a stand-in for whisper-cli: the fake
// transcriber, which writes a sample of the tool's real output where the
// adapter says to.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/whisper_cpp_engine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

#include "scratch_directory.hpp"
#include "transcription_engine_contract.hpp"

namespace {

using subedit::core::Timestamp;
using subedit::core::TranscriptionErrorKind;
using subedit::core::WhisperCppEngine;
using subedit::test::ScratchDirectory;

const std::filesystem::path kFake{SUBEDIT_FAKE_TRANSCRIBER_BINARY};
const std::string kSample = std::string{SUBEDIT_TEST_DATA_DIR} + "/whisper/jfk.full.json";

std::string header(unsigned vocabulary) {
    std::string bytes = "lmgg";
    for (int index = 0; index < 4; ++index)
        bytes += static_cast<char>((vocabulary >> (8 * index)) & 0xFFU);
    return bytes + std::string(8, '\0');
}

/// A scratch directory with a `whisper-cli` that succeeds, an audio file and a model.
struct Setup {
    ScratchDirectory scratch;
    std::filesystem::path executable;
    std::filesystem::path audio = scratch.write("audio.wav", "audio");
    std::filesystem::path multilingual = scratch.write("multilingual.bin", header(51865));
    std::filesystem::path englishOnly = scratch.write("english.bin", header(51864));

    explicit Setup(const std::string& knobs = "--progress=4 --output-from='" + kSample + "'")
        : executable(scratch.standIn("whisper-cli", kFake, knobs)) {}
};

} // namespace

TEST_CASE("a multilingual model gives detection and every language", "[whisper][engine]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};

    const auto capabilities = engine.capabilitiesFor(setup.multilingual);

    REQUIRE(capabilities.has_value());
    CHECK(capabilities->timedWords);
    CHECK(capabilities->languageDetection);
    CHECK(capabilities->languages.empty());
    CHECK(capabilities->needsModel);
}

TEST_CASE("an English-only model gives neither detection nor another language",
          "[whisper][engine]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};

    const auto capabilities = engine.capabilitiesFor(setup.englishOnly);

    REQUIRE(capabilities.has_value());
    CHECK_FALSE(capabilities->languageDetection);
    CHECK(capabilities->languages == std::vector<std::string>{"en"});
}

TEST_CASE("the capabilities refuse what they cannot say", "[whisper][engine]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};
    const WhisperCppEngine absent{setup.scratch.file("absent")};

    CHECK(absent.capabilitiesFor(setup.multilingual).error().kind ==
          TranscriptionErrorKind::EngineMissing);
    CHECK(engine.capabilitiesFor(std::nullopt).error().kind ==
          TranscriptionErrorKind::ModelMissing);
    CHECK(engine.capabilitiesFor(setup.scratch.file("none.bin")).error().kind ==
          TranscriptionErrorKind::ModelMissing);
    CHECK(engine.capabilitiesFor(setup.audio).error().kind == TranscriptionErrorKind::Failed);
}

TEST_CASE("a multilingual model passes the contract", "[whisper][engine][contract]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};

    subedit::test::checkTranscriptionEngineContract(
        engine, {.audio = setup.audio, .model = setup.multilingual, .supportedLanguage = "en"});
}

TEST_CASE("an English-only model passes the contract", "[whisper][engine][contract]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};

    subedit::test::checkTranscriptionEngineContract(engine,
                                                    {.audio = setup.audio,
                                                     .model = setup.englishOnly,
                                                     .supportedLanguage = "en",
                                                     .unsupportedLanguage = "fr"});
}

TEST_CASE("a run gives the segments of the sample, and reports its progress", "[whisper][engine]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};
    std::vector<double> progress;

    const auto done = engine.transcribe(
        {.audio = setup.audio, .model = setup.multilingual},
        {.progress = [&progress](double fraction) { progress.push_back(fraction); }});

    REQUIRE(done.has_value());
    CHECK(done->language == "en");
    REQUIRE(done->segments.size() == 1);
    CHECK(done->segments.front().words.size() == 22);
    CHECK(progress == std::vector<double>{0.25, 0.5, 0.75, 1.0, 1.0});
}

TEST_CASE("the command line is the one the tool expects", "[whisper][engine]") {
    const Setup setup{"--output-from='" + kSample + "'"};
    const WhisperCppEngine engine{setup.executable};
    std::vector<std::string> arguments;
    // The stand-in writes the arguments it received to standard output, which
    // the adapter ignores; a second stand-in captures them in a file instead.
    const auto spy = setup.scratch.write(
        "spy-cli",
        "#!/bin/sh\nprintf '%s\\n' \"$@\" > '" + setup.scratch.file("arguments").string() +
            "'\nexec '" + setup.executable.string() + "' \"$@\"\n");
    std::filesystem::permissions(
        spy, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    const WhisperCppEngine spied{spy};

    const auto done =
        spied.transcribe({.audio = setup.audio, .language = "fr", .model = setup.multilingual});

    REQUIRE(done.has_value());
    std::ifstream file{setup.scratch.file("arguments")};
    for (std::string line; std::getline(file, line);)
        arguments.push_back(line);
    REQUIRE(arguments.size() == 12);
    CHECK(arguments[0] == "-m");
    CHECK(arguments[1] == setup.multilingual.string());
    CHECK(arguments[2] == "-f");
    CHECK(arguments[3] == setup.audio.string());
    CHECK(arguments[4] == "-l");
    CHECK(arguments[5] == "fr");
    CHECK(arguments[6] == "-pp");
    CHECK(arguments[7] == "-oj");
    CHECK(arguments[8] == "-ojf");
    CHECK(arguments[9] == "-of");
    CHECK(arguments[10].ends_with("/out"));
    CHECK(arguments[11] == "-sns");
}

TEST_CASE("the exit codes of the tool become the errors of the port", "[whisper][engine]") {
    const auto errorOf = [](int code) {
        const Setup setup{"--fail-with=" + std::to_string(code)};
        const WhisperCppEngine engine{setup.executable};
        const auto done = engine.transcribe({.audio = setup.audio, .model = setup.multilingual});
        REQUIRE_FALSE(done.has_value());
        return done.error().kind;
    };

    CHECK(errorOf(3) == TranscriptionErrorKind::ModelMissing);
    CHECK(errorOf(2) == TranscriptionErrorKind::AudioUnreadable);
    CHECK(errorOf(11) == TranscriptionErrorKind::AudioUnreadable);
    CHECK(errorOf(1) == TranscriptionErrorKind::Failed);
}

TEST_CASE("output that is cut short is a failure", "[whisper][engine]") {
    const Setup setup{"--output-from='" + kSample + "' --truncate-output"};
    const WhisperCppEngine engine{setup.executable};

    const auto done = engine.transcribe({.audio = setup.audio, .model = setup.multilingual});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::Failed);
}

TEST_CASE("a stand-in that cannot be started is a missing engine", "[whisper][engine]") {
    const Setup setup;
    const auto plain = setup.scratch.write("plain", "not a program");
    const WhisperCppEngine engine{plain};

    const auto done = engine.transcribe({.audio = setup.audio, .model = setup.multilingual});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::Failed);
}

TEST_CASE("asking to stop stops the tool", "[whisper][engine]") {
    const Setup setup{"--progress=1 --linger=30"};
    const WhisperCppEngine engine{setup.executable};

    const auto done = engine.transcribe({.audio = setup.audio, .model = setup.multilingual},
                                        {.cancelled = [] { return true; }});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::Cancelled);
}

TEST_CASE("whisper-cli is found where the user says, or on the search path", "[whisper][engine]") {
    const Setup setup;
    const subedit::core::RealFileSystem files;
    const std::string path = setup.scratch.path().string();

    CHECK(subedit::core::findWhisperCli(files, path, std::nullopt) == setup.executable);
    CHECK(subedit::core::findWhisperCli(files, "", std::nullopt) == std::nullopt);
    CHECK(subedit::core::findWhisperCli(files, "", setup.executable) == setup.executable);
    CHECK(subedit::core::findWhisperCli(files, path, setup.scratch.file("elsewhere")) ==
          std::nullopt);
}

TEST_CASE("the last error line of the tool comes back with the failure", "[whisper][engine]") {
    const Setup setup;
    const auto failing = setup.scratch.write(
        "failing-cli", "#!/bin/sh\necho 'error: model file not found' >&2\nexit 3\n");
    std::filesystem::permissions(
        failing, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    const WhisperCppEngine engine{failing};

    const auto done = engine.transcribe({.audio = setup.audio, .model = setup.multilingual});

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::ModelMissing);
    CHECK(done.error().detail == "error: model file not found");
}

TEST_CASE("the engine is known by the name the setting uses", "[whisper][engine]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};

    CHECK(engine.name() == "whisper.cpp");
}

TEST_CASE("without a temporary directory to work in, the run fails and starts nothing",
          "[whisper][engine]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};
    const char* before = std::getenv("TMPDIR");
    const std::string saved = before != nullptr ? before : "";
    ::setenv("TMPDIR", "/nonexistent/directory", 1);

    const auto done = engine.transcribe({.audio = setup.audio, .model = setup.multilingual});

    if (before != nullptr)
        ::setenv("TMPDIR", saved.c_str(), 1);
    else
        ::unsetenv("TMPDIR");
    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == TranscriptionErrorKind::Failed);
}

// ## How far it is

TEST_CASE("the segments it prints tell how far it is, long before its own progress does",
          "[whisper][engine][progress]") {
    const ScratchDirectory scratch;
    const std::filesystem::path printed =
        scratch.write("printed",
                      "[00:00:00.000 --> 00:00:20.000]   First.\n"
                      "[00:00:20.000 --> 00:00:50.000]   Second.\n"
                      "[00:00:50.000 --> 00:01:20.000]   Third.\n");
    const std::filesystem::path banner = scratch.write(
        "banner", "main: processing 'a.wav' (1600000 samples, 100.0 sec), 4 threads\n");
    const std::filesystem::path executable =
        scratch.standIn("whisper-cli",
                        kFake,
                        "--no-echo --stdout-from='" + printed.string() + "' --stderr-from='" +
                            banner.string() + "' --output-from='" + kSample + "'");
    const std::filesystem::path audio = scratch.write("audio.wav", "audio");
    const std::filesystem::path model = scratch.write("m.bin", header(51865));
    const WhisperCppEngine engine{executable};
    std::vector<subedit::core::TranscriptionProgress> states;

    const auto done = engine.transcribe(
        {.audio = audio, .model = model},
        {.advanced = [&states](const subedit::core::TranscriptionProgress& state) {
            states.push_back(state);
        }});

    REQUIRE(done.has_value());
    // The start, the three segments, the end.
    REQUIRE(states.size() == 5);
    CHECK_FALSE(states.front().fraction.has_value());
    CHECK(states[1].fraction == 0.2);
    CHECK(states[2].fraction == 0.5);
    CHECK(states[3].fraction == 0.8);
    CHECK(states[3].position == Timestamp::fromMilliseconds(80000));
    CHECK(states[3].total == Timestamp::fromMilliseconds(100000));
    CHECK(states[3].lastText == "Third.");
    CHECK(states.back().fraction == 1.0);
}

TEST_CASE("the fraction never goes back, and never reaches 1 before the end",
          "[whisper][engine][progress]") {
    const ScratchDirectory scratch;
    // The tool's own progress says 50 % after a segment that stands for 20 %, and the audio is
    // shorter than its last segment says.
    const std::filesystem::path printed = scratch.write("printed",
                                                        "[00:00:00.000 --> 00:00:20.000]   A.\n"
                                                        "[00:00:20.000 --> 00:02:00.000]   B.\n");
    const std::filesystem::path banner = scratch.write(
        "banner", "main: processing 'a.wav' (1600000 samples, 100.0 sec), 4 threads\n");
    const std::filesystem::path executable = scratch.standIn(
        "whisper-cli",
        kFake,
        "--no-echo --progress=2 --stdout-from='" + printed.string() + "' --stderr-from='" +
            banner.string() + "' --output-from='" + kSample + "'");
    const std::filesystem::path audio = scratch.write("audio.wav", "audio");
    const std::filesystem::path model = scratch.write("m.bin", header(51865));
    const WhisperCppEngine engine{executable};
    std::vector<double> fractions;

    const auto done = engine.transcribe(
        {.audio = audio, .model = model},
        {.advanced = [&fractions](const subedit::core::TranscriptionProgress& state) {
            if (state.fraction)
                fractions.push_back(*state.fraction);
        }});

    REQUIRE(done.has_value());
    REQUIRE_FALSE(fractions.empty());
    for (std::size_t index = 1; index < fractions.size(); ++index)
        CHECK(fractions[index] >= fractions[index - 1]);
    CHECK(fractions.back() == 1.0);
    // Before the last report, the segment past the end of the audio stops short of 1.
    CHECK(fractions[fractions.size() - 2] <= 1.0);
}

// ## A range

TEST_CASE("a range is handed to the tool as an offset and a duration", "[whisper][engine][range]") {
    const Setup setup{"--output-from='" + kSample + "'"};
    const std::filesystem::path log = setup.scratch.file("arguments");
    const auto spy =
        setup.scratch.write("spy-cli",
                            "#!/bin/sh\nprintf '%s\\n' \"$@\" > '" + log.string() + "'\nexec '" +
                                setup.executable.string() + "' \"$@\"\n");
    std::filesystem::permissions(
        spy, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    const WhisperCppEngine engine{spy};

    const auto done = engine.transcribe({.audio = setup.audio,
                                         .model = setup.multilingual,
                                         .start = Timestamp::fromMilliseconds(30000),
                                         .end = Timestamp::fromMilliseconds(90000)});

    REQUIRE(done.has_value());
    std::ifstream written{log};
    const std::string arguments{std::istreambuf_iterator<char>{written},
                                std::istreambuf_iterator<char>{}};
    CHECK(arguments.find("-ot\n30000\n") != std::string::npos);
    // Sounds are not written as notes, in parentheses or in brackets.
    CHECK(arguments.find("-sns\n") != std::string::npos);
    CHECK(arguments.find("-d\n60000\n") != std::string::npos);

    // Without a range, neither is passed.
    const auto whole = engine.transcribe({.audio = setup.audio, .model = setup.multilingual});
    REQUIRE(whole.has_value());
    std::ifstream again{log};
    const std::string plain{std::istreambuf_iterator<char>{again},
                            std::istreambuf_iterator<char>{}};
    CHECK(plain.find("-ot") == std::string::npos);
    CHECK(plain.find("-d\n") == std::string::npos);
}

TEST_CASE("what lies outside the range is cut off, because the tool decodes whole windows",
          "[whisper][engine][range]") {
    const Setup setup;
    const WhisperCppEngine engine{setup.executable};

    // The sample is one segment from the start to 10.5 s.
    const auto cut = engine.transcribe({.audio = setup.audio,
                                        .model = setup.multilingual,
                                        .end = Timestamp::fromMilliseconds(5000)});
    REQUIRE(cut.has_value());
    REQUIRE(cut->segments.size() == 1);
    CHECK(cut->segments.front().end == Timestamp::fromMilliseconds(5000));
    for (const auto& word : cut->segments.front().words)
        CHECK(word.end <= Timestamp::fromMilliseconds(5000));
    CHECK(cut->segments.front().words.size() < 22);

    const auto before = engine.transcribe({.audio = setup.audio,
                                           .model = setup.multilingual,
                                           .start = Timestamp::fromMilliseconds(20000)});
    REQUIRE(before.has_value());
    CHECK(before->segments.empty());
}

TEST_CASE("without the banner the position is known and the fraction is not",
          "[whisper][engine][progress]") {
    const ScratchDirectory scratch;
    const std::filesystem::path printed =
        scratch.write("printed", "[00:00:00.000 --> 00:00:20.000]   First.\n");
    const std::filesystem::path executable = scratch.standIn(
        "whisper-cli",
        kFake,
        "--no-echo --stdout-from='" + printed.string() + "' --output-from='" + kSample + "'");
    const std::filesystem::path audio = scratch.write("audio.wav", "audio");
    const std::filesystem::path model = scratch.write("m.bin", header(51865));
    const WhisperCppEngine engine{executable};
    std::vector<subedit::core::TranscriptionProgress> states;

    const auto done = engine.transcribe(
        {.audio = audio, .model = model},
        {.advanced = [&states](const subedit::core::TranscriptionProgress& state) {
            states.push_back(state);
        }});

    REQUIRE(done.has_value());
    REQUIRE(states.size() == 3);
    CHECK(states[1].position == Timestamp::fromMilliseconds(20000));
    CHECK_FALSE(states[1].fraction.has_value());
    CHECK_FALSE(states[1].total.has_value());
}
