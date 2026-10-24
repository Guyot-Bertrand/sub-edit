// `transcribe`, without the binary: the fake engine and stand-ins for ffmpeg and ffprobe.
// The end-to-end tests hold the requirements; these hold the branches.

#include <subedit/cli/reporter.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/core/io/real_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <chrono>
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
using subedit::cli::ProgressNarration;
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

    std::shared_ptr<subedit::core::TranscriptionRequest> seen =
        std::make_shared<subedit::core::TranscriptionRequest>();

    World() {
        FakeEngineScript script;
        script.seen = seen;
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
    const std::array<std::pair<TranscriptionErrorKind, std::string>, 5> kinds{{
        {TranscriptionErrorKind::EngineMissing, "engine-missing"},
        {TranscriptionErrorKind::ModelMissing, "model-missing"},
        {TranscriptionErrorKind::AudioUnreadable, "audio-unreadable"},
        {TranscriptionErrorKind::Failed, "failed"},
        {TranscriptionErrorKind::Cancelled, "cancelled"},
    }};
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

TEST_CASE("a transcription that is stopped fails the file with a cancellation",
          "[cli][transcribe]") {
    World world;
    const auto sample = world.scratch.write("sample.wav", "pcm");
    (void)world.scratch.standIn("ffmpeg", kFake, "--linger=30");
    const Reporter reporter =
        Reporter{world.errors, 1}.withRecords(world.records).forCommand("transcribe");

    const ExitCode code = subedit::cli::transcribeFile(
        world.files,
        world.engines,
        world.scratch.path().string(),
        TranscribeOptions{.input = world.video.string(), .output = world.output.string()},
        world.settings,
        reporter,
        [] { return true; });

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"cancelled\""));
    (void)sample;
}

// ## How far it is, and a range

TEST_CASE("the third level adds the text obtained, once per new text", "[cli][transcribe]") {
    World world;
    world.settings.language = "fr";
    world.settings.model = world.scratch.write("m.bin", "model");

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()}, 3);

    const std::string said = world.errors.str();
    CHECK_THAT(said, ContainsSubstring("last text: I was going to"));
    // Four reports of the same text, said once.
    CHECK(said.find("last text:") == said.rfind("last text:"));
}

TEST_CASE("the second level says each percent once, with the step", "[cli][transcribe]") {
    World world;
    world.settings.language = "fr";

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()}, 2);

    const std::string said = world.errors.str();
    CHECK_THAT(said, ContainsSubstring("transcribing: 25%"));
    CHECK_THAT(said, ContainsSubstring("transcribing: 100%"));
    CHECK(said.find("transcribing: 50%") == said.rfind("transcribing: 50%"));
}

TEST_CASE("a range reaches the engine, in the time of the film", "[cli][transcribe][range]") {
    World world;
    world.settings.language = "fr";

    const ExitCode code = world.run(TranscribeOptions{.input = world.audio.string(),
                                                      .output = world.output.string(),
                                                      .start = "2:00",
                                                      .end = "210"});

    CHECK(code == ExitCode::Success);
    CHECK(world.seen->start == subedit::core::Timestamp::fromMilliseconds(120000));
    CHECK(world.seen->end == subedit::core::Timestamp::fromMilliseconds(210000));
}

TEST_CASE("no range is no bound", "[cli][transcribe][range]") {
    World world;
    world.settings.language = "fr";

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});

    CHECK_FALSE(world.seen->start.has_value());
    CHECK_FALSE(world.seen->end.has_value());
}

TEST_CASE("a bound that is no time, or a range with its end first, is a usage error",
          "[cli][transcribe][range]") {
    World world;
    world.settings.language = "fr";

    CHECK(world.run(TranscribeOptions{.input = world.audio.string(),
                                      .output = world.output.string(),
                                      .start = "soon"}) == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("\"soon\" is not a time"));

    CHECK(world.run(TranscribeOptions{.input = world.audio.string(),
                                      .output = world.output.string(),
                                      .end = "1:xx"}) == ExitCode::Usage);

    CHECK(world.run(TranscribeOptions{.input = world.audio.string(),
                                      .output = world.output.string(),
                                      .start = "3:00",
                                      .end = "2:00"}) == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("the end of the range must come after"));
    // Nothing was written, and the engine was never asked.
    CHECK_FALSE(std::filesystem::exists(world.output));
    CHECK(world.seen->audio.empty());
}

TEST_CASE("a line every ten seconds says the work goes on when nothing new has come",
          "[cli][transcribe]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 2};
    auto now = ProgressNarration::Clock::time_point{};
    ProgressNarration narration{reporter, [&now] { return now; }};

    narration({.fraction = 0.25});
    const std::string first = errors.str();
    CHECK_THAT(first, ContainsSubstring("transcribing: 25%"));

    // Nothing new, nine seconds on: silence.
    now += std::chrono::seconds{9};
    narration.idle();
    CHECK(errors.str() == first);

    // Ten seconds on: the same state, with the time spent.
    now += std::chrono::seconds{1};
    narration.idle();
    CHECK_THAT(errors.str(), ContainsSubstring("transcribing: 25%, elapsed: 0:00:10"));

    // And the next one waits ten more.
    const std::string second = errors.str();
    now += std::chrono::seconds{5};
    narration.idle();
    CHECK(errors.str() == second);
    now += std::chrono::seconds{5};
    narration.idle();
    CHECK_THAT(errors.str(), ContainsSubstring("elapsed: 0:00:20"));
}

TEST_CASE("before anything is known, waiting says nothing", "[cli][transcribe]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 2};
    auto now = ProgressNarration::Clock::time_point{};
    ProgressNarration narration{reporter, [&now] { return now; }};

    now += std::chrono::minutes{5};
    narration.idle();

    CHECK(errors.str().empty());
}

TEST_CASE("a language is read as a two-letter code, or as a three-letter one",
          "[cli][transcribe]") {
    World world;

    (void)world.run(TranscribeOptions{
        .input = world.audio.string(), .output = world.output.string(), .language = "fra"});
    CHECK(world.seen->language == "fr");

    (void)world.run(TranscribeOptions{
        .input = world.audio.string(), .output = world.output.string(), .language = " EN "});
    CHECK(world.seen->language == "en");

    // A code the table does not hold is kept: the engine knows more languages than the table.
    (void)world.run(TranscribeOptions{
        .input = world.audio.string(), .output = world.output.string(), .language = "af"});
    CHECK(world.seen->language == "af");
}

TEST_CASE("something that is not a language code is a usage error before any work",
          "[cli][transcribe]") {
    World world;

    for (const std::string typed : {"french", "f", "f1", "français"}) {
        const ExitCode code = world.run(TranscribeOptions{
            .input = world.audio.string(), .output = world.output.string(), .language = typed});
        CHECK(code == ExitCode::Usage);
    }
    CHECK_THAT(world.errors.str(), ContainsSubstring("is not a language code"));
    CHECK(world.seen->audio.empty());
    CHECK_FALSE(std::filesystem::exists(world.output));
}

TEST_CASE("a transcription made of notes in brackets is said to be probably in the wrong language",
          "[cli][transcribe]") {
    World world;
    FakeEngineScript script;
    script.name = "whisper.cpp";
    script.capabilities = {.languageDetection = true, .needsModel = false};
    script.segments = {segment(0, 1000, "[MUSIC PLAYING]"),
                       segment(1000, 2000, "[SPEAKING FRENCH]"),
                       segment(2000, 3000, "Hello.")};
    world.engines = EngineRegistry{};
    (void)world.engines.add(std::make_unique<FakeTranscriptionEngine>(script));

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});

    CHECK_THAT(world.errors.str(), ContainsSubstring("2 of the 3 pieces are only notes"));
}

TEST_CASE("a model for English alone is said so before the work", "[cli][transcribe]") {
    World world;
    FakeEngineScript script;
    script.name = "whisper.cpp";
    script.capabilities = {.languages = {"en"}, .needsModel = false};
    script.segments = {segment(0, 1000, "Hello.")};
    world.engines = EngineRegistry{};
    (void)world.engines.add(std::make_unique<FakeTranscriptionEngine>(script));

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});

    CHECK_THAT(world.errors.str(), ContainsSubstring("This model is for English only"));
}

TEST_CASE("a model that speaks every language is not warned about", "[cli][transcribe]") {
    World world;
    world.settings.language = "fr";

    (void)world.run(
        TranscribeOptions{.input = world.audio.string(), .output = world.output.string()});

    CHECK_THAT(world.errors.str(), !ContainsSubstring("English only"));
    CHECK_THAT(world.errors.str(), !ContainsSubstring("only notes"));
}
