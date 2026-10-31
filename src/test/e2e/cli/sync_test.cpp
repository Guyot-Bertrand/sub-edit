// `sync`, through the real binary, against stand-ins for `whisper-cli` and `ffmpeg` that the test
// puts at the front of the search path: the fake transcriber writes what an engine would have heard
// where the tool would, taken from the alignment cases of phase 23 — a file at a known fault, and
// the words that were really said.
//
// What is read back is worked out from the case, never from the program.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>
#include <vector>

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

/// The first bytes of a multilingual model.
std::string modelHeader() {
    std::string bytes = "lmgg";
    constexpr unsigned kVocabulary = 51865;
    for (int index = 0; index < 4; ++index)
        bytes += static_cast<char>((kVocabulary >> (8 * index)) & 0xFFU);
    return bytes + std::string(8, '\0');
}

/// A directory holding the stand-ins, a model, a film, and a case to re-time.
struct Stands {
    const Scratch scratch;
    std::string directory = scratch.of("bin");
    std::string model = writeFile(scratch, "model.bin", modelHeader());
    std::string video = writeFile(scratch, "film.mkv", "not really a video");
    std::string subtitles;

    explicit Stands(const std::string& name, const std::string& heard = "heard.json") {
        subtitles =
            writeFile(scratch, "film.srt", contentOf(kData + "/alignment/" + name + "/given.srt"));
        script("whisper-cli",
               "exec '" + kFake + "' --progress=4 --output-from='" + kData + "/alignment/" + name +
                   "/" + heard + "' \"$@\"");
        const std::string audio = writeFile(scratch, "sample.wav", "pcm");
        script("ffmpeg", "exec '" + kFake + "' --output-from='" + audio + "' \"$@\"");
    }

    void script(const std::string& name, const std::string& body) const {
        const std::string path = writeFile(scratch, "bin/" + name, "#!/bin/sh\n" + body + "\n");
        std::filesystem::permissions(
            path, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    }

    [[nodiscard]] CliRun run(std::vector<std::string> arguments) const {
        std::vector<std::string> all{"sync", subtitles};
        all.insert(all.end(), arguments.begin(), arguments.end());
        return invokeWithSearchPath(directory, all);
    }

    [[nodiscard]] std::vector<std::string> listening(const std::string& out) const {
        return {"--video", video, "--model", model, "--output", out};
    }
};

std::vector<std::string> join(std::vector<std::string> first,
                              const std::vector<std::string>& more) {
    first.insert(first.end(), more.begin(), more.end());
    return first;
}

} // namespace

TEST_CASE("sync re-times a file on the audio and changes the starts and ends only",
          "[e2e][CLI-SYNC-01]") {
    const Stands stands{"offset-plus-2400"};
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun run = stands.run(stands.listening(out));

    REQUIRE(run.exitCode == 0);
    const std::string written = contentOf(out);
    // The file said 3.4 s; the voice says 1.0 s. The end is where the last word ends, which the
    // engine puts a few hundredths before the line's own.
    CHECK_THAT(
        written,
        ContainsSubstring("1\n00:00:01,000 --> 00:00:02,960\nThe harbour was quiet at dawn."));
    CHECK_THAT(
        written,
        ContainsSubstring("2\n00:00:04,200 --> 00:00:05,760\nNobody had lit the lamps yet."));
    // Nothing else moved: the same twelve entries, the same texts.
    CHECK_THAT(written, ContainsSubstring("12\n"));
    CHECK_THAT(written, ContainsSubstring("She carried the letter across the square."));
    // The file it was given is left as it was.
    CHECK_THAT(contentOf(stands.subtitles), ContainsSubstring("00:00:03,400 --> 00:00:05,400"));
    // An audio file instead of a film does as well.
    const std::string again = stands.scratch.of("out/again.srt");
    const std::string audio = writeFile(stands.scratch, "voice.wav", "pcm");
    const CliRun byAudio =
        stands.run({"--audio", audio, "--model", stands.model, "--output", again});
    CHECK(byAudio.exitCode == 0);
}

TEST_CASE("a subtitle nobody says keeps its time, is reported, and the command succeeds; a file "
          "none of whose subtitles is found is refused",
          "[e2e][CLI-SYNC-02]") {
    const Stands stands{"extra-line"};
    const std::string out = stands.scratch.of("out/film.srt");
    const std::string report = stands.scratch.of("out/to-check.txt");

    const CliRun run = stands.run(join(stands.listening(out), {"--report", report}));

    REQUIRE(run.exitCode == 0);
    // The credit that is said nowhere is where the file had it.
    CHECK_THAT(contentOf(out),
               ContainsSubstring("00:00:21,400 --> 00:00:23,000\nSubtitles by an unknown hand."));
    CHECK_THAT(run.errors, ContainsSubstring("to check"));
    CHECK_THAT(contentOf(report), ContainsSubstring("6\tnot found in the audio, left as it was"));

    // The wrong audio: the engine heard other words altogether.
    const Stands wrong{"extra-line"};
    wrong.script("whisper-cli",
                 "exec '" + kFake + "' --output-from='" + kData + "/whisper/jfk.full.json' \"$@\"");
    const std::string none = wrong.scratch.of("out/none.srt");
    const CliRun refused = wrong.run(wrong.listening(none));
    CHECK(refused.exitCode == 2);
    CHECK_THAT(refused.errors, ContainsSubstring("was found in the audio"));
    CHECK_FALSE(std::filesystem::exists(none));
}

TEST_CASE("the options --start and --end restrict the audio, and only the subtitles that begin in "
          "the range "
          "are touched; a bad range is refused before any work",
          "[e2e][CLI-SYNC-03]") {
    const Stands stands{"offset-plus-2400"};
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun run = stands.run(join(stands.listening(out), {"--start", "0:20"}));

    REQUIRE(run.exitCode == 0);
    const std::string written = contentOf(out);
    // Before the range: where the file had it. In the range: where it is said.
    CHECK_THAT(written, ContainsSubstring("00:00:03,400 --> 00:00:05,400"));
    CHECK_THAT(written, ContainsSubstring("00:00:42,000 --> 00:00:44,960"));

    const std::string refused = stands.scratch.of("out/refused.srt");
    const CliRun bad =
        stands.run(join(stands.listening(refused), {"--start", "1:00", "--end", "0:30"}));
    CHECK(bad.exitCode == 1);
    CHECK_FALSE(std::filesystem::exists(refused));
}

TEST_CASE("the report counts what was placed and what is to check, and --format json writes it",
          "[e2e][CLI-SYNC-04]") {
    const Stands stands{"unheard-line"};
    const std::string out = stands.scratch.of("out/film.srt");

    const CliRun run = invokeWithSearchPath(
        stands.directory,
        join({"--format", "json", "sync", stands.subtitles}, stands.listening(out)));

    REQUIRE(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("\"command\":\"sync\""));
    CHECK_THAT(run.output, ContainsSubstring("\"subtitles\":12"));
    CHECK_THAT(run.output, ContainsSubstring("\"placed\":11"));
    CHECK_THAT(run.output, ContainsSubstring("\"not_found\":1"));
    CHECK_THAT(run.output, ContainsSubstring("\"median_shift_ms\":"));
    CHECK_THAT(run.output, ContainsSubstring("\"drift_ms\":"));
    CHECK_THAT(run.output,
               ContainsSubstring("\"flagged\":[{\"subtitle\":9,\"kind\":\"not-found\"}"));
    // The line nobody says keeps its time, which runs into the next one, now earlier: cut back.
    CHECK_THAT(run.output, ContainsSubstring("{\"subtitle\":9,\"kind\":\"cut-back\"}]"));
}

TEST_CASE(
    "the options --min-match and --doubt-gap override the settings, and a value out of range is "
    "refused before any work",
    "[e2e][CLI-SYNC-05]") {
    const Stands stands{"partial"};
    const std::string out = stands.scratch.of("out/film.srt");

    // Asking that every word be heard flags nothing here, but the run is the same run.
    const CliRun strict = stands.run(join(stands.listening(out), {"--min-match", "100"}));
    CHECK(strict.exitCode == 0);

    for (const std::vector<std::string>& bad : {std::vector<std::string>{"--min-match", "0"},
                                                std::vector<std::string>{"--min-match", "101"},
                                                std::vector<std::string>{"--doubt-gap", "-5"}}) {
        const std::string refused = stands.scratch.of("out/refused.srt");
        const CliRun run = stands.run(join(stands.listening(refused), bad));
        INFO(bad.front());
        CHECK(run.exitCode == 1);
        CHECK_FALSE(std::filesystem::exists(refused));
    }

    // A setting is read, and an option goes over it.
    const std::string settings =
        writeFile(stands.scratch, "settings.conf", "alignment.doubt-gap = 1\n");
    const std::string flagged = stands.scratch.of("out/flagged.srt");
    const CliRun tight = invokeWithSearchPath(
        stands.directory,
        join({"--config", settings, "sync", stands.subtitles}, stands.listening(flagged)));
    CHECK(tight.exitCode == 0);
    CHECK_THAT(tight.errors, ContainsSubstring("to check"));
    const std::string loose = stands.scratch.of("out/loose.srt");
    const CliRun over =
        invokeWithSearchPath(stands.directory,
                             join({"--config", settings, "sync", stands.subtitles},
                                  join(stands.listening(loose), {"--doubt-gap", "10000"})));
    CHECK(over.exitCode == 0);
    CHECK_THAT(over.errors, ContainsSubstring("0 to check"));
}

TEST_CASE("a missing model, engine or audio source is refused with the pointer to the guide, "
          "and nothing is written",
          "[e2e][CLI-SYNC-06]") {
    const Stands stands{"offset-plus-2400"};
    const std::string out = stands.scratch.of("out/film.srt");

    // No model given, nor in the settings.
    const CliRun noModel = stands.run({"--video", stands.video, "--output", out});
    CHECK(noModel.exitCode == 2);
    CHECK_THAT(noModel.errors, ContainsSubstring("no model was given"));
    CHECK_THAT(noModel.errors, ContainsSubstring("Installing the transcription tools"));
    CHECK_FALSE(std::filesystem::exists(out));

    // Neither a film nor an audio file to listen to, or both.
    const CliRun neither = stands.run({"--model", stands.model, "--output", out});
    CHECK(neither.exitCode == 1);
    CHECK_THAT(neither.errors, ContainsSubstring("give --video or --audio"));
    const CliRun both =
        stands.run({"--video", stands.video, "--audio", stands.video, "--output", out});
    CHECK(both.exitCode == 1);

    // The engine is not there.
    const std::string settings =
        writeFile(stands.scratch,
                  "settings.conf",
                  "transcription.executable = " + stands.scratch.of("none") + "\n");
    std::filesystem::remove(stands.directory + "/whisper-cli");
    const CliRun noEngine = invokeWithSearchPath(
        stands.directory,
        join({"--config", settings, "sync", stands.subtitles}, stands.listening(out)));
    CHECK(noEngine.exitCode == 2);
    CHECK_THAT(noEngine.errors, ContainsSubstring("the transcription engine was not found"));
    CHECK_FALSE(std::filesystem::exists(out));
}
