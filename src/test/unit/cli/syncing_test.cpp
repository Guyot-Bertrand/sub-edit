// `sync`, without the binary: a fake engine that hears what the alignment cases say was said.
// The end-to-end tests hold the requirements; these hold the branches.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/syncing.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "core/transcription/alignment_cases.hpp"
#include "core/transcription/fake_transcription_engine.hpp"
#include "core/transcription/scratch_directory.hpp"

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::SyncOptions;
using subedit::core::AlignmentSettings;
using subedit::core::EngineCapabilities;
using subedit::core::EngineRegistry;
using subedit::core::RealFileSystem;
using subedit::core::TranscriptionErrorKind;
using subedit::core::TranscriptionSettings;
using subedit::test::FakeEngineScript;
using subedit::test::FakeTranscriptionEngine;
using subedit::test::ScratchDirectory;

/// An engine that always fails the same way.
class FailingEngine final : public subedit::core::TranscriptionEngine {
public:
    explicit FailingEngine(TranscriptionErrorKind kind) : m_kind(kind) {}

    [[nodiscard]] std::string_view name() const override { return "whisper.cpp"; }

    [[nodiscard]] std::expected<EngineCapabilities, subedit::core::TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& /*model*/) const override {
        return EngineCapabilities{
            .timedWords = true, .languageDetection = true, .needsModel = false};
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

std::string readAll(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

struct World {
    ScratchDirectory scratch;
    RealFileSystem files;
    std::ostringstream errors;
    std::ostringstream records;
    EngineRegistry engines;
    TranscriptionSettings transcription;
    AlignmentSettings alignment{.method = subedit::core::AlignmentMethod::Words};
    std::filesystem::path audio = scratch.write("voice.wav", "audio");
    std::filesystem::path video = scratch.write("film.mkv", "video");
    std::filesystem::path subtitles;
    std::filesystem::path output = scratch.file("out/film.srt");

    /// Where the programs are looked for: nowhere, unless a test puts some there.
    std::string searchPath;

    explicit World(const std::string& name, bool voiceActivity = true) {
        const subedit::test::AlignmentCase wanted = subedit::test::loadAlignmentCase(name);
        subtitles = scratch.write("film.srt",
                                  readAll(std::filesystem::path{SUBEDIT_TEST_DATA_DIR} /
                                          "alignment" / name / "given.srt"));
        const auto heard = subedit::core::readWhisperJson(readAll(wanted.heard));
        FakeEngineScript script;
        script.name = "whisper.cpp";
        script.capabilities = {.timedWords = true,
                               .languageDetection = true,
                               .voiceActivity = voiceActivity,
                               .needsModel = false};
        script.segments = heard ? heard->transcription.segments
                                : std::vector<subedit::core::TranscribedSegment>{};
        (void)engines.add(std::make_unique<FakeTranscriptionEngine>(script));
    }

    /// A file written here, and an engine that hears these words.
    World(const std::string& srt, const std::vector<subedit::core::TranscribedSegment>& heard) {
        subtitles = scratch.write("film.srt", srt);
        FakeEngineScript script;
        script.name = "whisper.cpp";
        script.capabilities = {.timedWords = true, .languageDetection = true, .needsModel = false};
        script.segments = heard;
        (void)engines.add(std::make_unique<FakeTranscriptionEngine>(script));
    }

    ExitCode run(const SyncOptions& options, std::size_t inputs = 1) {
        const Reporter reporter = Reporter{errors, 1}.withRecords(records).forCommand("sync");
        const std::vector<std::string> paths(inputs, subtitles.string());
        return syncAll(files,
                       engines,
                       searchPath,
                       paths,
                       std::nullopt,
                       options,
                       transcription,
                       alignment,
                       destinationFor(inputs),
                       reporter);
    }

    /// One file goes to `--output`; several go to a directory, as the command line has it.
    Destination destinationFor(std::size_t inputs) const {
        return inputs == 1
                   ? Destination::from(output.string(), "", false, inputs).value()
                   : Destination::from("", scratch.file("many").string(), false, inputs).value();
    }

    SyncOptions listening() const { return SyncOptions{.audio = audio.string()}; }
};

subedit::core::TranscribedSegment heardWords(std::int64_t start,
                                             const std::vector<std::string>& words) {
    constexpr std::int64_t kWord = 300;
    subedit::core::TranscribedSegment made{
        .text = {},
        .start = subedit::core::Timestamp::fromMilliseconds(start),
        .end = subedit::core::Timestamp::fromMilliseconds(
            start + (kWord * static_cast<std::int64_t>(words.size())))};
    std::int64_t at = start;
    for (const std::string& word : words) {
        made.words.push_back({.text = " " + word,
                              .start = subedit::core::Timestamp::fromMilliseconds(at),
                              .end = subedit::core::Timestamp::fromMilliseconds(at + kWord)});
        made.text += " " + word;
        at += kWord;
    }
    return made;
}

} // namespace

TEST_CASE("a file is re-timed on an audio file, as it is when ffmpeg is not there", "[cli][sync]") {
    World world{"offset-plus-2400"};

    const ExitCode code = world.run(world.listening());

    CHECK(code == ExitCode::Success);
    CHECK_THAT(readAll(world.output), ContainsSubstring("00:00:01,000 --> 00:00:02,960"));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"placed\":12"));
    CHECK_THAT(world.errors.str(), ContainsSubstring("12 of 12 subtitles placed on the audio"));
}

TEST_CASE("the media is said under the element, after the subtitle file it re-times",
          "[cli][sync]") {
    World world{"offset-plus-2400"};

    REQUIRE(world.run(world.listening()) == ExitCode::Success);

    const std::string said = world.errors.str();
    CHECK(said.starts_with("[subedit] [sync] [1/1] source: " + world.subtitles.string() + "\n" +
                           "[subedit] [sync] [1/1] media: " + world.audio.string() + "\n"));
    CHECK_THAT(said, ContainsSubstring("[subedit] [sync] [1/1] result: 12 of 12 subtitles placed"));
    CHECK(said.ends_with("[subedit] [sync] [1/1] dest: " + world.output.string() + "\n"));
}

TEST_CASE("a refusal of the options is about no element", "[cli][sync]") {
    World world{"offset-plus-2400"};

    CHECK(world.run(SyncOptions{}) == ExitCode::Usage);

    CHECK(world.errors.str().starts_with("[subedit] [sync] [-] error: "));
    CHECK_FALSE(world.errors.str().contains("source:"));
}

TEST_CASE("the audio must come from a film or from a file, and not from both", "[cli][sync]") {
    World world{"offset-plus-2400"};

    CHECK(world.run(SyncOptions{}) == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("give --video or --audio"));

    CHECK(world.run(SyncOptions{.video = world.video.string(), .audio = world.audio.string()}) ==
          ExitCode::Usage);
}

TEST_CASE("a report names one file", "[cli][sync]") {
    World world{"offset-plus-2400"};
    SyncOptions options = world.listening();
    options.report = world.scratch.file("report.txt").string();

    CHECK(world.run(options, 2) == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("--report names one file"));
}

TEST_CASE("an engine nobody registered, and an alignment engine nobody knows, are usage errors",
          "[cli][sync]") {
    World world{"offset-plus-2400"};
    SyncOptions options = world.listening();
    options.engine = "other";

    CHECK(world.run(options) == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("no transcription engine is called other"));

    world.alignment.engine = "nobody";
    CHECK(world.run(world.listening()) == ExitCode::Usage);
    CHECK_THAT(world.errors.str(), ContainsSubstring("there is no alignment engine called nobody"));
}

TEST_CASE("a language that is not a code, and a range that is not one, are usage errors",
          "[cli][sync]") {
    World world{"offset-plus-2400"};
    SyncOptions language = world.listening();
    language.language = "english";
    CHECK(world.run(language) == ExitCode::Usage);

    SyncOptions range = world.listening();
    range.start = "1:00";
    range.end = "0:30";
    CHECK(world.run(range) == ExitCode::Usage);
}

TEST_CASE("a voice-activity model is refused by name for an engine that cannot use it, and when "
          "it is not there",
          "[cli][sync]") {
    World without{"offset-plus-2400", false};
    SyncOptions asked = without.listening();
    asked.vadModel = without.scratch.write("vad.bin", "vad").string();
    CHECK(without.run(asked) == ExitCode::Usage);

    World with{"offset-plus-2400", true};
    SyncOptions missing = with.listening();
    missing.vadModel = with.scratch.file("absent.bin").string();
    CHECK(with.run(missing) == ExitCode::Usage);

    // A setting only says where one is, and is dropped without a word for an engine that cannot.
    without.transcription.vadModel = without.scratch.write("vad2.bin", "vad");
    CHECK(without.run(without.listening()) == ExitCode::Success);
}

TEST_CASE("only the subtitles that begin in the range are looked for", "[cli][sync]") {
    World world{"offset-plus-2400"};
    SyncOptions options = world.listening();
    options.start = "0:20";

    CHECK(world.run(options) == ExitCode::Success);

    const std::string written = readAll(world.output);
    CHECK_THAT(written, ContainsSubstring("00:00:03,400 --> 00:00:05,400"));
    CHECK_THAT(written, ContainsSubstring("00:00:42,000 --> 00:00:44,960"));
}

TEST_CASE("the options go over the settings, and the lines to check are reported", "[cli][sync]") {
    World world{"unheard-line"};
    SyncOptions options = world.listening();
    options.minimumMatchPercent = 100;
    options.doubtGapMilliseconds = 1;
    options.report = world.scratch.file("out/to-check.txt").string();

    CHECK(world.run(options) == ExitCode::Success);

    CHECK_THAT(readAll(options.report), ContainsSubstring("9\tnot found in the audio"));
    CHECK_THAT(readAll(options.report), ContainsSubstring("cut back"));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"not-found\""));
}

TEST_CASE("a report that cannot be written is said, and the file is still written", "[cli][sync]") {
    World world{"unheard-line"};
    SyncOptions options = world.listening();
    // Below a file: no directory can be made there.
    options.report = (world.audio / "report.txt").string();

    CHECK(world.run(options) == ExitCode::Success);

    CHECK_THAT(world.errors.str(), ContainsSubstring("report.txt"));
    CHECK(std::filesystem::exists(world.output));
}

TEST_CASE("a file none of whose subtitles is found is refused and not written", "[cli][sync]") {
    World world{"offset-plus-2400"};
    world.engines = EngineRegistry{};
    FakeEngineScript script;
    script.name = "whisper.cpp";
    script.capabilities = {.timedWords = true, .languageDetection = true, .needsModel = false};
    script.segments = {{.text = " Something else entirely.",
                        .start = subedit::core::Timestamp::fromMilliseconds(0),
                        .end = subedit::core::Timestamp::fromMilliseconds(2000),
                        .words = {{.text = " Something",
                                   .start = subedit::core::Timestamp::fromMilliseconds(0),
                                   .end = subedit::core::Timestamp::fromMilliseconds(900)}}}};
    (void)world.engines.add(std::make_unique<FakeTranscriptionEngine>(script));

    CHECK(world.run(world.listening()) == ExitCode::AllFailed);

    CHECK_THAT(world.errors.str(), ContainsSubstring("was found in the audio"));
    CHECK_FALSE(std::filesystem::exists(world.output));
}

TEST_CASE("an engine that fails, and a film without ffmpeg, fail the file", "[cli][sync]") {
    World world{"offset-plus-2400"};
    world.engines = EngineRegistry{};
    (void)world.engines.add(std::make_unique<FailingEngine>(TranscriptionErrorKind::Failed));

    CHECK(world.run(world.listening()) == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("the transcription failed"));

    CHECK(world.run(SyncOptions{.video = world.video.string()}) == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("ffmpeg was not found"));
}

TEST_CASE("the extracted audio is kept where the option says, with the rate in its name",
          "[cli][sync]") {
    World world{"offset-plus-2400"};
    SyncOptions options = world.listening();
    options.keepAudio = world.scratch.file("kept/film.{rate}.wav").string();

    CHECK(world.run(options) == ExitCode::Success);
}

TEST_CASE("a partly heard subtitle, one that moved differently and one left overlapping are "
          "each flagged",
          "[cli][sync]") {
    const std::string srt = "1\n00:00:00,000 --> 00:00:01,000\nred apple tree\n\n"
                            "2\n00:00:03,000 --> 00:00:04,000\ngreen river stone\n\n"
                            "3\n00:00:06,000 --> 00:00:07,000\nblue mountain cloud walks\n\n"
                            "4\n00:00:09,000 --> 00:00:10,000\nyellow desert wind\n\n"
                            "5\n00:00:12,000 --> 00:00:13,000\npurple forest rain\n\n"
                            "6\n00:00:15,000 --> 00:00:16,000\norange valley snow\n\n"
                            "7\n00:00:18,000 --> 00:00:19,000\ngrey ocean\n";
    World world{srt,
                {heardWords(500, {"red", "apple", "tree"}),
                 heardWords(3500, {"green", "river", "stone"}),
                 heardWords(6500, {"blue", "mountain", "cloud"}),
                 heardWords(9500, {"yellow", "desert", "wind"}),
                 heardWords(20500, {"purple", "forest", "rain"}),
                 heardWords(23500, {"orange", "valley", "snow"})}};
    SyncOptions options = world.listening();
    options.model = world.scratch.write("model.bin", "model").string();

    CHECK(world.run(options) == ExitCode::Success);

    const std::string records = world.records.str();
    CHECK_THAT(records, ContainsSubstring("\"kind\":\"partial\""));
    CHECK_THAT(records, ContainsSubstring("\"kind\":\"out-of-line\""));
    CHECK_THAT(records, ContainsSubstring("\"kind\":\"not-found\""));
    CHECK_THAT(records, ContainsSubstring("\"kind\":\"overlaps\""));
}

TEST_CASE("an engine that cannot say what it can do fails the file", "[cli][sync]") {
    World world{"offset-plus-2400"};
    world.engines = EngineRegistry{};
    FakeEngineScript script;
    script.name = "whisper.cpp";
    script.capabilitiesError =
        subedit::core::TranscriptionError{.kind = TranscriptionErrorKind::EngineMissing};
    (void)world.engines.add(std::make_unique<FakeTranscriptionEngine>(script));

    CHECK(world.run(world.listening()) == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("the transcription engine was not found"));
}

TEST_CASE("the speech method is refused, before any work, without its program or its model",
          "[cli][sync]") {
    World world{"offset-plus-2400"};
    world.alignment.method = subedit::core::AlignmentMethod::Speech;
    const auto model = world.scratch.write("vad.bin", "model");

    // No model named.
    CHECK(world.run(world.listening()) == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("needs a voice-activity model"));

    // A model that is not there.
    SyncOptions missing = world.listening();
    missing.vadModel = world.scratch.file("nowhere.bin").string();
    CHECK(world.run(missing) == ExitCode::AllFailed);

    // A model, and nowhere to find the program (the search path of the harness is empty).
    SyncOptions noTool = world.listening();
    noTool.vadModel = model.string();
    CHECK(world.run(noTool) == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("whisper-vad-speech-segments"));
    // The pointer to the installation page on a line of its own, under the same key.
    CHECK_THAT(
        world.errors.str(),
        ContainsSubstring("it was not found.\n[subedit] [sync] [-] error: See the manual page"));
    CHECK_FALSE(std::filesystem::exists(world.output));
}

namespace {

std::string stamp(std::int64_t milliseconds) {
    std::array<char, 32> text{};
    std::snprintf(text.data(),
                  text.size(),
                  "%02d:%02d:%02d,%03d",
                  static_cast<int>(milliseconds / 3600000),
                  static_cast<int>((milliseconds / 60000) % 60),
                  static_cast<int>((milliseconds / 1000) % 60),
                  static_cast<int>(milliseconds % 1000));
    return text.data();
}

/// A film of speech and the file of subtitles for it, in no language the program knows: what the
/// voice detector prints, and a file whose times are `ratio` times the truth, `late` after it.
struct SpeechFilm {
    std::string detected = "Detected speech segments:\n";
    std::string srt;
    std::vector<std::int64_t> truth;

    SpeechFilm(double ratio, std::int64_t late) {
        constexpr int kLines = 60;
        std::uint32_t state = 777;
        std::int64_t now = 5000;
        for (int line = 0; line < kLines; ++line) {
            state = (state * 1664525U) + 1013904223U;
            const std::int64_t length = 1200 + ((state >> 8U) % 2800);
            state = (state * 1664525U) + 1013904223U;
            const std::int64_t pause = 600 + ((state >> 8U) % 2400);
            truth.push_back(now);
            detected += "Speech segment " + std::to_string(line) +
                        ": start = " + std::to_string(now / 10) +
                        ".00, end = " + std::to_string((now + length) / 10) + ".00\n";
            const auto moved = [&](std::int64_t at) {
                return static_cast<std::int64_t>(std::llround(static_cast<double>(at) * ratio)) +
                       late;
            };
            srt += std::to_string(line + 1) + "\n" + stamp(moved(now)) + " --> " +
                   stamp(moved(now + length)) + "\nsome words\n\n";
            now += length + pause;
        }
    }
};

/// A world where the voice detector is a script printing `detected`, and the file is `srt`.
void layOnSpeech(World& world, const std::string& detected, const std::string& srt) {
    world.alignment.method = subedit::core::AlignmentMethod::Speech;
    world.subtitles = world.scratch.write("film.srt", srt);
    const std::filesystem::path bin = world.scratch.file("bin");
    std::filesystem::create_directories(bin);
    const std::filesystem::path listing = world.scratch.write("segments.txt", detected);
    const std::filesystem::path tool = world.scratch.write(
        "bin/whisper-vad-speech-segments", "#!/bin/sh\ncat '" + listing.string() + "'\n");
    std::filesystem::permissions(
        tool, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    world.searchPath = bin.string();
}

SyncOptions onSpeech(const World& world) {
    SyncOptions options = world.listening();
    options.vadModel = world.scratch.write("vad.bin", "model").string();
    return options;
}

} // namespace

TEST_CASE("the speech method lays a file on when there is speech, and warns", "[cli][sync]") {
    World world{"offset-plus-2400"};
    const SpeechFilm film{1.0, 3000};
    layOnSpeech(world, film.detected, film.srt);
    SyncOptions options = onSpeech(world);
    options.keepAudio = world.scratch.file("kept.wav").string();

    const ExitCode code = world.run(options);

    CHECK(code == ExitCode::Success);
    const std::string written = readAll(world.output);
    for (const std::int64_t start : film.truth)
        CHECK_THAT(written, ContainsSubstring(stamp(start) + " --> "));
    CHECK_THAT(world.errors.str(), ContainsSubstring("Only the file as a whole was put in line"));
    CHECK_THAT(world.records.str(), ContainsSubstring("\"kind\":\"speech-only\""));
}

TEST_CASE("the speech method says when it put the frame rate of the file right", "[cli][sync]") {
    World world{"offset-plus-2400"};
    const SpeechFilm film{25.0 / 23.976, 0};
    layOnSpeech(world, film.detected, film.srt);

    const ExitCode code = world.run(onSpeech(world));

    CHECK(code == ExitCode::Success);
    CHECK_THAT(world.errors.str(), ContainsSubstring("The frame rate of the file was put right"));
}

TEST_CASE("the speech method refuses what it cannot lay, and what the program cannot do",
          "[cli][sync]") {
    World world{"offset-plus-2400"};
    const SpeechFilm film{1.0, 3000};

    // A voice detector that fails.
    layOnSpeech(world, film.detected, film.srt);
    (void)world.scratch.write("bin/whisper-vad-speech-segments",
                              "#!/bin/sh\necho 'error: no way' >&2\nexit 1\n");
    CHECK(world.run(onSpeech(world)) == ExitCode::AllFailed);
    CHECK_FALSE(std::filesystem::exists(world.output));

    // A film with almost no speech: the file has nothing to agree with.
    layOnSpeech(world, "Speech segment 0: start = 100.00, end = 110.00\n", film.srt);
    CHECK(world.run(onSpeech(world)) == ExitCode::AllFailed);
    CHECK_THAT(world.errors.str(), ContainsSubstring("too little in common"));
    CHECK_FALSE(std::filesystem::exists(world.output));

    // A range that cannot be.
    layOnSpeech(world, film.detected, film.srt);
    SyncOptions backwards = onSpeech(world);
    backwards.start = "1:00";
    backwards.end = "0:30";
    CHECK(world.run(backwards) == ExitCode::Usage);
}
