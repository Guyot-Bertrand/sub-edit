// `case`, `italics` and `dialogue-dashes`, through the real binary: the texts
// change, the tags stay where they were.
//
// The expected texts are worked out by hand from the rules of `data/textes/`
// — `casse-*.cas` and `tirets.cas`, the corpora the window's operations obey —
// and never read back from the program.

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

/// A SubRip file whose subtitles say `texts`, each lasting a second.
std::string srt(const std::vector<std::string>& texts) {
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

/// For the case: a plain text, a shout, a tag in capitals, a tag that cuts a
/// word, two lines, and a text already as it will be.
const std::vector<std::string> kCase{"bonjour marie",
                                     "- BONJOUR MARIE",
                                     "<I>bonjour</I> marie",
                                     "<i>bon</i>jour marie",
                                     "Bonjour\nMarie",
                                     "- Salut"};

/// For the italics: nothing, a cut italic, two lines, and a text already in them.
const std::vector<std::string> kItalics{
    "bonjour marie", "<i>bon</i>jour marie", "Bonjour\nMarie", "<i>Salut</i>"};

/// For the dashes: none, two lines, one already, a tag, and two lines with both.
const std::vector<std::string> kDashes{
    "Bonjour", "Bonjour\nMarie", "- Salut", "<i>Bonjour</i>", "- Bonjour\n- Marie"};

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

/// What a subcommand writes for `arguments` over `texts`, and how it ended.
struct Rewritten {
    CliRun run;
    std::string written{};
};

Rewritten rewrite(const Scratch& scratch,
                  const std::vector<std::string>& texts,
                  std::vector<std::string> arguments) {
    const std::string input = writeFile(scratch, "in/c.srt", srt(texts));
    arguments.insert(arguments.end(), {"--output", scratch.of("out/c.srt"), input});
    Rewritten result{.run = invoke(arguments)};
    if (std::filesystem::exists(scratch.of("out/c.srt"))) {
        result.written = contentOf(scratch.of("out/c.srt"));
    }
    return result;
}

} // namespace

TEST_CASE("title case puts a capital on each word and leaves the tags where they were",
          "[e2e][CLI-CASE-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, kCase, {"case", "--to", "title"});

    CHECK(done.run.exitCode == 0);
    // The capital tag stays capital, the tag cutting a word still cuts it, and
    // what precedes the first letter — a dash — is not touched.
    CHECK_THAT(done.written, MatchesFile(corpus("attendus/texte/casse-titre.srt")));
    CHECK_THAT(done.run.errors, ContainsSubstring(": 4 subtitles recased -> "));
}

TEST_CASE("each of the four cases changes what it should and nothing else", "[e2e][CLI-CASE-01]") {
    struct Wanted {
        std::string to;
        std::vector<std::string> texts;
        std::string said;
    };

    const std::vector<Wanted> cases{{.to = "sentence",
                                     .texts = {"Bonjour marie",
                                               "- Bonjour marie",
                                               "<I>Bonjour</I> marie",
                                               "<i>Bon</i>jour marie",
                                               "Bonjour\nmarie",
                                               "- Salut"},
                                     .said = "5 subtitles recased"},
                                    {.to = "upper",
                                     .texts = {"BONJOUR MARIE",
                                               "- BONJOUR MARIE",
                                               "<I>BONJOUR</I> MARIE",
                                               "<i>BON</i>JOUR MARIE",
                                               "BONJOUR\nMARIE",
                                               "- SALUT"},
                                     .said = "5 subtitles recased"},
                                    {.to = "lower",
                                     .texts = {"bonjour marie",
                                               "- bonjour marie",
                                               "<I>bonjour</I> marie",
                                               "<i>bon</i>jour marie",
                                               "bonjour\nmarie",
                                               "- salut"},
                                     .said = "3 subtitles recased"}};

    for (const Wanted& wanted : cases) {
        const Scratch scratch;
        INFO(wanted.to);

        const Rewritten done = rewrite(scratch, kCase, {"case", "--to", wanted.to});

        CHECK(done.run.exitCode == 0);
        CHECK(done.written == srt(wanted.texts));
        CHECK_THAT(done.run.errors, ContainsSubstring(wanted.said));
    }
}

TEST_CASE("a word that is no case is a usage error", "[e2e][CLI-CASE-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, kCase, {"case", "--to", "loud"});

    CHECK(done.run.exitCode == 1);
    CHECK(done.written.empty());
}

TEST_CASE("a text already in the case asked for is nothing to change", "[e2e][CLI-CASE-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, {"BONJOUR", "- SALUT"}, {"case", "--to", "upper"});

    CHECK(done.run.exitCode == 0);
    CHECK_THAT(done.run.errors, ContainsSubstring(": nothing to change -> "));
    CHECK(done.written == srt({"BONJOUR", "- SALUT"}));
}

TEST_CASE("italics --on wraps the text once, and a cut italic is made whole",
          "[e2e][CLI-ITALIC-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, kItalics, {"italics", "--on"});

    CHECK(done.run.exitCode == 0);
    // Not wrapped twice where it already was, and one pair across the two lines.
    CHECK_THAT(done.written, MatchesFile(corpus("attendus/texte/italiques-on.srt")));
    CHECK_THAT(done.run.errors, ContainsSubstring(": 3 subtitles put in italics -> "));
}

TEST_CASE("italics --off takes the italics out and touches nothing else", "[e2e][CLI-ITALIC-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, kItalics, {"italics", "--off"});

    CHECK(done.run.exitCode == 0);
    CHECK(done.written == srt({"bonjour marie", "bonjour marie", "Bonjour\nMarie", "Salut"}));
    CHECK_THAT(done.run.errors, ContainsSubstring(": 2 subtitles taken out of italics -> "));
}

TEST_CASE("italics needs one way, and only one", "[e2e][CLI-ITALIC-01]") {
    const Scratch scratch;

    for (const std::vector<std::string>& arguments :
         std::vector<std::vector<std::string>>{{"italics"}, {"italics", "--on", "--off"}}) {
        const Rewritten done = rewrite(scratch, kItalics, arguments);

        CHECK(done.run.exitCode == 1);
        CHECK(done.run.output.empty());
        CHECK(done.written.empty());
    }
}

TEST_CASE("a format that writes no style refuses, file by file, and the others go on",
          "[e2e][CLI-ITALIC-02]") {
    const Scratch scratch;
    const std::string good = writeFile(scratch, "in/c.srt", srt(kItalics));

    for (const char* way : {"--on", "--off"}) {
        const CliRun run = invoke({"--format",
                                   "json",
                                   "italics",
                                   way,
                                   "--output-dir",
                                   scratch.of("out"),
                                   corpus("formats/scene.tmplayer.txt"),
                                   good,
                                   corpus("formats/scene.lrc")});

        INFO(way);
        // One of three came out; the two that write no style said why.
        CHECK(run.exitCode == 3);
        CHECK_THAT(run.errors,
                   ContainsSubstring("TMPlayer writes no style: there are no italics to "));
        CHECK_THAT(run.errors, ContainsSubstring("LRC writes no style: there are no italics to "));
        CHECK_THAT(run.output, ContainsSubstring("\"kind\":\"no-style\""));
        CHECK_THAT(run.errors, ContainsSubstring("1 of 3 files changed, 2 failed"));
        CHECK(std::filesystem::exists(scratch.of("out/c.srt")));
        CHECK_FALSE(std::filesystem::exists(scratch.of("out/scene.tmplayer.txt")));
        CHECK_FALSE(std::filesystem::exists(scratch.of("out/scene.lrc")));
    }
}

TEST_CASE("dialogue-dashes --add puts a hyphen at the head of each line, inside a tag",
          "[e2e][CLI-DASH-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, kDashes, {"dialogue-dashes", "--add"});

    CHECK(done.run.exitCode == 0);
    // A dash already there is not doubled, and the tag that held the start of
    // the line still holds it.
    CHECK_THAT(done.written, MatchesFile(corpus("attendus/texte/tirets-add.srt")));
    CHECK_THAT(done.run.errors, ContainsSubstring(": 3 subtitles dashed -> "));
}

TEST_CASE("dialogue-dashes --remove takes the dashes off", "[e2e][CLI-DASH-01]") {
    const Scratch scratch;

    const Rewritten done = rewrite(scratch, kDashes, {"dialogue-dashes", "--remove"});

    CHECK(done.run.exitCode == 0);
    CHECK(done.written ==
          srt({"Bonjour", "Bonjour\nMarie", "Salut", "<i>Bonjour</i>", "Bonjour\nMarie"}));
    CHECK_THAT(done.run.errors, ContainsSubstring(": 2 subtitles undashed -> "));
}

TEST_CASE("dialogue-dashes needs one way, and only one", "[e2e][CLI-DASH-01]") {
    const Scratch scratch;

    for (const std::vector<std::string>& arguments : std::vector<std::vector<std::string>>{
             {"dialogue-dashes"}, {"dialogue-dashes", "--add", "--remove"}}) {
        const Rewritten done = rewrite(scratch, kDashes, arguments);

        CHECK(done.run.exitCode == 1);
        CHECK(done.written.empty());
    }
}

TEST_CASE("a range limits the three to the subtitles named", "[e2e][CLI-CASE-01][CLI-DASH-01]") {
    const Scratch scratch;

    const Rewritten recased = rewrite(scratch, kCase, {"case", "--to", "upper", "--range", "1-1"});
    CHECK(recased.written ==
          srt({"BONJOUR MARIE", kCase[1], kCase[2], kCase[3], kCase[4], kCase[5]}));

    const Scratch other;
    const Rewritten dashed = rewrite(other, kDashes, {"dialogue-dashes", "--add", "--range", "2-"});
    CHECK(dashed.written == srt({"Bonjour",
                                 "- Bonjour\n- Marie",
                                 "- Salut",
                                 "<i>- Bonjour</i>",
                                 "- Bonjour\n- Marie"}));
}

TEST_CASE("a dry run of each proposes number, text before and text after, in json too",
          "[e2e][CLI-CASE-01][CLI-ITALIC-01][CLI-DASH-01][CLI-DRYRUN-05]") {
    const Scratch scratch;
    const std::string cased = writeFile(scratch, "in/c.srt", srt(kCase));
    const std::string italic = writeFile(scratch, "in/i.srt", srt(kItalics));
    const std::string dashed = writeFile(scratch, "in/d.srt", srt(kDashes));

    // A destination is given, so that the absence of a file is something observed.
    const std::string out = scratch.of("out");
    std::string all;
    for (const std::vector<std::string>& line : std::vector<std::vector<std::string>>{
             {"case", "--to", "title", "--dry-run", "--output-dir", out, cased},
             {"italics", "--on", "--dry-run", "--output-dir", out, italic},
             {"dialogue-dashes", "--add", "--dry-run", "--output-dir", out, dashed}}) {
        std::vector<std::string> command{"--format", "json"};
        command.insert(command.end(), line.begin(), line.end());
        const CliRun run = invoke(command);

        REQUIRE(run.exitCode == 0);
        all += anonymised(run.output, scratch.path(), "<scratch>");
    }

    CHECK_THAT(all, MatchesFile(corpus("attendus/json/texte-dry-run.jsonl")));
    // Dry: nothing was written, not even the directory, and all three inputs are as
    // they were.
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK(contentOf(cased) == srt(kCase));
    CHECK(contentOf(italic) == srt(kItalics));
    CHECK(contentOf(dashed) == srt(kDashes));
}
