// `pair`, through the real binary: a translation written at the positions of its
// main file, and the alignment said.
//
// The expected files are those of `data/paires/`, **written by hand from a reading
// of Gaupol** before anything aligned anything — one per pair and per method —
// and never read back from the program.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include <cstddef>
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

// The eight cases of `paires/`, each with the sentence its two methods lead to.
struct PairCase {
    std::string_view directory;
    std::string_view byPosition;
    std::string_view byNumber;
};

constexpr std::array<PairCase, 8> kCases{
    PairCase{.directory = "temoin",
             .byPosition = "translation: 4 lines attached",
             .byNumber = "translation: 4 lines attached"},
    PairCase{.directory = "traduction-plus-courte",
             .byPosition = "translation: 3 lines attached; 1 subtitle left without a translation",
             .byNumber = "translation: 3 lines attached; 1 subtitle left without a translation"},
    PairCase{.directory = "traduction-dans-le-desordre",
             .byPosition = "translation: 4 lines attached; 2 lines out of order",
             .byNumber = "translation: 4 lines attached; 2 lines out of order"},
    PairCase{.directory = "une-ligne-de-moins-au-milieu",
             .byPosition = "translation: 3 lines attached; 1 subtitle left without a translation",
             .byNumber = "translation: 3 lines attached; 1 subtitle left without a translation"},
    PairCase{.directory = "une-ligne-de-plus-a-la-fin",
             .byPosition = "translation: 4 lines attached; 1 subtitle born of a line",
             .byNumber = "translation: 4 lines attached; 1 subtitle born of a line"},
    PairCase{.directory = "une-ligne-dans-un-intervalle-vide",
             .byPosition = "translation: 4 lines attached; 1 subtitle born of a line",
             .byNumber = "translation: 4 lines attached; 1 subtitle born of a line"},
    PairCase{.directory = "deux-lignes-dans-un-meme-sous-titre",
             .byPosition = "translation: 4 lines attached; 1 subtitle born of a line",
             .byNumber = "translation: 4 lines attached; 1 subtitle born of a line"},
    PairCase{.directory = "positions-decalees",
             .byPosition =
                 "translation: 1 line attached; 3 subtitles born of a line; 3 subtitles left "
                 "without a translation",
             .byNumber = "translation: 4 lines attached"},
};

[[nodiscard]] std::string pairFile(std::string_view directory, std::string_view file) {
    return corpus("paires/" + std::string{directory} + "/" + std::string{file});
}

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

} // namespace

TEST_CASE("the translation written is the expected one, for every pair and both methods",
          "[e2e][CLI-PAIR-01]") {
    const Scratch scratch;
    for (const PairCase& pair : kCases) {
        for (const bool byNumber : {false, true}) {
            INFO(pair.directory << (byNumber ? " by number" : " by position"));
            const std::string out = scratch.of("out.srt");

            const CliRun run = invoke({"pair",
                                       pairFile(pair.directory, "principal.srt"),
                                       "-t",
                                       pairFile(pair.directory, "traduction.srt"),
                                       "--align-method",
                                       byNumber ? "number" : "position",
                                       "--output",
                                       out});

            CHECK(run.exitCode == 0);
            CHECK_THAT(contentOf(out),
                       MatchesFile(pairFile(pair.directory,
                                            byNumber ? "attendu-numero.traduction.srt"
                                                     : "attendu-position.traduction.srt")));
            // Said at the default level: the alignment is the result.
            CHECK_THAT(run.errors,
                       ContainsSubstring(std::string{byNumber ? pair.byNumber : pair.byPosition} +
                                         " -> "));
        }
    }
}

TEST_CASE("the position is the way to match when none is given",
          "[e2e][CLI-PAIR-01][CLI-TRANS-05]") {
    const Scratch scratch;
    const std::string out = scratch.of("out.srt");

    const CliRun run = invoke({"pair",
                               pairFile("positions-decalees", "principal.srt"),
                               "-t",
                               pairFile("positions-decalees", "traduction.srt"),
                               "--output",
                               out});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out),
               MatchesFile(pairFile("positions-decalees", "attendu-position.traduction.srt")));
}

TEST_CASE("only the translation is written, and the main file is left alone",
          "[e2e][CLI-PAIR-01]") {
    const Scratch scratch;
    const std::string main =
        writeFile(scratch, "film.srt", contentOf(pairFile("temoin", "principal.srt")));
    const std::string translation =
        writeFile(scratch,
                  "film.fr.srt",
                  contentOf(pairFile("une-ligne-de-plus-a-la-fin", "traduction.srt")));

    SECTION("--in-place rewrites the translation") {
        const CliRun run = invoke({"--quiet", "pair", main, "-t", translation, "--in-place"});

        CHECK(run.exitCode == 0);
        CHECK_THAT(
            contentOf(translation),
            MatchesFile(pairFile("une-ligne-de-plus-a-la-fin", "attendu-position.traduction.srt")));
        CHECK(contentOf(main) == contentOf(pairFile("temoin", "principal.srt")));
    }

    SECTION("--output-dir takes the base name of the translation") {
        const std::string dir = scratch.of("out");

        const CliRun run =
            invoke({"--quiet", "pair", main, "-t", translation, "--output-dir", dir});

        CHECK(run.exitCode == 0);
        CHECK(std::filesystem::exists(dir + "/film.fr.srt"));
        CHECK_FALSE(std::filesystem::exists(dir + "/film.srt"));
    }
}

TEST_CASE("the record is one object carrying the alignment", "[e2e][CLI-PAIR-01][CLI-JSON-09]") {
    const Scratch scratch;
    const std::string main =
        writeFile(scratch, "in/film.srt", contentOf(pairFile("temoin", "principal.srt")));
    const std::string translation =
        writeFile(scratch, "in/traduction.srt", contentOf(pairFile("temoin", "traduction.srt")));

    const CliRun run = invoke({"--format",
                               "json",
                               "pair",
                               main,
                               "-t",
                               translation,
                               "--output",
                               scratch.of("out/fr.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch.path(), "<scratch>"),
               MatchesFile(corpus("attendus/json/pair.jsonl")));
}

TEST_CASE("a dry run says the alignment and writes nothing", "[e2e][CLI-PAIR-01][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("out/fr.srt");

    const CliRun named = invoke({"pair",
                                 "--dry-run",
                                 pairFile("temoin", "principal.srt"),
                                 "-t",
                                 pairFile("temoin", "traduction.srt"),
                                 "--output",
                                 out});
    const CliRun bare = invoke({"pair",
                                "--dry-run",
                                pairFile("temoin", "principal.srt"),
                                "-t",
                                pairFile("temoin", "traduction.srt")});

    CHECK(named.exitCode == 0);
    CHECK(bare.exitCode == 0);
    CHECK_THAT(bare.errors,
               ContainsSubstring("translation: 4 lines attached (dry run, nothing written)"));
    CHECK_FALSE(std::filesystem::exists(out));
}

TEST_CASE("pair asks for the translation, and for one main file", "[e2e][CLI-PAIR-01]") {
    const Scratch scratch;
    const std::string main = pairFile("temoin", "principal.srt");
    const std::string translation = pairFile("temoin", "traduction.srt");
    const std::string out = scratch.of("out.srt");

    const CliRun none = invoke({"pair", main, "--output", out});
    const CliRun two = invoke({"pair", main, main, "-t", translation, "--output", out});
    const CliRun document =
        invoke({"pair", main, "-t", translation, "--document", "translation", "--output", out});
    const CliRun destination = invoke({"pair", main, "-t", translation});

    CHECK(none.exitCode == 1);
    CHECK_THAT(none.errors, ContainsSubstring("pair needs the translation file"));
    CHECK(two.exitCode == 1);
    CHECK(document.exitCode == 1);
    CHECK(destination.exitCode == 1);
    CHECK_FALSE(std::filesystem::exists(out));
}

TEST_CASE("a translation that is the main file itself is refused, and nothing is written",
          "[e2e][CLI-PAIR-01]") {
    const Scratch scratch;
    const std::string main = pairFile("temoin", "principal.srt");
    const std::string out = scratch.of("out.srt");

    const CliRun run = invoke({"pair", main, "-t", main, "--output", out});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("the file is already open as the main document"));
    CHECK_FALSE(std::filesystem::exists(out));
}
