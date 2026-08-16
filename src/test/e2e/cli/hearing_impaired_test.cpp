// Removing the hearing-impaired mentions of a real file, through the real
// binary.
//
// The rule itself is settled by `mentions.cas`, case by case, and the command
// by its unit tests. What is proved here is that the whole of it reaches a file
// on disk: the right subtitles rewritten, the emptied ones gone, and a report
// that says how many of each.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;

TEST_CASE("mentions between brackets and parentheses are removed", "[e2e][CLI-HEARING-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("propre.srt");

    const CliRun run =
        invoke({"hearing-impaired", "--output", out, corpus("valides/mentions.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("Attends Marie."));
    CHECK_THAT(contentOf(out), !ContainsSubstring("il tousse"));
}

TEST_CASE("a subtitle the removal empties leaves the file", "[e2e][CLI-HEARING-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("sans-vides.srt");

    CHECK(invoke({"--quiet", "hearing-impaired", "--output", out, corpus("valides/mentions.srt")})
              .exitCode == 0);

    // Five subtitles in, one of them nothing but a mention: four come out, and
    // they are renumbered from one.
    CHECK_THAT(contentOf(out), !ContainsSubstring("Bruit de pas"));
    CHECK_THAT(contentOf(out), ContainsSubstring("4\n00:00:13,000"));
    CHECK_THAT(contentOf(out), !ContainsSubstring("5\n"));
}

TEST_CASE("a numeric reference is left alone", "[e2e][CLI-HEARING-03]") {
    const Scratch scratch;
    const std::string out = scratch.of("reference.srt");

    CHECK(invoke({"--quiet", "hearing-impaired", "--output", out, corpus("valides/mentions.srt")})
              .exitCode == 0);

    CHECK_THAT(contentOf(out), ContainsSubstring("Voir [1] la note."));
}

TEST_CASE("the report names what changed and what went", "[e2e][CLI-HEARING-04]") {
    const Scratch scratch;
    const std::string out = scratch.of("compté.srt");

    const CliRun run =
        invoke({"hearing-impaired", "--output", out, corpus("valides/mentions.srt")});

    CHECK(run.exitCode == 0);
    // Two rewritten — « Attends [il tousse] Marie » and the dialogue that lost
    // a voice — and one taken out entirely.
    CHECK_THAT(run.errors, ContainsSubstring("2 subtitles cleaned, 1 removed"));
}

TEST_CASE("a file with no mention is written unchanged", "[e2e][CLI-HEARING-05]") {
    const Scratch scratch;
    const std::string out = scratch.of("intact.srt");

    const CliRun run = invoke({"hearing-impaired", "--output", out, corpus("valides/trois.srt")});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(out) == contentOf(corpus("valides/trois.srt")));
    CHECK_THAT(run.errors, ContainsSubstring("no mention to remove"));
}

TEST_CASE("without a destination nothing is written", "[e2e][CLI-HEARING-06]") {
    const CliRun run = invoke({"hearing-impaired", corpus("valides/mentions.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
}
