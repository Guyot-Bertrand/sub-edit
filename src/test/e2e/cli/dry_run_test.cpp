// `--dry-run`: read, compute, say, and write nothing (ADR 0040).
//
// The expectations are written by hand, under `attendus/dry-run/` and
// `attendus/json/`, and compared byte for byte. The directories that vary
// between runs and machines are made anonymous before the comparison, as in
// `json_test.cpp`.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::MatchesFile;
using subedit::e2e::Scratch;
using subedit::e2e::writeSrt;
using subedit::e2e::writeUnreadable;

namespace {

/// `text` with every occurrence of `what` written as `placeholder`.
std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

std::string anonymised(const std::string& text, const Scratch& scratch) {
    return anonymised(anonymised(text, scratch.path(), "<scratch>"),
                      corpus("").substr(0, corpus("").find_last_not_of('/') + 1),
                      "<corpus>");
}

/// What each subcommand that writes is asked, minus the file and the destination.
const std::vector<std::vector<std::string>> kWriters{
    {"transform", "--first", "1=1.000", "--last", "2=2.000"},
    {"framerate", "--from", "25", "--to", "24"},
    {"snap", "--rate", "10"},
    {"hearing-impaired"},
    {"convert", "--to", "vtt"},
    {"shift", "--by", "1"}};

std::vector<std::string> with(std::vector<std::string> command,
                              const std::vector<std::string>& more) {
    command.insert(command.end(), more.begin(), more.end());
    return command;
}

/// The lines of `errors` that say a file failed, or sum a batch up.
std::string failuresOf(const std::string& errors) {
    std::string kept;
    std::istringstream lines{errors};
    for (std::string line; std::getline(lines, line);) {
        if (line.find("shifted by") == std::string::npos) {
            kept += line + '\n';
        }
    }
    return kept;
}

} // namespace

TEST_CASE("a dry run writes no file and creates no directory", "[e2e][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);

    for (const std::vector<std::string>& writer : kWriters) {
        const CliRun run =
            invoke(with(writer, {"--dry-run", "--output-dir", scratch.of("out/deep"), input}));

        CHECK(run.exitCode == 0);
        // Neither the directory the destination lies in, nor the file.
        CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
    }

    // And by --output, whose parent a real run would create.
    CHECK(invoke({"shift", "--by", "1", "--dry-run", "--output", scratch.of("new/a.srt"), input})
              .exitCode == 0);
    CHECK_FALSE(std::filesystem::exists(scratch.of("new")));
}

TEST_CASE("a dry run over --in-place leaves the input as it was", "[e2e][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);
    const std::string before = contentOf(input);

    CHECK(invoke({"shift", "--by", "5", "--dry-run", "--in-place", input}).exitCode == 0);

    CHECK(contentOf(input) == before);
}

TEST_CASE("a dry run ends with the code of the real run", "[e2e][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");
    const std::string absent = scratch.of("in/absent.srt");

    for (const std::vector<std::string>& inputs :
         std::vector<std::vector<std::string>>{{good}, {broken}, {good, broken}, {absent, good}}) {
        const CliRun dry = invoke(
            with({"shift", "--by", "1", "--dry-run", "--output-dir", scratch.of("a")}, inputs));
        const CliRun real =
            invoke(with({"shift", "--by", "1", "--output-dir", scratch.of("b")}, inputs));

        CHECK(dry.exitCode == real.exitCode);
        // The failures are the same words — a dry run reads, and what it reads
        // is not different. What differs is the line of a file that came out.
        CHECK(failuresOf(dry.errors) ==
              failuresOf(anonymised(real.errors, scratch.of("b"), scratch.of("a"))));
    }
}

TEST_CASE("a dry run asks for no destination", "[e2e][CLI-DRYRUN-02]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);

    // Without the option, no destination is the usage error it always was.
    CHECK(invoke({"shift", "--by", "1", input}).exitCode == 1);

    const CliRun run = invoke({"shift", "--by", "1", "--dry-run", input});
    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("(dry run, nothing written)"));
}

TEST_CASE("a destination given with --dry-run is judged as it is without it",
          "[e2e][CLI-DRYRUN-03]") {
    const Scratch scratch;
    const std::string first = writeSrt(scratch, "a/film.srt", 2);
    const std::string second = writeSrt(scratch, "b/film.srt", 2);

    // Each of these is refused without the option, and is with it.
    const std::vector<std::vector<std::string>> refused{
        // Two inputs for one destination.
        {"--output-dir", scratch.of("out"), first, second},
        // A destination that is an input.
        {"--output-dir", scratch.of("a"), first},
        // --output names one file, several given.
        {"--output", scratch.of("one.srt"), first, second},
        // Two ways to say where.
        {"--output", scratch.of("one.srt"), "--in-place", first}};

    for (const std::vector<std::string>& arguments : refused) {
        const CliRun real = invoke(with({"shift", "--by", "1"}, arguments));
        const CliRun dry = invoke(with({"shift", "--by", "1", "--dry-run"}, arguments));

        REQUIRE(real.exitCode == 1);
        CHECK(dry.exitCode == 1);
        CHECK(dry.errors == real.errors);
        CHECK(dry.output.empty());
    }
    CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
}

TEST_CASE("a dry run says on standard error that nothing was written", "[e2e][CLI-DRYRUN-06]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);

    std::string narration;
    for (const std::vector<std::string>& writer : kWriters) {
        const CliRun run = invoke(with(writer, {"--dry-run", input}));
        REQUIRE(run.exitCode == 0);
        // Standard output is the changes of a subcommand of text, and this file
        // has none to propose.
        CHECK(run.output.empty());
        narration += anonymised(run.errors, scratch);
    }

    CHECK_THAT(narration, MatchesFile(corpus("attendus/dry-run/narration.txt")));
}

TEST_CASE("a dry run in json says it and has no destination", "[e2e][CLI-JSON-09][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);

    std::string all;
    for (const std::vector<std::string>& writer : kWriters) {
        const CliRun run = invoke(with({"--format", "json"}, with(writer, {"--dry-run", input})));
        REQUIRE(run.exitCode == 0);
        all += anonymised(run.output, scratch);
    }

    CHECK_THAT(all, MatchesFile(corpus("attendus/json/dry-run-writers.jsonl")));
}

TEST_CASE("a dry run in json still gives one record per input, failures included",
          "[e2e][CLI-JSON-02][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");

    const CliRun run =
        invoke({"--format", "json", "shift", "--by", "1", "--dry-run", good, broken, good});

    CHECK(run.exitCode == 3);
    std::size_t lines = 0;
    for (const char c : run.output) {
        lines += c == '\n' ? 1 : 0;
    }
    CHECK(lines == 3);
    CHECK_THAT(run.output, ContainsSubstring("\"ok\":false"));
}

TEST_CASE("a dry run of a subcommand of text prints each change: number, before, after",
          "[e2e][CLI-DRYRUN-04]") {
    const Scratch scratch;

    const CliRun run = invoke({"hearing-impaired", "--dry-run", corpus("valides/mentions.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch),
               MatchesFile(corpus("attendus/dry-run/hearing-impaired.txt")));
    // The account is not the result: it stays on standard error.
    CHECK_THAT(run.errors, ContainsSubstring("3 subtitles cleaned, 1 removed (dry run"));
}

TEST_CASE("the same changes are the json `changes`, after being null for a removal",
          "[e2e][CLI-DRYRUN-05]") {
    const Scratch scratch;

    const CliRun run = invoke(
        {"--format", "json", "hearing-impaired", "--dry-run", corpus("valides/mentions.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch),
               MatchesFile(corpus("attendus/json/dry-run-hearing-impaired.jsonl")));
}

TEST_CASE("what a dry run showed is what the next run writes", "[e2e][CLI-DRYRUN-04]") {
    const Scratch scratch;
    const std::string out = scratch.of("clean.srt");

    const CliRun dry = invoke({"hearing-impaired", "--dry-run", corpus("valides/mentions.srt")});
    REQUIRE(invoke({"--quiet", "hearing-impaired", "--output", out, corpus("valides/mentions.srt")})
                .exitCode == 0);

    // Each proposed text is in the file written, and the removed one is not.
    CHECK_THAT(dry.output, ContainsSubstring("+ Attends Marie."));
    CHECK_THAT(contentOf(out), ContainsSubstring("Attends Marie."));
    CHECK_THAT(dry.output, ContainsSubstring("(removed)"));
    CHECK_THAT(contentOf(out), !ContainsSubstring("Bruit de pas"));
}
