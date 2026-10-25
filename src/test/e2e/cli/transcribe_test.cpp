// `transcribe`, through the real binary, against stand-ins for `whisper-cli`, `ffmpeg` and
// `ffprobe` that the test puts at the front of the search path: the fake transcriber, which
// writes a sample of whisper.cpp's real output where the tool would.
//
// What is read back is worked out from the sample, never from the program.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
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
               R"(printf '%s\n' "$@" > ')" + argumentLog + "'\nexec '" + kFake + "' " + knobs +
                   " --output-from='" + sample + "' \"$@\"");
    }

    /// `whisper-cli` that prints the banner it starts with and three segments as it goes,
    /// as the real tool does, before it writes its result.
    void whisperPrinting() {
        const std::string banner =
            writeFile(scratch,
                      "banner",
                      "main: processing 'a.wav' (1600000 samples, 100.0 sec), 4 threads\n");
        const std::string printed = writeFile(scratch,
                                              "printed",
                                              "[00:00:00.000 --> 00:00:20.000]   First words.\n"
                                              "[00:00:20.000 --> 00:00:50.000]   Then more.\n"
                                              "[00:00:50.000 --> 00:01:20.000]   And the last.\n");
        script("whisper-cli",
               "exec '" + kFake + "' --no-echo --stderr-from='" + banner + "' --stdout-from='" +
                   printed + "' --output-from='" + kData + "/whisper/jfk.full.json' \"$@\"");
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

    [[nodiscard]] CliRun run(std::vector<std::string> arguments) const {
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

TEST_CASE("the option --to writes another format than the extension says",
          "[e2e][CLI-TRANSCRIBE-01]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.txt");

    const CliRun run =
        stands.run({stands.video, "--output", out, "--model", stands.model, "--to", "vtt"});

    REQUIRE(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("WEBVTT"));
}

TEST_CASE("the option --audio-track picks a track, and a track the video lacks is refused with "
          "those it has",
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

TEST_CASE("the option --keep-audio writes the extracted audio to the file it names",
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

TEST_CASE("the option --raw writes the segments of the engine and nothing is merged",
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

TEST_CASE("with --format json the command writes one record with the language, the counts and the "
          "files written",
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

TEST_CASE("a range is handed to the engine, and a bad one is refused before any work",
          "[e2e][CLI-TRANSCRIBE-09]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun fine = stands.run(
        {stands.video, "--output", out, "--model", stands.model, "--start", "1:00", "--end", "90"});

    REQUIRE(fine.exitCode == 0);
    CHECK_THAT(contentOf(stands.argumentLog), ContainsSubstring("-ot\n60000\n"));
    CHECK_THAT(contentOf(stands.argumentLog), ContainsSubstring("-d\n30000\n"));

    std::filesystem::remove(stands.argumentLog);
    const std::string refused = stands.scratch.of("out/refused.srt");
    const CliRun backwards = stands.run({stands.video,
                                         "--output",
                                         refused,
                                         "--model",
                                         stands.model,
                                         "--start",
                                         "2:00",
                                         "--end",
                                         "1:00"});

    CHECK(backwards.exitCode != 0);
    CHECK_THAT(backwards.errors, ContainsSubstring("the end of the range must come after"));
    CHECK_FALSE(std::filesystem::exists(stands.argumentLog));
    CHECK_FALSE(std::filesystem::exists(refused));

    const CliRun nonsense = stands.run(
        {stands.video, "--output", refused, "--model", stands.model, "--start", "later"});
    CHECK(nonsense.exitCode != 0);
    CHECK_THAT(nonsense.errors, ContainsSubstring("\"later\" is not a time"));
}

TEST_CASE("the command says the step, how far it is, and the text last obtained",
          "[e2e][CLI-TRANSCRIBE-10]") {
    Stands stands;
    stands.whisperPrinting();
    stands.ffprobe();
    stands.ffmpeg();
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun run = invokeWithSearchPath(
        stands.directory,
        {"-vvv", "transcribe", stands.video, "--output", out, "--model", stands.model});

    REQUIRE(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("extracting the audio…"));
    CHECK_THAT(run.errors, ContainsSubstring("transcribing…"));
    CHECK_THAT(run.errors, ContainsSubstring("transcribing: 20%, 0:00:20 of 0:01:40"));
    CHECK_THAT(run.errors, ContainsSubstring("transcribing: 80%, 0:01:20 of 0:01:40"));
    CHECK_THAT(run.errors, ContainsSubstring("last text: Then more."));
    CHECK_THAT(run.errors, ContainsSubstring("transcribing: 100%"));
}

TEST_CASE("the name of the kept audio takes the rate the engine asked for",
          "[e2e][CLI-TRANSCRIBE-04]") {
    Stands stands;
    stands.everything();
    const std::string out = stands.scratch.of("out/film.srt");

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
                                             stands.scratch.of("out/film.{rate}.wav")});

    REQUIRE(run.exitCode == 0);
    const std::string kept = stands.scratch.of("out/film.16000.wav");
    CHECK(contentOf(kept) == "pcm");
    CHECK_THAT(run.output, ContainsSubstring("\"audio\":\"" + kept + "\""));
}

TEST_CASE("--list-languages lists what the engine offers for the model, and runs nothing",
          "[e2e][CLI-TRANSCRIBE-11]") {
    Stands stands;
    stands.everything();
    const std::string english = writeFile(stands.scratch, "english.bin", modelHeader(51864));

    const CliRun every = stands.run({"--list-languages", "--model", stands.model});

    REQUIRE(every.exitCode == 0);
    CHECK_THAT(every.output, ContainsSubstring("fr\tFrench\n"));
    CHECK_THAT(every.output, ContainsSubstring("yue\tCantonese\n"));
    CHECK(std::count(every.output.begin(), every.output.end(), '\n') == 100);
    // Nothing was run: no engine received a thing.
    CHECK_FALSE(std::filesystem::exists(stands.argumentLog));

    const CliRun only = stands.run({"--list-languages", "--model", english});
    REQUIRE(only.exitCode == 0);
    CHECK(only.output == "en\tEnglish\n");

    // Without a model there is nothing to ask the engine about, and the guide is pointed to.
    const CliRun none = stands.run({"--list-languages"});
    CHECK(none.exitCode == 2);
    CHECK_THAT(none.errors, ContainsSubstring("Installing the transcription tools"));
}
