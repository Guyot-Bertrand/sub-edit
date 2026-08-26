// Aligning a file on a frame rate, from the command line.
//
// `snap` and `framerate` take the same arguments and do opposite things, and
// the mistake is silent. That is why the report says how far it moved things,
// and why the manual of each sends the reader to the other.
//
// Every case writes into a `Scratch`: a subcommand given a bare name writes it
// where the test happens to run, which for CTest is the root of the checkout.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;

TEST_CASE("the help lists the alignment", "[e2e][CLI-USAGE-01]") {
    const std::string help = invoke({}).output;

    CHECK_THAT(help, ContainsSubstring("\n  snap"));
}

TEST_CASE("snap lays every position on the nearest frame", "[e2e][CLI-SNAP-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("aligne.srt");

    const CliRun run =
        invoke({"snap", "--rate", "25", "--output", out, corpus("grilles/grille-24.srt")});

    CHECK(run.exitCode == 0);
    // Written on a grid at 24 and asked for 25: the report now says the file is
    // on the rate it was given.
    CHECK_THAT(invoke({"inspect", out}).output,
               ContainsSubstring("  frame rate grid: 25 fps, clean ("));
}

TEST_CASE("snap says how many positions moved, and by how much", "[e2e][CLI-SNAP-02]") {
    const Scratch scratch;

    const CliRun run = invoke({"snap",
                               "--rate",
                               "25",
                               "--output",
                               scratch.of("aligne.srt"),
                               corpus("grilles/grille-24.srt")});

    // Half a frame at 25 frames per second, and not one millisecond more. It is
    // the line that tells a user which of the two operations they just ran.
    CHECK_THAT(run.errors, ContainsSubstring(" aligned on 25 fps, "));
    CHECK_THAT(run.errors, ContainsSubstring(" positions moved, by at most 20 ms"));
}

TEST_CASE("aligning is not converting, and the two say so", "[e2e][CLI-SNAP-02]") {
    const Scratch scratch;

    const CliRun aligned = invoke({"snap",
                                   "--rate",
                                   "25",
                                   "--output",
                                   scratch.of("aligne.srt"),
                                   corpus("grilles/grille-24.srt")});
    const CliRun converted = invoke({"framerate",
                                     "--from",
                                     "24",
                                     "--to",
                                     "25",
                                     "--output",
                                     scratch.of("converti.srt"),
                                     corpus("grilles/grille-24.srt")});

    CHECK_THAT(aligned.errors, ContainsSubstring("by at most 20 ms"));
    CHECK_THAT(converted.errors, ContainsSubstring("retimed from 24 to 25 fps"));
}

TEST_CASE("shift measures its own amount from the grid", "[e2e][CLI-SHIFT-03]") {
    const Scratch scratch;

    const CliRun run = invoke({"shift",
                               "--to-grid",
                               "--output",
                               scratch.of("recale.srt"),
                               corpus("grilles/grille-24-decalee.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(" shifted by 0.001 s onto their 24 fps grid"));
}

TEST_CASE("a file on no grid cannot be shifted onto one", "[e2e][CLI-SHIFT-04]") {
    const Scratch scratch;

    const CliRun run = invoke({"shift",
                               "--to-grid",
                               "--output",
                               scratch.of("rien.srt"),
                               corpus("grilles/grille-absurde.srt")});

    CHECK(run.exitCode != 0);
    CHECK_THAT(run.errors, ContainsSubstring("no frame rate grid was found"));
}

TEST_CASE("shift refuses to be told the amount twice", "[e2e][CLI-SHIFT-03]") {
    const Scratch scratch;

    const CliRun run = invoke({"shift",
                               "--by",
                               "1.000",
                               "--to-grid",
                               "--output",
                               scratch.of("rien.srt"),
                               corpus("grilles/grille-24-decalee.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("give one or the other"));
}

TEST_CASE("shift refuses to be told the amount not at all", "[e2e][CLI-SHIFT-03]") {
    const Scratch scratch;

    const CliRun run = invoke(
        {"shift", "--output", scratch.of("rien.srt"), corpus("grilles/grille-24-decalee.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("--by, or --to-grid"));
}
