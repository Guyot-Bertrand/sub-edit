// The nine formats and the one frame rate, through the real binary.
//
// **The surface of phase 9, and nothing below it.** Every reader, every writer,
// the markup pivot and the loss report were built and unit-tested one issue at
// a time; what no unit test can say is that a user typing `--to lrc` reaches
// them. That is what this file asserts, and it is why its cases carry the
// requirement tags of the phase rather than a tag of their own.
//
// The corpus is `formats/`: one scene, nine renderings, the same four subtitles
// in each. Comparing a reading to a reading is what makes a round trip worth
// asserting — see its LISEZMOI.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <string_view>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;

namespace {

/// One rendering of the scene: the file, the `--to` value, and the name the
/// report gives its format.
///
/// **Written out rather than derived**, as the renderings of the markup tests
/// are: deriving these three from each other would compare the tool to itself,
/// and the point of the table is that a human can check it against the manual.
struct Rendering {
    std::string_view file;
    std::string_view option;
    std::string_view reported;
};

const std::vector<Rendering> kNine = {
    {.file = "formats/scene.srt", .option = "srt", .reported = "SubRip"},
    {.file = "formats/scene.vtt", .option = "vtt", .reported = "WebVTT"},
    {.file = "formats/scene.subviewer2.sub", .option = "subviewer2", .reported = "SubViewer 2"},
    {.file = "formats/scene.ssa", .option = "ssa", .reported = "Sub Station Alpha"},
    {.file = "formats/scene.ass", .option = "ass", .reported = "Advanced SSA"},
    {.file = "formats/scene.microdvd.sub", .option = "microdvd", .reported = "MicroDVD"},
    {.file = "formats/scene.mpl2.txt", .option = "mpl2", .reported = "MPL2"},
    {.file = "formats/scene.tmplayer.txt", .option = "tmplayer", .reported = "TMPlayer"},
    {.file = "formats/scene.lrc", .option = "lrc", .reported = "LRC"},
};

} // namespace

TEST_CASE("each of the nine opens and comes back byte for byte", "[e2e][CLI-FORMAT-01]") {
    // **The strongest thing a format pair can promise**, and the one the phase
    // was built around: a file asked for nothing gives back the same bytes, its
    // header and its own oddities among them.
    for (const Rendering& rendering : kNine) {
        INFO("format : " << rendering.reported);
        const Scratch scratch;
        const std::string out = scratch.of("again");

        const CliRun run = invoke({"--quiet",
                                   "convert",
                                   "--to",
                                   std::string{rendering.option},
                                   "--output",
                                   out,
                                   corpus(std::string{rendering.file})});

        CHECK(run.exitCode == 0);
        CHECK(contentOf(out) == contentOf(corpus(std::string{rendering.file})));
    }
}

TEST_CASE("the format option accepts each of the nine, and nothing else", "[e2e][CLI-CONVERT-04]") {
    // The other half of the closed set: the nine are offered, and a tenth name
    // is refused by the option itself rather than by a reader that would have
    // to be written to say no.
    //
    // **The rate is given to all nine**, so that this case is about `--to` and
    // nothing else. Eight of them ignore it; MicroDVD without it would be
    // refused for want of a grid, which is a promise of its own further down.
    for (const Rendering& rendering : kNine) {
        INFO("valeur : " << rendering.option);
        const Scratch scratch;
        const CliRun run = invoke({"--quiet",
                                   "convert",
                                   "--to",
                                   std::string{rendering.option},
                                   "--frame-rate",
                                   "25",
                                   "--output",
                                   scratch.of("out"),
                                   corpus("formats/scene.srt")});

        CHECK(run.exitCode == 0);
        CHECK_FALSE(contentOf(scratch.of("out")).empty());
    }

    const Scratch scratch;
    const CliRun refused = invoke(
        {"convert", "--to", "sami", "--output", scratch.of("out"), corpus("formats/scene.srt")});

    CHECK(refused.exitCode == 1);
    CHECK_THAT(refused.errors, ContainsSubstring("--to"));
}

TEST_CASE("inspect names the format it read, among the nine", "[e2e][CLI-FORMAT-03]") {
    for (const Rendering& rendering : kNine) {
        INFO("fichier : " << rendering.file);
        const CliRun run = invoke({"--quiet", "inspect", corpus(std::string{rendering.file})});

        CHECK(run.exitCode == 0);
        CHECK_THAT(run.output,
                   ContainsSubstring("  format: " + std::string{rendering.reported} + "\n"));
    }
}

TEST_CASE("a file no format claims is refused, without a supposition", "[e2e][CLI-FORMAT-02]") {
    // **Refused rather than read as the least unlikely of the nine.** An empty
    // file fits none of them, and answering « SubRip, with no subtitle » would
    // be an invention dressed as a reading.
    const CliRun run = invoke({"inspect", corpus("malformes/vide.srt")});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("is in no format this tool knows"));
    CHECK(run.output.empty());
}

TEST_CASE("a format that carries no end opens, and the reading says so", "[e2e][CLI-FORMAT-04]") {
    // **The diagnostic is the whole point** — ADR 0029. TMPlayer and LRC state
    // no end, so every end in the table was worked out; a user seeing a filled
    // column has to be told that none of it came from the file.
    for (const std::string_view file : {"formats/scene.tmplayer.txt", "formats/scene.lrc"}) {
        INFO("fichier : " << file);
        const CliRun run = invoke({"-vvv", "inspect", corpus(std::string{file})});

        CHECK(run.exitCode == 0);
        CHECK_THAT(run.errors,
                   ContainsSubstring("carries no end times; each one was taken from the "
                                     "next start"));
    }
}

TEST_CASE("a file counted in frames opens at a rate the reading names", "[e2e][CLI-FRAMES-01]") {
    // A MicroDVD file states no rate and no MicroDVD file does — ADR 0030. The
    // one the tool picked is announced, because every position on screen rests
    // on it and the file cannot confirm it.
    const CliRun reading = invoke({"-vvv", "inspect", corpus("formats/scene.microdvd.sub")});

    CHECK(reading.exitCode == 0);
    CHECK_THAT(reading.errors, ContainsSubstring("counts in frames and states no rate"));
    CHECK_THAT(reading.output, ContainsSubstring("  frame rate: 24000/1001 fps, assumed\n"));
}

TEST_CASE("the frame rate option imposes the rate a file is read at", "[e2e][CLI-FRAMES-02]") {
    // **The scene was mastered at twenty-five**, so read there its frames land
    // back on the very positions the eight other renderings carry. Read at the
    // assumed rate they do not, and the gap is the whole reason the option
    // exists.
    const CliRun asked =
        invoke({"--quiet", "inspect", "--frame-rate", "25", corpus("formats/scene.microdvd.sub")});

    CHECK(asked.exitCode == 0);
    CHECK_THAT(asked.output, ContainsSubstring("  frame rate: 25 fps, as asked for\n"));
    CHECK_THAT(asked.output, ContainsSubstring("  span: 00:00:01.000 -> 00:00:15.000\n"));

    const Scratch scratch;
    const CliRun converted = invoke({"--quiet",
                                     "convert",
                                     "--to",
                                     "srt",
                                     "--frame-rate",
                                     "25",
                                     "--output",
                                     scratch.of("a.srt"),
                                     corpus("formats/scene.microdvd.sub")});

    CHECK(converted.exitCode == 0);
    CHECK_THAT(contentOf(scratch.of("a.srt")), ContainsSubstring("00:00:01,000 --> 00:00:03,000"));
}

TEST_CASE("the frame rate option imposes the rate a file is written at", "[e2e][CLI-FRAMES-02]") {
    // The same option, the other direction: this file falls on no grid at all,
    // so without a word it would be refused. Given one, it is written.
    const Scratch scratch;
    const CliRun run = invoke({"--quiet",
                               "convert",
                               "--to",
                               "microdvd",
                               "--frame-rate",
                               "25",
                               "--output",
                               scratch.of("a.sub"),
                               corpus("grilles/grille-absurde.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(scratch.of("a.sub")), ContainsSubstring("{25}{99}"));
}

TEST_CASE("without a rate, writing frames takes the grid and says which", "[e2e][CLI-CONVERT-05]") {
    // **The one place the deduction of phase 16 decides rather than informs.**
    // The rate a time-based file was timed at *is* its grid, and taking it is
    // the only answer that moves nothing.
    const Scratch scratch;
    const CliRun run = invoke({"-vv",
                               "convert",
                               "--to",
                               "microdvd",
                               "--output",
                               scratch.of("a.sub"),
                               corpus("grilles/grille-25.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors,
               ContainsSubstring("counted in frames at 25, the grid the positions fall on"));
}

TEST_CASE("without a rate and without a grid, writing frames is refused", "[e2e][CLI-CONVERT-06]") {
    // The only move left would be to invent a number that displaces every
    // subtitle in the file. Refusing names the option that settles it.
    const Scratch scratch;
    const CliRun run = invoke({"convert",
                               "--to",
                               "microdvd",
                               "--output",
                               scratch.of("a.sub"),
                               corpus("grilles/grille-absurde.srt")});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("--frame-rate"));
    CHECK(contentOf(scratch.of("a.sub")).empty());
}

TEST_CASE("a conversion that loses something says so, post by post", "[e2e][CLI-CONVERT-07]") {
    // **The degradation policy of the phase, seen from where a user stands**:
    // never refuse, never keep quiet about a loss, keep quiet when there is
    // none. LRC carries no end, joins the lines of a subtitle, and has no
    // vocabulary for the italic the scene wears.
    const Scratch scratch;
    const CliRun lossy = invoke(
        {"convert", "--to", "lrc", "--output", scratch.of("a.lrc"), corpus("formats/scene.srt")});

    CHECK(lossy.exitCode == 0);
    CHECK_THAT(lossy.errors, ContainsSubstring("ends are not carried by LRC"));
    CHECK_THAT(lossy.errors, ContainsSubstring("line breaks were joined in 2 subtitles"));
    CHECK_THAT(lossy.errors, ContainsSubstring("1 tag dropped"));

    // And the other half of the promise, which is the harder one to keep.
    const CliRun clean = invoke(
        {"convert", "--to", "vtt", "--output", scratch.of("a.vtt"), corpus("formats/scene.srt")});

    CHECK(clean.exitCode == 0);
    CHECK_FALSE(clean.errors.contains("dropped"));
    CHECK_FALSE(clean.errors.contains("not carried"));
}
