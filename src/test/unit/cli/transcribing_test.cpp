// `transcribe`, without the binary: the fake engine and stand-ins for ffmpeg and ffprobe.
// The end-to-end tests hold the requirements; these hold the branches.

#include <subedit/cli/reporter.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/core/io/real_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include "core/transcription/fake_transcription_engine.hpp"
#include "core/transcription/scratch_directory.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::TranscribeOptions;
using subedit::core::EngineCapabilities;
using subedit::core::EngineRegistry;
using subedit::core::RealFileSystem;
using subedit::core::Timestamp;
using subedit::core::TranscribedSegment;
using subedit::core::TranscriptionErrorKind;
using subedit::core::TranscriptionSettings;
using subedit::test::FakeEngineScript;
using subedit::test::FakeTranscriptionEngine;
using subedit::test::ScratchDirectory;

const std::filesystem::path kFake{SUBEDIT_FAKE_TRANSCRIBER_BINARY};

TranscribedSegment segment(std::int64_t start, std::int64_t end, std::string text) {
    return {.text = std::move(text),
            .start = Timestamp::fromMilliseconds(start),
            .end = Timestamp::fromMilliseconds(end)};
}

/// An engine that always fails the same way, to reach each error the command words.
class FailingEngine final : public subedit::core::TranscriptionEngine {
public:
    explicit FailingEngine(TranscriptionErrorKind kind) : m_kind(kind) {}

    [[nodiscard]] std::string_view name() const override { return "failing"; }

    [[nodiscard]] std::expected<EngineCapabilities, subedit::core::TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& /*model*/) const override {
        return EngineCapabilities{.languageDetection = true, .needsModel = false};
    }

protected:
    [[nodiscard]] std::expected<subedit::core::Transcription, subedit::core::TranscriptionError>
    run(const subedit::core::TranscriptionRequest& /*request*/,
        const EngineCapabilities& /*capabilities*/,
        const subedit::core::TranscriptionObserver& /*observer*/) const override {
        return std::unexpected(subedit::core::TranscriptionError{.kind = m_kind, .detail = "why"});
    }

private:
    TranscriptionErrorKind m_kind;
};

struct World {
    ScratchDirectory scratch;
    RealFileSystem files;
    std::ostringstream errors;
    std::ostringstream records;
    EngineRegistry engines;
    TranscriptionSettings settings;
    std::filesystem::path audio = scratch.write("voice.wav", "audio");
    std::filesystem::path video = scratch.write("film.mkv", "video");
    std::filesystem::path output = scratch.file("out/film.srt");

    World() {
        FakeEngineScript script;
        script.name = "whisper.cpp";
        script.capabilities = {.languageDetection = true, .needsModel = false};
        script.segments = {segment(0, 1500, "I was going to"), segment(1600, 3000, "tell you.")};
        (void)engines.add(std::make_unique<FakeTranscriptionEngine>(script));
    }

    ExitCode
    run(const TranscribeOptions& options, int level = 1, std::string_view searchPath = "") {
        const Reporter reporter =
            Reporter{errors, level}.withRecords(records).forCommand("transcribe");
        return transcribeFile(files, engines, searchPath, options, settings, reporter);
    }

    [[nodiscard]] std::string written() const {
        std::ifstream file{output, std::ios::binary};
        return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    }
};

} // namespace

TEST_CASE("an engine nobody registered is a usage error", "[cli][transcribe]") {
    World world;

    const ExitCode code = world.run(
        {.input = world.audio.string(), .output = world.output.string(), .engine = "other"});

    CHECK(code == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("no transcription engine is called other"));
}

TEST_CASE("an audio file is read as it is when ffmpeg is not there", "[cli][transcribe]") {
    World world;

    const ExitCode code = world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(world.written(), ContainsSubstring("I was going to tell you."));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"language\":\"en\""));
}

TEST_CASE("a video cannot be read without ffmpeg, and the guide is named", "[cli][transcribe]") {
    World world;

    const ExitCode code =
        world.run({.input = world.video.string(), .output = world.output.string()});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("ffmpeg was not found"));
    CHECK_THAT(world.errors.str(), ContainsSubstring("Installing the transcription tools"));
    CHECK_FALSE(std::filesystem::exists(world.output));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"tool-missing\""));
}

TEST_CASE("asking for a track needs ffmpeg even for an audio file", "[cli][transcribe]") {
    World world;

    const ExitCode code = world.run(
        {.input = world.audio.string(), .output = world.output.string(), .audioTrack = 1});

    CHECK(code == ExitCode::AllFailed);
}

TEST_CASE("a track the video lacks is named with the tracks it has", "[cli][transcribe]") {
    World world;
    const std::string probe =
        std::string{SUBEDIT_TEST_DATA_DIR} + "/audio-tracks/ffprobe.audio-2.txt";
    (void)world.scratch.standIn("ffprobe", kFake, "--no-echo --stdout-from='" + probe + "'");
    (void)world.scratch.standIn("ffmpeg", kFake, "");

    const ExitCode code =
        world.run({.input = world.video.string(), .output = world.output.string(), .audioTrack = 4},
                  1,
                  world.scratch.path().string());

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("the video has no audio track 4"));
    CHECK_THAT(world.errors.str(), ContainsSubstring("audio track 1: eng, Commentary"));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"no-such-track\""));
}

TEST_CASE("a failed extraction is told with its kind", "[cli][transcribe]") {
    World world;
    (void)world.scratch.standIn("ffmpeg", kFake, "--fail-with=1");

    const ExitCode code =
        world.run({.input = world.video.string(), .output = world.output.string()},
                  1,
                  world.scratch.path().string());

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"audio-extraction-failed\""));
}

TEST_CASE("the segments are merged unless --raw asks for them as they came", "[cli][transcribe]") {
    World world;

    REQUIRE(world.run(TranscribeOptions{.input = world.audio.string(),
                                        .output = world.output.string()}) == ExitCode::Success);
    const std::string merged = world.written();
    REQUIRE(
        world.run({.input = world.audio.string(), .output = world.output.string(), .raw = true}) ==
        ExitCode::Success);

    CHECK_THAT(merged, ContainsSubstring("I was going to tell you."));
    CHECK_THAT(world.written(), ContainsSubstring("I was going to\n"));
}

TEST_CASE("the format comes from --to, else from the extension, else SubRip", "[cli][transcribe]") {
    World world;
    const auto write = [&](const std::string& name, const std::string& format) {
        world.output = world.scratch.file(name);
        REQUIRE(world.run({.input = world.audio.string(),
                           .output = world.output.string(),
                           .format = format}) == ExitCode::Success);
        return world.written();
    };

    CHECK_THAT(write("a.vtt", ""), ContainsSubstring("WEBVTT"));
    CHECK_THAT(write("b.VTT", ""), ContainsSubstring("WEBVTT"));
    CHECK_THAT(write("c.xyz", ""), ContainsSubstring("-->"));
    CHECK_THAT(write("d.srt", "vtt"), ContainsSubstring("WEBVTT"));
}

TEST_CASE("each way an engine can fail has an identifier a script can test", "[cli][transcribe]") {
    const std::pair<TranscriptionErrorKind, std::string> kinds[] = {
        {TranscriptionErrorKind::EngineMissing, "engine-missing"},
        {TranscriptionErrorKind::ModelMissing, "model-missing"},
        {TranscriptionErrorKind::AudioUnreadable, "audio-unreadable"},
        {TranscriptionErrorKind::Failed, "failed"},
        {TranscriptionErrorKind::Cancelled, "cancelled"},
    };
    for (const auto& [kind, identifier] : kinds) {
        World world;
        world.engines = EngineRegistry{};
        (void)world.engines.add(std::make_unique<FailingEngine>(kind));

        const ExitCode code = world.run(
            {.input = world.audio.string(), .output = world.output.string(), .engine = "failing"});

        CHECK(code == ExitCode::AllFailed);
        CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"" + identifier + "\""));
    }
}

TEST_CASE("a language the engine cannot take, and one it cannot detect, are named",
          "[cli][transcribe]") {
    World world;
    FakeEngineScript script;
    script.name = "whisper.cpp";
    script.capabilities = {.languages = {"en", "fr"}, .needsModel = false};
    world.engines = EngineRegistry{};
    (void)world.engines.add(std::make_unique<FakeTranscriptionEngine>(script));

    (void)world.run(
        {.input = world.audio.string(), .output = world.output.string(), .language = "de"});
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"unsupported-language\""));

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"language-required\""));
}

TEST_CASE(
    "the progress is narrated at the second level, the model and language of the settings are used",
    "[cli][transcribe]") {
    World world;
    world.settings.language = "fr";
    world.settings.model = world.scratch.write("m.bin", "model");

    const ExitCode code = world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()}, 2);

    CHECK(code == ExitCode::Success);
    CHECK_THAT(world.errors.str(), ContainsSubstring("transcribing: 100%"));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"language\":\"fr\""));
}

TEST_CASE("a subtitle file that cannot be written fails the file", "[cli][transcribe]") {
    World world;
    const auto blocker = world.scratch.write("blocker", "a file, not a directory");
    world.output = blocker / "film.srt";

    const ExitCode code = world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});

    CHECK(code == ExitCode::AllFailed);
}

TEST_CASE("the extracted audio is kept where --keep-audio says, directories made",
          "[cli][transcribe]") {
    World world;
    const auto sample = world.scratch.write("sample.wav", "pcm");
    (void)world.scratch.standIn("ffmpeg", kFake, "--output-from='" + sample.string() + "'");
    const auto kept = world.scratch.file("deep/er/audio.wav");

    const ExitCode code = world.run({.input = world.video.string(),
                                     .output = world.output.string(),
                                     .keepAudio = kept.string()},
                                    1,
                                    world.scratch.path().string());

    CHECK(code == ExitCode::Success);
    CHECK(std::filesystem::exists(kept));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"audio\":\"" + kept.string() + "\""));
}
