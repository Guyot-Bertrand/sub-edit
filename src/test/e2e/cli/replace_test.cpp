// `replace`, through the real binary: look in what is shown, write into what is
// stored, and never break a tag.
//
// The expected texts are worked out by hand from the rules of
// `data/textes/recherche.cas` — the same ones the window's search obeys — and
// never read back from the program. The input is five subtitles, each there for
// one rule:
//
//   1  "Hello hello."                     the case, and several matches in one text
//   2  "<i>Bon</i>jour tout le monde"     a tag that cuts the word looked for
//   3  "Bonjour <i>Marie</i>, ca va"      a style on the second half of a match
//   4  "Prix: 12 euros, et 7 euros."      groups of a regular expression
//   5  "Unrelated"                        a text a match can be replaced by itself in

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

/// A SubRip file whose five subtitles say `texts`.
std::string srt(const std::vector<std::string>& texts) {
    // Two digits for the seconds: five subtitles end at ten seconds at most.
    const auto seconds = [](std::size_t n) {
        return std::string{"00:00:"} + (n < 10 ? "0" : "") + std::to_string(n) + ",000";
    };
    std::string out;
    for (std::size_t i = 0; i < texts.size(); ++i) {
        out += std::to_string(i + 1);
        out += '\n';
        out += seconds((2 * i) + 1);
        out += " --> ";
        out += seconds((2 * i) + 2);
        out += '\n';
        out += texts[i];
        out += "\n\n";
    }
    return out;
}

const std::vector<std::string> kTexts{"Hello hello.",
                                      "<i>Bon</i>jour tout le monde",
                                      "Bonjour <i>Marie</i>, ca va",
                                      "Prix: 12 euros, et 7 euros.",
                                      "Unrelated"};

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

/// What `replace` writes for `arguments`, or the run that refused to.
struct Replaced {
    CliRun run;
    std::string written{};
};

Replaced replace(const Scratch& scratch, std::vector<std::string> arguments) {
    const std::string input = writeFile(scratch, "in/r.srt", srt(kTexts));
    std::vector<std::string> line{"replace"};
    line.insert(line.end(), arguments.begin(), arguments.end());
    line.insert(line.end(), {"--output", scratch.of("out/r.srt"), input});
    Replaced result{.run = invoke(line)};
    if (std::filesystem::exists(scratch.of("out/r.srt"))) {
        result.written = contentOf(scratch.of("out/r.srt"));
    }
    return result;
}

} // namespace

TEST_CASE("plain text is replaced everywhere it is, whatever its case",
          "[e2e][CLI-REPLACE-01][CLI-REPLACE-03]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"hello", "bye"});

    CHECK(done.run.exitCode == 0);
    // « Hello » and « hello » are one word when the case is ignored, which is
    // the default — as in the window.
    CHECK(done.written == srt({"bye bye.", kTexts[1], kTexts[2], kTexts[3], kTexts[4]}));
    CHECK_THAT(done.run.errors, ContainsSubstring(": replaced 2 matches -> "));
}

TEST_CASE("case sensitivity tells a capital from a small letter", "[e2e][CLI-REPLACE-03]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"--case-sensitive", "hello", "bye"});

    CHECK(done.run.exitCode == 0);
    CHECK(done.written == srt({"Hello bye.", kTexts[1], kTexts[2], kTexts[3], kTexts[4]}));
    CHECK_THAT(done.run.errors, ContainsSubstring(": replaced 1 match -> "));
}

TEST_CASE("an expression reads its groups in the replacement", "[e2e][CLI-REPLACE-02]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"--regex", "([0-9]+) euros", "$1 EUR"});

    CHECK(done.run.exitCode == 0);
    CHECK(done.written ==
          srt({kTexts[0], kTexts[1], kTexts[2], "Prix: 12 EUR, et 7 EUR.", kTexts[4]}));
}

TEST_CASE("a match that a tag cuts is replaced without breaking the tag", "[e2e][CLI-REPLACE-01]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"Bonjour", "Salut"});

    CHECK(done.run.exitCode == 0);
    // The tag that cut the word now holds its replacement, whole — rule 2 of
    // `recherche.cas` — and the one next to the second match stays where it was.
    CHECK_THAT(done.written, MatchesFile(corpus("attendus/replace/bonjour-salut.srt")));
}

TEST_CASE("a style that touches a match covers all of its replacement", "[e2e][CLI-REPLACE-01]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"Bonjour Marie", "Salut Sophie"});

    CHECK(done.run.exitCode == 0);
    CHECK(done.written ==
          srt({kTexts[0], kTexts[1], "<i>Salut Sophie</i>, ca va", kTexts[3], kTexts[4]}));
}

TEST_CASE("a pattern that is nowhere is said, and the file is written as it was",
          "[e2e][CLI-REPLACE-04]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"zzz", "x"});

    CHECK(done.run.exitCode == 0);
    CHECK_THAT(done.run.errors, ContainsSubstring(": \"zzz\" not found -> "));
    CHECK(done.written == srt(kTexts));
}

TEST_CASE("a match replaced by itself changes nothing, and is not 'not found'",
          "[e2e][CLI-REPLACE-04]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"Unrelated", "Unrelated"});

    CHECK(done.run.exitCode == 0);
    CHECK_THAT(done.run.errors, ContainsSubstring(": nothing to change -> "));
    CHECK(done.written == srt(kTexts));
}

TEST_CASE("an expression that cannot be read is refused before any file is read, with the reason",
          "[e2e][CLI-REPLACE-02]") {
    const Scratch scratch;

    for (const std::vector<std::string>& arguments :
         std::vector<std::vector<std::string>>{{"--regex", "(", "x"}, {"--regex", "[a-", "x"}}) {
        const Replaced done = replace(scratch, arguments);

        CHECK(done.run.exitCode == 1);
        CHECK(done.run.output.empty());
        CHECK_THAT(done.run.errors, ContainsSubstring("pattern: not a regular expression ("));
        CHECK(done.written.empty());
    }

    // Nothing to look for is no pattern either.
    const Replaced empty = replace(scratch, {"", "x"});
    CHECK(empty.run.exitCode == 1);
    CHECK_THAT(empty.run.errors, ContainsSubstring("pattern: nothing to look for"));
    CHECK(empty.written.empty());
}

TEST_CASE("a range limits the subtitles looked in", "[e2e][CLI-REPLACE-01]") {
    const Scratch scratch;

    const Replaced done = replace(scratch, {"--range", "2-2", "Bonjour", "Salut"});

    CHECK(done.run.exitCode == 0);
    CHECK(done.written ==
          srt({kTexts[0], "<i>Salut</i> tout le monde", kTexts[2], kTexts[3], kTexts[4]}));
}

TEST_CASE("a dry run proposes each change as number, text before and text after",
          "[e2e][CLI-REPLACE-04][CLI-DRYRUN-04]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/r.srt", srt(kTexts));

    const CliRun run = invoke({"replace", "--dry-run", "Bonjour", "Salut", input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch.path(), "<scratch>"),
               MatchesFile(corpus("attendus/replace/dry-run.txt")));
    CHECK_THAT(run.errors, ContainsSubstring("replaced 2 matches (dry run, nothing written)"));
    CHECK(contentOf(input) == srt(kTexts));
}

TEST_CASE("the same changes are the json, with the counts of matches",
          "[e2e][CLI-REPLACE-04][CLI-DRYRUN-05]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/r.srt", srt(kTexts));

    const CliRun run =
        invoke({"--format", "json", "replace", "--dry-run", "Bonjour", "Salut", input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch.path(), "<scratch>"),
               MatchesFile(corpus("attendus/json/replace-dry-run.jsonl")));
}
