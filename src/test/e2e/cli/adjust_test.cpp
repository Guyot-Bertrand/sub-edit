// `adjust`: bringing durations within a reading speed and limits, through the
// real binary.
//
// Every expectation is worked out by hand from the rule — Gaupol's order:
// reading speed, minimum, maximum, then the gap to the next subtitle, which
// wins — and never read back from the program. The input is one file of four
// subtitles built so that each constraint, and each pair of them, is the one
// that decides somewhere:
//
//   1  0.000 -> 0.400   30 characters, needs 2.0 s at 15 characters a second
//   2  10.000 -> 14.000 15 characters, needs 1.0 s, and sits 0.2 s before the next
//   3  14.200 -> 14.300  3 characters, needs 0.2 s, and the next starts 0.7 s on
//   4  15.000 -> 15.100  6 characters, needs 0.4 s, and has no next one

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <filesystem>
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

namespace {

const std::string kInput = "1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n"
                           "2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n"
                           "3\n00:00:14,200 --> 00:00:14,300\nabc\n\n"
                           "4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n";

struct Case {
    std::string name;
    std::vector<std::string> arguments;
    /// The four ends the file must come out with.
    std::vector<std::string> ends;
};

/// Worked out by hand — see the file's header. The order is the order of
/// `adjust.jsonl`, whose lines say the counts and the constraints of each.
const std::vector<Case> kCases{
    {.name = "A",
     .arguments = {},
     .ends = {"00:00:02,000", "00:00:14,000", "00:00:15,000", "00:00:16,500"}},
    {.name = "B1",
     .arguments = {"--speed", "off", "--minimum", "off"},
     .ends = {"00:00:00,400", "00:00:14,000", "00:00:14,300", "00:00:15,100"}},
    {.name = "B2",
     .arguments = {"--speed", "off"},
     .ends = {"00:00:01,500", "00:00:14,000", "00:00:15,000", "00:00:16,500"}},
    {.name = "C",
     .arguments = {"--minimum", "off"},
     .ends = {"00:00:02,000", "00:00:14,000", "00:00:14,400", "00:00:15,400"}},
    {.name = "D",
     .arguments = {"--speed", "30", "--minimum", "off"},
     .ends = {"00:00:01,000", "00:00:14,000", "00:00:14,300", "00:00:15,200"}},
    {.name = "E",
     .arguments = {"--shorten"},
     .ends = {"00:00:02,000", "00:00:11,500", "00:00:15,000", "00:00:16,500"}},
    {.name = "F",
     .arguments = {"--shorten", "--no-lengthen"},
     .ends = {"00:00:01,500", "00:00:11,500", "00:00:15,000", "00:00:16,500"}},
    {.name = "G",
     .arguments = {"--minimum", "3"},
     .ends = {"00:00:03,000", "00:00:14,000", "00:00:15,000", "00:00:18,000"}},
    {.name = "H",
     .arguments = {"--maximum", "1.8"},
     .ends = {"00:00:01,800", "00:00:11,800", "00:00:15,000", "00:00:16,500"}},
    {.name = "I",
     .arguments = {"--gap", "0.5"},
     .ends = {"00:00:02,000", "00:00:13,700", "00:00:14,500", "00:00:16,500"}},
    {.name = "J",
     .arguments = {"--gap", "off"},
     .ends = {"00:00:02,000", "00:00:14,000", "00:00:15,700", "00:00:16,500"}},
    {.name = "K",
     .arguments = {"--speed", "2", "--minimum", "off"},
     .ends = {"00:00:10,000", "00:00:14,200", "00:00:15,000", "00:00:18,000"}},
    {.name = "L",
     .arguments = {"--range", "2-3", "--shorten"},
     .ends = {"00:00:00,400", "00:00:11,500", "00:00:15,000", "00:00:15,100"}},
};

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

/// The end of each subtitle of a SubRip file.
std::vector<std::string> endsOf(const std::string& srt) {
    std::vector<std::string> ends;
    for (std::size_t at = srt.find(" --> "); at != std::string::npos;
         at = srt.find(" --> ", at + 1)) {
        ends.push_back(srt.substr(at + 5, 12));
    }
    return ends;
}

std::vector<std::string> command(std::vector<std::string> arguments, const Scratch& scratch) {
    std::vector<std::string> line{"--format", "json", "adjust"};
    line.insert(line.end(), arguments.begin(), arguments.end());
    line.insert(line.end(), {"--output-dir", scratch.of("out"), scratch.of("in/d.srt")});
    return line;
}

} // namespace

TEST_CASE("each constraint is set, switched off, or switched on, and Gaupol's order holds",
          "[e2e][CLI-ADJUST-01][CLI-ADJUST-02][CLI-ADJUST-04][CLI-ADJUST-05][CLI-RANGE-01]") {
    const Scratch scratch;
    writeFile(scratch, "in/d.srt", kInput);

    std::string records;
    for (const Case& one : kCases) {
        INFO(one.name);
        const CliRun run = invoke(command(one.arguments, scratch));

        REQUIRE(run.exitCode == 0);
        CHECK(endsOf(contentOf(scratch.of("out/d.srt"))) == one.ends);
        records += anonymised(run.output, scratch.path(), "<scratch>");
    }

    // The counts, and the constraints that were employed, case by case.
    CHECK_THAT(records, MatchesFile(corpus("attendus/json/adjust.jsonl")));
}

TEST_CASE("the gap wins over the minimum, and what gave way is said",
          "[e2e][CLI-ADJUST-01][CLI-ADJUST-04]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    const CliRun run = invoke({"adjust", "--output-dir", scratch.of("out"), input});

    CHECK(run.exitCode == 0);
    // Subtitle 3 is held to 15.000 by the gap — the minimum would have made it
    // 15.700, over the start of the next — and the report names the minimum.
    CHECK(run.errors ==
          input +
              ": adjusted the durations of 3 subtitles; could not satisfy the minimum "
              "duration in 1 subtitle -> " +
              scratch.of("out/d.srt") + "\n");
}

TEST_CASE("a file within its limits says there was nothing to adjust", "[e2e][CLI-ADJUST-04]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    const CliRun run = invoke(
        {"adjust", "--speed", "off", "--minimum", "off", "--output-dir", scratch.of("out"), input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(": no duration to adjust -> "));
    // Written all the same: a destination given is a destination written.
    CHECK(contentOf(scratch.of("out/d.srt")) == kInput);
}

TEST_CASE("every constraint off, or none left to apply, is a usage error", "[e2e][CLI-ADJUST-03]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    for (const std::vector<std::string>& arguments : std::vector<std::vector<std::string>>{
             {"--speed", "off", "--minimum", "off", "--gap", "off"},
             // A speed told to move no end is no constraint either.
             {"--no-lengthen", "--minimum", "off", "--gap", "off"}}) {
        std::vector<std::string> line{"--format", "json", "adjust"};
        line.insert(line.end(), arguments.begin(), arguments.end());
        line.insert(line.end(), {"--output-dir", scratch.of("out"), input});
        const CliRun run = invoke(line);

        CHECK(run.exitCode == 1);
        CHECK(run.output.empty());
        CHECK_THAT(run.errors, ContainsSubstring("nothing to adjust to"));
        CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
    }
}

TEST_CASE("a value that is no constraint is refused before any file is read",
          "[e2e][CLI-ADJUST-02]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    struct Refusal {
        std::vector<std::string> arguments;
        std::string said;
    };

    for (const Refusal& one : std::vector<Refusal>{
             {.arguments = {"--speed", "0"}, .said = "--speed: \"0\" is not a reading speed"},
             {.arguments = {"--speed", "fast"}, .said = "--speed: \"fast\" is not a reading speed"},
             {.arguments = {"--speed", "1e3"}, .said = "--speed: \"1e3\" is not a reading speed"},
             {.arguments = {"--speed", "off", "--shorten"},
              .said = "--speed off switches the reading speed off"},
             {.arguments = {"--minimum", "-1"},
              .said = "--minimum: \"-1\" is not a duration of zero or more"},
             {.arguments = {"--maximum", "0"},
              .said = "--maximum: \"0\" is not a duration above zero"},
             {.arguments = {"--gap", "soon"}, .said = "--gap: "},
             {.arguments = {"--range", "5"}, .said = "--range: \"5\" is not a range"}}) {
        std::vector<std::string> line{"adjust"};
        line.insert(line.end(), one.arguments.begin(), one.arguments.end());
        line.insert(line.end(), {"--output-dir", scratch.of("out"), input});
        const CliRun run = invoke(line);

        INFO(one.said);
        CHECK(run.exitCode == 1);
        CHECK_THAT(run.errors, ContainsSubstring(one.said));
        CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
    }
}

TEST_CASE("a range beyond the file fails that file, naming the bound, and writes nothing",
          "[e2e][CLI-ADJUST-02][CLI-RANGE-02]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    const CliRun run = invoke(
        {"--format", "json", "adjust", "--range", "3-9", "--output-dir", scratch.of("out"), input});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors,
               ContainsSubstring("range 3-9 ends after the last subtitle: the file has 4"));
    CHECK_THAT(run.output, ContainsSubstring("\"kind\":\"range-out-of-bounds\""));
    CHECK_FALSE(std::filesystem::exists(scratch.of("out/d.srt")));
}

TEST_CASE("a dry run counts and writes nothing, without a destination",
          "[e2e][CLI-ADJUST-04][CLI-ADJUST-05][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/d.srt", kInput);

    const CliRun run = invoke({"--format", "json", "adjust", "--dry-run", input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(
        anonymised(run.output, scratch.path(), "<scratch>"),
        ContainsSubstring("\"dry_run\":true,\"destination\":null,\"counts\":{\"subtitles\":4,"
                          "\"adjusted\":3,\"sacrificed\":{\"speed\":0,\"minimum\":1,\"gap\":0}}"));
    CHECK_THAT(run.errors, ContainsSubstring("(dry run, nothing written)"));
    CHECK(contentOf(input) == kInput);
}

TEST_CASE("what adjust sacrifices on the fixtures is what was worked out by hand",
          "[e2e][CLI-ADJUST-06]") {
    // Nine small files, one for each pair of constraints and each overlap — see
    // `data/durees/LISEZMOI.md`. The expected counts were calculated from the
    // definitions, on paper; the script that predicts them
    // (`measure-duration-constraints.py --check-fixtures`) is held to the same
    // files, so that if the two ever disagree one of them is wrong, and which one.
    const std::vector<std::string> names{"arrondi.srt",
                                         "balises.srt",
                                         "deux-lignes.srt",
                                         "ecart.srt",
                                         "lecture.vtt",
                                         "minimum-contre-ecart.srt",
                                         "union.srt",
                                         "vitesse-contre-ecart.srt",
                                         "vitesse-contre-maximum.srt"};
    const std::string root = corpus("").substr(0, corpus("").find_last_not_of('/') + 1);

    struct Setting {
        std::vector<std::string> options;
        std::string expected;
    };

    for (const Setting& setting : std::vector<Setting>{
             {.options = {"--maximum", "6"}, .expected = "attendus/json/recoupement-defaut.jsonl"},
             {.options = {"--maximum", "6", "--gap", "0.5"},
              .expected = "attendus/json/recoupement-ecart.jsonl"}}) {
        std::vector<std::string> line{"--format", "json", "adjust", "--dry-run"};
        line.insert(line.end(), setting.options.begin(), setting.options.end());
        for (const std::string& name : names) {
            line.push_back(corpus("durees/" + name));
        }

        const CliRun run = invoke(line);

        INFO(setting.expected);
        CHECK(run.exitCode == 0);
        CHECK_THAT(anonymised(run.output, root, "<corpus>"), MatchesFile(corpus(setting.expected)));
    }
}
