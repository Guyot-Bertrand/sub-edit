// `-t`, `--document` and `--align-method`, through the real binary.
//
// The eight pairs of `data/paires/` carry, in `LISEZMOI.md`, what each method
// must do with each translation — worked out by hand, **before** anything
// aligned anything. The counts below are that table read as numbers, never
// copied from the program: `inspect -t` is proved against them.
//
// What writes a translation is proved on one input of our own, and its expected
// file is written by hand as well (`attendus/LISEZMOI.md`).

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <filesystem>
#include <string>
#include <string_view>

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

/// What a method makes of one translation: the four counts of the report.
struct Counts {
    int attached;
    int born;
    int untranslated;
    int outOfOrder;
};

[[nodiscard]] constexpr Counts counts(int attached, int born, int untranslated, int outOfOrder) {
    return Counts{
        .attached = attached, .born = born, .untranslated = untranslated, .outOfOrder = outOfOrder};
}

struct PairExpectation {
    std::string_view directory;
    Counts byPosition;
    Counts byNumber;
};

// The table of `paires/LISEZMOI.md`, line by line. The main document holds four
// subtitles; a translation of four lines laid exactly on them is the control.
constexpr std::array<PairExpectation, 8> kPairs{
    // four lines, at the positions of the main file
    PairExpectation{
        .directory = "temoin", .byPosition = counts(4, 0, 0, 0), .byNumber = counts(4, 0, 0, 0)},
    // the line of S4 is missing
    PairExpectation{.directory = "traduction-plus-courte",
                    .byPosition = counts(3, 0, 1, 0),
                    .byNumber = counts(3, 0, 1, 0)},
    // order 2, 1, 4, 3: two lines come after a later one
    PairExpectation{.directory = "traduction-dans-le-desordre",
                    .byPosition = counts(4, 0, 0, 2),
                    .byNumber = counts(4, 0, 0, 2)},
    // the line of S2 is missing: by number S4 is left alone, by position S2 is
    PairExpectation{.directory = "une-ligne-de-moins-au-milieu",
                    .byPosition = counts(3, 0, 1, 0),
                    .byNumber = counts(3, 0, 1, 0)},
    // a fifth line, at 16-18 s: a subtitle is born, by both methods
    PairExpectation{.directory = "une-ligne-de-plus-a-la-fin",
                    .byPosition = counts(4, 1, 0, 0),
                    .byNumber = counts(4, 1, 0, 0)},
    // a line in the empty interval: it is born, or it slides and the last one is born
    PairExpectation{.directory = "une-ligne-dans-un-intervalle-vide",
                    .byPosition = counts(4, 1, 0, 0),
                    .byNumber = counts(4, 1, 0, 0)},
    // S3 in two lines: the second is born, or slides onto S4
    PairExpectation{.directory = "deux-lignes-dans-un-meme-sous-titre",
                    .byPosition = counts(4, 1, 0, 0),
                    .byNumber = counts(4, 1, 0, 0)},
    // all two seconds late: one line falls on S2 and three are born, or each goes to its number
    PairExpectation{.directory = "positions-decalees",
                    .byPosition = counts(1, 3, 3, 0),
                    .byNumber = counts(4, 0, 0, 0)},
};

[[nodiscard]] std::string pairFile(std::string_view directory, std::string_view file) {
    return corpus("paires/" + std::string{directory} + "/" + std::string{file});
}

[[nodiscard]] std::string countsOf(const Counts& counts) {
    return "\"attached\":" + std::to_string(counts.attached) +
           ",\"born\":" + std::to_string(counts.born) +
           ",\"untranslated\":" + std::to_string(counts.untranslated) +
           ",\"out_of_order\":" + std::to_string(counts.outOfOrder) + "}";
}

/// The translation of `temoin`, laid on the four subtitles of its main file,
/// with a mention in three of them: the first is nothing else, and is emptied.
const std::string kMentions =
    "1\n00:00:01,000 --> 00:00:03,000\n[Un oiseau chante]\n\n"
    "2\n00:00:04,000 --> 00:00:06,000\nRien ne bouge [il tousse] sur l'eau.\n\n"
    "3\n00:00:08,000 --> 00:00:11,000\nVoir [1] la note.\n\n"
    "4\n00:00:12,000 --> 00:00:15,000\nPuis la lumière vient.\n\n";

} // namespace

TEST_CASE("inspect -t reports how the lines of every pair were matched", "[e2e][CLI-TRANS-01]") {
    for (const PairExpectation& pair : kPairs) {
        for (const bool byNumber : {false, true}) {
            INFO(pair.directory << (byNumber ? " by number" : " by position"));
            const CliRun run = invoke({"--format",
                                       "json",
                                       "inspect",
                                       pairFile(pair.directory, "principal.srt"),
                                       "-t",
                                       pairFile(pair.directory, "traduction.srt"),
                                       "--align-method",
                                       byNumber ? "number" : "position"});

            CHECK(run.exitCode == 0);
            CHECK_THAT(run.output,
                       ContainsSubstring(std::string{"\"method\":\""} +
                                         (byNumber ? "number" : "position") + "\","));
            CHECK_THAT(run.output,
                       ContainsSubstring(countsOf(byNumber ? pair.byNumber : pair.byPosition)));
        }
    }
}

TEST_CASE("the position is the default way to match, and the report says it",
          "[e2e][CLI-TRANS-05]") {
    const CliRun run = invoke({"inspect",
                               pairFile("positions-decalees", "principal.srt"),
                               "-t",
                               pairFile("positions-decalees", "traduction.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("matched by position\n"));
    CHECK_THAT(run.output,
               ContainsSubstring("  translation: 1 line attached; 3 subtitles born of a line; "
                                 "3 subtitles left without a translation\n"));
    // The report of the main file is of the file alone: a subtitle born of a
    // line is not one of its subtitles.
    CHECK_THAT(run.output, ContainsSubstring("  subtitles: 4\n"));
}

TEST_CASE("inspect without -t writes no translation key", "[e2e][CLI-TRANS-01]") {
    const CliRun run = invoke({"--format", "json", "inspect", pairFile("temoin", "principal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, !ContainsSubstring("\"translation\""));
}

TEST_CASE("a document of translation needs -t, and -t needs a document of translation",
          "[e2e][CLI-TRANS-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("x.srt");
    const std::string main = pairFile("temoin", "principal.srt");
    const std::string translation = pairFile("temoin", "traduction.srt");

    const CliRun withoutFile =
        invoke({"hearing-impaired", main, "--document", "translation", "--output", out});
    CHECK(withoutFile.exitCode == 1);
    CHECK_THAT(withoutFile.errors, ContainsSubstring("--document translation needs"));

    const CliRun withoutDocument =
        invoke({"hearing-impaired", main, "-t", translation, "--output", out});
    CHECK(withoutDocument.exitCode == 1);
    CHECK_THAT(withoutDocument.errors, ContainsSubstring("use --document translation"));

    const CliRun withMainDocument = invoke(
        {"case", "--to", "upper", main, "-t", translation, "--document", "main", "--output", out});
    CHECK(withMainDocument.exitCode == 1);

    CHECK_FALSE(std::filesystem::exists(out));
}

TEST_CASE("an alignment method needs a translation file", "[e2e][CLI-TRANS-05]") {
    const Scratch scratch;
    const CliRun run = invoke({"hearing-impaired",
                               pairFile("temoin", "principal.srt"),
                               "--align-method",
                               "number",
                               "--output",
                               scratch.of("x.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("--align-method needs a translation file"));
}

TEST_CASE("a translation file is refused with a batch, before anything is written",
          "[e2e][CLI-TRANS-03]") {
    const Scratch scratch;
    const std::string main = pairFile("temoin", "principal.srt");
    const std::string other = pairFile("positions-decalees", "principal.srt");

    const CliRun run = invoke({"hearing-impaired",
                               main,
                               other,
                               "-t",
                               pairFile("temoin", "traduction.srt"),
                               "--document",
                               "translation",
                               "--output-dir",
                               scratch.path()});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("-t names one file but several inputs were given"));
    CHECK(std::filesystem::is_empty(scratch.path()));
}

TEST_CASE("a translation that is the main file itself is refused", "[e2e][CLI-TRANS-02]") {
    const Scratch scratch;
    const std::string main = pairFile("temoin", "principal.srt");

    const CliRun run = invoke({"case",
                               "--to",
                               "upper",
                               main,
                               "-t",
                               main,
                               "--document",
                               "translation",
                               "--output",
                               scratch.of("x.srt")});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("the file is already open as the main document"));
    CHECK_FALSE(std::filesystem::exists(scratch.of("x.srt")));
}

TEST_CASE("the translation written after hearing-impaired is the expected one, byte for byte",
          "[e2e][CLI-TRANS-04]") {
    // The mention that empties a translation leaves it empty and keeps the
    // subtitle: there are four blocks in, and four out.
    const Scratch scratch;
    const std::string translation = writeFile(scratch, "traduction.srt", kMentions);
    const std::string out = scratch.of("nettoyee.srt");

    const CliRun run = invoke({"--quiet",
                               "hearing-impaired",
                               pairFile("temoin", "principal.srt"),
                               "-t",
                               translation,
                               "--document",
                               "translation",
                               "--output",
                               out});

    CHECK(run.exitCode == 0);
    CHECK_THAT(
        contentOf(out),
        MatchesFile(corpus("attendus/translation/mentions.hearing-impaired.traduction.srt")));
}

TEST_CASE("only the translation is written, to its own path", "[e2e][CLI-TRANS-04]") {
    const Scratch scratch;
    const std::string main =
        writeFile(scratch, "film.srt", contentOf(pairFile("temoin", "principal.srt")));
    const std::string translation = writeFile(scratch, "film.fr.srt", kMentions);

    SECTION("--in-place rewrites the translation and leaves the main file alone") {
        const CliRun run = invoke({"--quiet",
                                   "hearing-impaired",
                                   main,
                                   "-t",
                                   translation,
                                   "--document",
                                   "translation",
                                   "--in-place"});

        CHECK(run.exitCode == 0);
        CHECK_THAT(
            contentOf(translation),
            MatchesFile(corpus("attendus/translation/mentions.hearing-impaired.traduction.srt")));
        CHECK(contentOf(main) == contentOf(pairFile("temoin", "principal.srt")));
    }

    SECTION("--output-dir takes the base name of the translation") {
        const std::string dir = scratch.of("out");
        const CliRun run = invoke({"--quiet",
                                   "hearing-impaired",
                                   main,
                                   "-t",
                                   translation,
                                   "--document",
                                   "translation",
                                   "--output-dir",
                                   dir});

        CHECK(run.exitCode == 0);
        CHECK(std::filesystem::exists(dir + "/film.fr.srt"));
        CHECK_FALSE(std::filesystem::exists(dir + "/film.srt"));
        CHECK(contentOf(translation) == kMentions);
    }
}

TEST_CASE("a dry run lists the changes of the translation, and writes nothing",
          "[e2e][CLI-TRANS-04]") {
    const Scratch scratch;
    const std::string translation = writeFile(scratch, "traduction.srt", kMentions);

    const CliRun run = invoke({"replace",
                               "lumière",
                               "clarté",
                               pairFile("temoin", "principal.srt"),
                               "-t",
                               translation,
                               "--document",
                               "translation",
                               "--dry-run"});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring(translation + ": subtitle 4 (translation)\n"));
    CHECK_THAT(run.output,
               ContainsSubstring("- Puis la lumière vient.\n+ Puis la clarté vient.\n"));
    CHECK(contentOf(translation) == kMentions);
}

TEST_CASE("a record of a changed translation says how it was matched", "[e2e][CLI-TRANS-01]") {
    const Scratch scratch;
    const std::string translation = writeFile(scratch, "traduction.srt", kMentions);

    const CliRun run = invoke({"--format",
                               "json",
                               "case",
                               "--to",
                               "upper",
                               pairFile("temoin", "principal.srt"),
                               "-t",
                               translation,
                               "--document",
                               "translation",
                               "--align-method",
                               "number",
                               "--output",
                               scratch.of("x.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("\"alignment\":{\"file\":"));
    CHECK_THAT(run.output, ContainsSubstring("\"method\":\"number\""));
    CHECK_THAT(run.output, ContainsSubstring("\"document\":\"translation\""));
}
