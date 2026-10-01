// `--format json`: one record per input on standard output, and what makes it
// a promise a script can rely on (ADR 0038).
//
// The expectations are written by hand, under `attendus/json/`, and compared
// byte for byte. The paths in them are the scratch directory's and the
// corpus's, which differ between runs and machines: the output is made
// anonymous before the comparison, and the files say `<scratch>` and `<corpus>`.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
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
using subedit::e2e::writeFile;
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

/// What a run wrote on standard output, with the two directories that vary
/// replaced by what the expected files say.
std::string outputOf(const CliRun& run, const Scratch& scratch) {
    return anonymised(anonymised(run.output, scratch.path(), "<scratch>"),
                      corpus("").substr(0, corpus("").find_last_not_of('/') + 1),
                      "<corpus>");
}

std::string expected(const std::string& name) {
    return corpus("attendus/json/" + name);
}

std::size_t linesOf(const std::string& text) {
    std::size_t lines = 0;
    for (const char c : text)
        if (c == '\n')
            ++lines;
    return lines;
}

} // namespace

TEST_CASE("inspect describes each input in a record, failures included",
          "[e2e][CLI-JSON-01][CLI-JSON-02][CLI-JSON-03][CLI-JSON-08][CLI-JSON-10]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");

    const CliRun run =
        invoke({"--format", "json", "inspect", good, scratch.of("in/absent.srt"), broken});

    CHECK(run.exitCode == 3);
    CHECK_THAT(outputOf(run, scratch), MatchesFile(expected("inspect-batch.jsonl")));
}

TEST_CASE("the grid, or the rate of a file counted in frames, is described without a decimal point",
          "[e2e][CLI-JSON-06][CLI-JSON-08][CLI-JSON-10]") {
    const Scratch scratch;
    const std::string frames = writeFile(scratch, "in/frames.sub", "{25}{50}Un.\n{75}{100}Deux.\n");

    const CliRun grid =
        invoke({"--format", "json", "inspect", corpus("grilles/grille-24-courte.srt")});
    const CliRun counted = invoke({"--format", "json", "inspect", "--frame-rate", "25", frames});

    CHECK(outputOf(grid, scratch) + outputOf(counted, scratch) ==
          contentOf(expected("inspect-grid.jsonl")));
}

TEST_CASE("a batch that writes gives one record per input, in the order of the inputs",
          "[e2e][CLI-JSON-02][CLI-JSON-09][CLI-JSON-10]") {
    const Scratch scratch;
    const std::string first = writeSrt(scratch, "in/a.srt", 2);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");
    const std::string last = writeSrt(scratch, "in/c.srt", 2);

    const CliRun run = invoke({"--format",
                               "json",
                               "shift",
                               "--by",
                               "1",
                               "--output-dir",
                               scratch.of("out"),
                               first,
                               broken,
                               last});

    CHECK(run.exitCode == 3);
    // Never zero and never two for an input, whatever became of it.
    CHECK(linesOf(run.output) == 3);
    CHECK_THAT(outputOf(run, scratch), MatchesFile(expected("shift-batch.jsonl")));
}

TEST_CASE("every subcommand that writes says its destination and its counts",
          "[e2e][CLI-JSON-09][CLI-JSON-10]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);
    const std::string out = scratch.of("out");

    std::string all;
    for (const std::vector<std::string>& arguments : std::vector<std::vector<std::string>>{
             {"transform", "--first", "1=1.000", "--last", "2=2.000"},
             {"framerate", "--from", "25", "--to", "24"},
             {"snap", "--rate", "10"},
             {"hearing-impaired"},
             {"convert", "--to", "vtt"}}) {
        std::vector<std::string> command{"--format", "json"};
        command.insert(command.end(), arguments.begin(), arguments.end());
        command.insert(command.end(), {"--output-dir", out, input});
        const CliRun run = invoke(command);
        REQUIRE(run.exitCode == 0);
        all += outputOf(run, scratch);
    }
    CHECK(all == contentOf(expected("writers.jsonl")));
}

TEST_CASE("what the reading had to decide is in the record, at every level of narration",
          "[e2e][CLI-JSON-05][CLI-JSON-10]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch,
                                        "in/unnumbered.srt",
                                        "00:00:01,000 --> 00:00:01,500\nUn\n\n"
                                        "00:00:02,000 --> 00:00:02,500\nDeux\n\n");

    for (const char* level : {"-q", "-vv", "-vvv"}) {
        const CliRun run = invoke({level,
                                   "--format",
                                   "json",
                                   "shift",
                                   "--by",
                                   "1",
                                   "--output-dir",
                                   scratch.of("out"),
                                   input});
        CHECK(run.exitCode == 0);
        CHECK_THAT(outputOf(run, scratch), MatchesFile(expected("warnings.jsonl")));
    }
}

TEST_CASE("the verbosity does not change standard output, and the narration is the text's own",
          "[e2e][CLI-JSON-04]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");

    for (const char* level : {"-q", "-v", "-vv", "-vvv"}) {
        const CliRun json = invoke({level, "--format", "json", "inspect", good, broken});
        const CliRun text = invoke({level, "inspect", good, broken});

        // Same on the way out, whatever was asked of the narration...
        CHECK(json.output == invoke({"--format", "json", "inspect", good, broken}).output);
        // ... and the account on standard error is the text's, word for word.
        CHECK(json.errors == text.errors);
        CHECK(json.exitCode == text.exitCode);
    }
}

TEST_CASE("text stays the default, and says the same with --format text", "[e2e][CLI-JSON-01]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);

    const CliRun plain = invoke({"inspect", good});
    const CliRun named = invoke({"--format", "text", "inspect", good});

    CHECK(plain.output == named.output);
    CHECK_THAT(plain.output, ContainsSubstring("  format: SubRip"));
    CHECK(plain.output.find('{') == std::string::npos);
}

TEST_CASE("a usage error writes nothing on standard output, in json as in text",
          "[e2e][CLI-JSON-07]") {
    const Scratch scratch;
    const std::string good = writeSrt(scratch, "in/a.srt", 2);

    for (const std::vector<std::string>& arguments : std::vector<std::vector<std::string>>{
             {"--format", "json", "shift", "--by", "1", good},     // no destination
             {"--format", "xml", "inspect", good},                 // unknown format
             {"--format", "json", "-q", "-v", "inspect", good},    // opposite intentions
             {"--format", "json", "shift", "--in-place", good}}) { // no amount
        const CliRun run = invoke(arguments);
        CHECK(run.exitCode == 1);
        CHECK(run.output.empty());
    }
}

TEST_CASE("a path that is not UTF-8 is written with U+FFFD, and the record says so",
          "[e2e][CLI-JSON-03]") {
    const Scratch scratch;
    // Linux allows a name that is bytes and not text.
    const std::string name = scratch.of("in/caf\xE9.srt");
    std::filesystem::create_directories(scratch.of("in"));
    {
        const std::string content = contentOf(writeSrt(scratch, "in/plain.srt", 1));
        std::ofstream file{name, std::ios::binary};
        file << content;
    }

    const CliRun run = invoke({"--format", "json", "inspect", name});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("caf\xEF\xBF\xBD.srt"));
    CHECK_THAT(run.output, ContainsSubstring("\"warnings\":[{\"kind\":\"path-not-utf8\"}]"));
    // And the record is one line of valid UTF-8.
    CHECK(linesOf(run.output) == 1);
}

TEST_CASE("two runs of the same arguments write the same bytes", "[e2e][CLI-JSON-10]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/a.srt", 2);

    const CliRun first = invoke({"--format", "json", "inspect", input});
    const CliRun second = invoke({"--format", "json", "inspect", input});

    CHECK(first.output == second.output);
}
