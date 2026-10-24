// `transcribe`, through the real binary, against stand-ins for `whisper-cli`, `ffmpeg` and
// `ffprobe` that the test puts at the front of the search path: the fake transcriber, which
// writes a sample of whisper.cpp's real output where the tool would.
//
// What is read back is worked out from the sample, never from the program.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::invokeWithSearchPath;
using subedit::e2e::Scratch;
using subedit::e2e::writeFile;

namespace {

const std::string kFake = SUBEDIT_FAKE_TRANSCRIBER_BINARY;
const std::string kData = SUBEDIT_TEST_DATA_DIR;

/// The words of the sample, as whisper wrote them.
const std::string kSentence = "And so, my fellow Americans, ask not what your country can do for "
                              "you, ask what you can do for your country.";

/// The first bytes of a multilingual model, and of an English-only one.
std::string modelHeader(unsigned vocabulary) {
    std::string bytes = "lmgg";
    for (int index = 0; index < 4; ++index)
        bytes += static_cast<char>((vocabulary >> (8 * index)) & 0xFFU);
    return bytes + std::string(8, '\0');
}

/// Two segments a tenth of a second apart, the first not ending its sentence.
const std::string kTwoClosePieces = R"({
  "model": {"multilingual": true},
  "result": {"language": "en"},
  "transcription": [
    {"text": " I was going to", "offsets": {"from": 0, "to": 1500}},
    {"text": " tell you something.", "offsets": {"from": 1600, "to": 3000}}
  ]})";

/// A directory holding the three stand-ins, and an audio sample and a model.
struct Stands {
    const Scratch scratch;
    std::string directory = scratch.of("bin");
    std::string model = writeFile(scratch, "model.bin", modelHeader(51865));
    std::string video = writeFile(scratch, "film.mkv", "not really a video");
    std::string argumentLog = scratch.of("whisper-arguments");

    void script(const std::string& name, const std::string& body) {
        const std::string path = writeFile(scratch, "bin/" + name, "#!/bin/sh\n" + body + "\n");
        std::filesystem::permissions(
            path, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    }

    /// `whisper-cli` that logs its arguments and writes `sample` as its result.
    void whisper(const std::string& sample, const std::string& knobs = "--progress=4") {
        script("whisper-cli",
               "printf '%s\\n' \"$@\" > '" + argumentLog + "'\nexec '" + kFake + "' " + knobs +
                   " --output-from='" + sample + "' \"$@\"");
    }

    void ffprobe() {
        script("ffprobe",
               "exec '" + kFake + "' --no-echo --stdout-from='" + kData +
                   "/audio-tracks/ffprobe.audio-2.txt' \"$@\"");
    }

    void ffmpeg() {
        const std::string audio = writeFile(scratch, "sample.wav", "pcm");
        script("ffmpeg", "exec '" + kFake + "' --output-from='" + audio + "' \"$@\"");
    }

    void everything() {
        whisper(kData + "/whisper/jfk.full.json");
        ffprobe();
        ffmpeg();
    }

    CliRun run(std::vector<std::string> arguments) const {
        std::vector<std::string> all{"transcribe"};
        all.insert(all.end(), arguments.begin(), arguments.end());
        return invokeWithSearchPath(directory, all);
    }
};

} // namespace

TEST_CASE("a video is transcribed into a subtitle file in the format its name says",
          "[e2e][CLI-TRANSCRIBE-01]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun run = stands.run({stands.video, "--output", out, "--model", stands.model});

    REQUIRE(run.exitCode == 0);
    const std::string written = contentOf(out);
    CHECK_THAT(written, ContainsSubstring("1\n00:00:00,320 --> "));
    CHECK_THAT(written, ContainsSubstring("And so, my fellow Americans"));
    CHECK_THAT(written, ContainsSubstring("for your country.\n"));
    // Longer than a subtitle holds, so it came out in more than one.
    CHECK_THAT(written, ContainsSubstring("\n2\n00:00:"));
    CHECK_THAT(written, ContainsSubstring(" --> 00:00:10,500\n"));
}

TEST_CASE("--to writes another format than the extension says", "[e2e][CLI-TRANSCRIBE-01]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.txt");

    const CliRun run =
        stands.run({stands.video, "--output", out, "--model", stands.model, "--to", "vtt"});

    REQUIRE(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("WEBVTT"));
}

TEST_CASE("--audio-track picks a track, and a track the video lacks is refused with those it has",
          "[e2e][CLI-TRANSCRIBE-02]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun fine =
        stands.run({stands.video, "--output", out, "--model", stands.model, "--audio-track", "1"});
    CHECK(fine.exitCode == 0);

    const std::string refused = stands.scratch.of("out/refused.srt");
    const CliRun run = stands.run(
        {stands.video, "--output", refused, "--model", stands.model, "--audio-track", "5"});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("the video has no audio track 5"));
    CHECK_THAT(run.errors, ContainsSubstring("audio track 0: fra, Original"));
    CHECK_THAT(run.errors, ContainsSubstring("audio track 1: eng, Commentary"));
    CHECK_FALSE(std::filesystem::exists(refused));
}

TEST_CASE("the language is detected unless --language forces it, and the one used is reported",
          "[e2e][CLI-TRANSCRIBE-03]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun detected = stands.run({stands.video, "--output", out, "--model", stands.model});
    REQUIRE(detected.exitCode == 0);
    CHECK_THAT(detected.errors, ContainsSubstring("language: en"));
    CHECK_THAT(contentOf(stands.argumentLog), ContainsSubstring("-l\nauto\n"));

    const CliRun forced =
        stands.run({stands.video, "--output", out, "--model", stands.model, "--language", "fr"});
    REQUIRE(forced.exitCode == 0);
    CHECK_THAT(contentOf(stands.argumentLog), ContainsSubstring("-l\nfr\n"));
}

TEST_CASE("--keep-audio writes the extracted audio to the file it names",
          "[e2e][CLI-TRANSCRIBE-04]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");
    const std::string audio = stands.scratch.of("out/film.wav");

    const CliRun run =
        stands.run({stands.video, "--output", out, "--model", stands.model, "--keep-audio", audio});

    REQUIRE(run.exitCode == 0);
    CHECK(contentOf(audio) == "pcm");
}

TEST_CASE("--raw writes the segments of the engine and nothing is merged",
          "[e2e][CLI-TRANSCRIBE-05]") {
    Stands stands;
    stands.everything();
    stands.whisper(writeFile(stands.scratch, "two.json", kTwoClosePieces));
    const std::string merged = stands.scratch.of("out/merged.srt");
    const std::string raw = stands.scratch.of("out/raw.srt");

    REQUIRE(stands.run({stands.video, "--output", merged, "--model", stands.model}).exitCode == 0);
    REQUIRE(
        stands.run({stands.video, "--output", raw, "--model", stands.model, "--raw"}).exitCode ==
        0);

    CHECK_THAT(contentOf(merged), ContainsSubstring("I was going to tell you something."));
    CHECK_FALSE(contentOf(merged).find("\n2\n") != std::string::npos);
    CHECK_THAT(contentOf(raw), ContainsSubstring("I was going to\n"));
    CHECK_THAT(contentOf(raw),
               ContainsSubstring("\n2\n00:00:01,600 --> 00:00:03,000\ntell you something.\n"));
}

TEST_CASE("a missing engine or model is reported with the guide, nothing written, a non-zero code",
          "[e2e][CLI-TRANSCRIBE-06]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");

    // No model given, nor in the settings.
    const CliRun noModel = stands.run({stands.video, "--output", out});
    CHECK(noModel.exitCode == 2);
    CHECK_THAT(noModel.errors, ContainsSubstring("no model was given"));
    CHECK_THAT(noModel.errors, ContainsSubstring("Installing the transcription tools"));
    CHECK_FALSE(std::filesystem::exists(out));

    // An executable the settings name and that is not there: the engine is missing.
    const std::string settings =
        writeFile(stands.scratch,
                  "settings.conf",
                  "transcription.executable = " + stands.scratch.of("none") + "\n");
    std::filesystem::remove(stands.directory + "/whisper-cli");
    const CliRun noEngine = invokeWithSearchPath(stands.directory,
                                                 {"--config",
                                                  settings,
                                                  "transcribe",
                                                  stands.video,
                                                  "--output",
                                                  out,
                                                  "--model",
                                                  stands.model});
    CHECK(noEngine.exitCode == 2);
    CHECK_THAT(noEngine.errors, ContainsSubstring("the transcription engine was not found"));
    CHECK_THAT(noEngine.errors, ContainsSubstring("Installing the transcription tools"));
    CHECK_FALSE(std::filesystem::exists(out));
}

TEST_CASE("a language the model cannot transcribe is refused before any work starts",
          "[e2e][CLI-TRANSCRIBE-07]") {
    Stands stands;
    stands.everything();
    const std::string english = writeFile(stands.scratch, "english.bin", modelHeader(51864));
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun run =
        stands.run({stands.video, "--output", out, "--model", english, "--language", "fr"});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("the model cannot transcribe the language fr"));
    CHECK_FALSE(std::filesystem::exists(stands.argumentLog));
    CHECK_FALSE(std::filesystem::exists(out));
}

TEST_CASE("--format json writes one record with the language, the counts and the files written",
          "[e2e][CLI-TRANSCRIBE-08]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");
    const std::string audio = stands.scratch.of("out/film.wav");

    const CliRun run = invokeWithSearchPath(stands.directory,
                                            {"--format",
                                             "json",
                                             "transcribe",
                                             stands.video,
                                             "--output",
                                             out,
                                             "--model",
                                             stands.model,
                                             "--keep-audio",
                                             audio});

    REQUIRE(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("\"command\":\"transcribe\""));
    CHECK_THAT(run.output, ContainsSubstring("\"language\":\"en\""));
    CHECK_THAT(run.output, ContainsSubstring("\"subtitles\":2"));
    CHECK_THAT(run.output, ContainsSubstring("\"segments\":1"));
    CHECK_THAT(run.output, ContainsSubstring("\"audio\":\"" + audio + "\""));
    CHECK_THAT(run.output, ContainsSubstring("\"destination\":\"" + out + "\""));
}
