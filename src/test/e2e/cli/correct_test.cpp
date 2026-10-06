// `correct`, through the real binary: the tasks of the assistant, one property
// at a time.
//
// The expected texts are worked out by hand from the rules of the patterns the
// shipped files hold, and from the cases of `data/motifs/` — the ones Gaupol's
// own code was confronted with — never read back from the program. The patterns
// a case needs that the installation does not provide are fixtures of
// `data/motifs/utilisateur/`, dropped in the user's directory the harness moved.

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
using subedit::e2e::configHome;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::enchantHome;
using subedit::e2e::invoke;
using subedit::e2e::MatchesFile;
using subedit::e2e::Scratch;
using subedit::e2e::TestDictionary;
using subedit::e2e::UserPatterns;
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

/// The fixture of `data/motifs/utilisateur/`, dropped under its own name.
[[nodiscard]] UserPatterns dropped(const std::string& name) {
    return UserPatterns{name, contentOf(corpus("motifs/utilisateur/" + name))};
}

/// `count` letters `a` and a `b`: what `(a+)+$` never finishes on.
const std::string kCatastrophic = std::string(40, 'a') + "b";

} // namespace

TEST_CASE("correct needs its tasks, and none is ticked beforehand", "[e2e][CLI-CORRECT-01]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"eﬀet"}));

    const CliRun absent =
        invoke({"correct", "--code", "Latn", "--output", scratch.of("o.srt"), in});
    CHECK(absent.exitCode == 1);
    CHECK_THAT(absent.errors, ContainsSubstring("--tasks is required"));

    const CliRun unknown = invoke({"correct",
                                   "--tasks",
                                   "common-errors,tidy",
                                   "--code",
                                   "Latn",
                                   "--output",
                                   scratch.of("o.srt"),
                                   in});
    CHECK(unknown.exitCode == 1);
    CHECK_THAT(unknown.errors, ContainsSubstring("--tasks: \"tidy\" is not a task"));
    CHECK_FALSE(std::filesystem::exists(scratch.of("o.srt")));

    // Only the task named runs: capitalization would have made it « Eﬀet ».
    const std::string out = scratch.of("effet.srt");
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn",
                  "--output",
                  out,
                  in})
              .exitCode == 0);
    CHECK(contentOf(out) == srt({"effet"}));
}

TEST_CASE("the tasks run in Gaupol's order, whatever the order of the list",
          "[e2e][CLI-CORRECT-01]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"eﬀet ,voila"}));
    const std::string one = scratch.of("one.srt");
    const std::string two = scratch.of("two.srt");

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors,capitalization",
                  "--code",
                  "Latn-en",
                  "--output",
                  one,
                  in})
              .exitCode == 0);
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "capitalization,common-errors",
                  "--code",
                  "Latn-en",
                  "--output",
                  two,
                  in})
              .exitCode == 0);

    // Common errors first — the ligature, the space before the comma — then the
    // capital, on the text they left.
    CHECK(contentOf(one) == srt({"Effet, voila"}));
    CHECK(contentOf(two) == contentOf(one));
}

TEST_CASE("correct needs a code as soon as a task reads patterns", "[e2e][CLI-CORRECT-02]") {
    const Scratch scratch;
    const CliRun run = invoke({"correct",
                               "--tasks",
                               "common-errors",
                               "--output",
                               scratch.of("o.srt"),
                               writeFile(scratch, "a.srt", srt({"eﬀet"}))});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("--code is required by the tasks that read patterns"));
    CHECK_FALSE(std::filesystem::exists(scratch.of("o.srt")));

    const CliRun malformed = invoke({"correct",
                                     "--tasks",
                                     "common-errors",
                                     "--code",
                                     "english",
                                     "--output",
                                     scratch.of("o.srt"),
                                     scratch.of("a.srt")});
    CHECK(malformed.exitCode == 1);
    CHECK_THAT(malformed.errors, ContainsSubstring("is not a pattern code"));
}

TEST_CASE("the count says the texts changed and the subtitles removed, never the matches",
          "[e2e][CLI-CORRECT-03]") {
    const Scratch scratch;
    // Two matches in one text, one in another, none in the third.
    const std::string in = writeFile(scratch, "a.srt", srt({"eﬀet et ﬁn", "ﬁn", "fin"}));

    const CliRun run = invoke({"correct",
                               "--tasks",
                               "common-errors",
                               "--code",
                               "Latn",
                               "--output",
                               scratch.of("o.srt"),
                               in});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("a.srt: Edited 2 and removed 0 subtitles -> "));
    CHECK(contentOf(scratch.of("o.srt")) == srt({"effet et fin", "fin", "fin"}));
}

TEST_CASE("an unchecked class is left out of the application, not of the display",
          "[e2e][CLI-CORRECT-04]") {
    const Scratch scratch;
    // The ligatures are an OCR pattern; no pattern of the class « human » touches them.
    const std::string in = writeFile(scratch, "a.srt", srt({"eﬀet"}));
    const std::string human = scratch.of("human.srt");
    const std::string ocr = scratch.of("ocr.srt");
    const std::string both = scratch.of("both.srt");

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn",
                  "--classes",
                  "human",
                  "--output",
                  human,
                  in})
              .exitCode == 0);
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn",
                  "--classes",
                  "ocr",
                  "--output",
                  ocr,
                  in})
              .exitCode == 0);
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn",
                  "--classes",
                  "human,ocr",
                  "--output",
                  both,
                  in})
              .exitCode == 0);

    CHECK(contentOf(human) == srt({"eﬀet"}));
    CHECK(contentOf(ocr) == srt({"effet"}));
    CHECK(contentOf(both) == srt({"effet"}));

    const CliRun refused = invoke({"correct",
                                   "--tasks",
                                   "common-errors",
                                   "--code",
                                   "Latn",
                                   "--classes",
                                   "both",
                                   "--output",
                                   human,
                                   in});
    CHECK(refused.exitCode == 1);
    CHECK_THAT(refused.errors, ContainsSubstring("--classes: \"both\" is not a class"));
}

TEST_CASE("a pattern is switched on or off by its name, and a name that names nothing is refused",
          "[e2e][CLI-CORRECT-05]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"eﬀet"}));
    const std::string off = scratch.of("off.srt");

    // « Ligatures » is the name of seven records: one box, seven records.
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn",
                  "--disable",
                  "Ligatures",
                  "--output",
                  off,
                  in})
              .exitCode == 0);
    CHECK(contentOf(off) == srt({"eﬀet"}));

    const CliRun unknown = invoke({"correct",
                                   "--tasks",
                                   "common-errors",
                                   "--code",
                                   "Latn",
                                   "--enable",
                                   "Ligatuers",
                                   "--output",
                                   off,
                                   in});
    CHECK(unknown.exitCode == 1);
    CHECK_THAT(unknown.errors, ContainsSubstring("--enable: \"Ligatuers\" names no pattern"));

    const CliRun both = invoke({"correct",
                                "--tasks",
                                "common-errors",
                                "--code",
                                "Latn",
                                "--enable",
                                "Ligatures",
                                "--disable",
                                "Ligatures",
                                "--output",
                                off,
                                in});
    CHECK(both.exitCode == 1);
    CHECK_THAT(both.errors, ContainsSubstring("is given to --enable and to --disable"));
}

TEST_CASE("a name shared by two types is written with its type", "[e2e][CLI-CORRECT-05]") {
    const Scratch scratch;
    const UserPatterns first = dropped("Latn-xa.common-error");
    const UserPatterns second = dropped("Latn-xa.capitalization");
    const std::string in = writeFile(scratch, "a.srt", srt({"the colour"}));
    const std::string out = scratch.of("o.srt");

    const CliRun ambiguous = invoke({"correct",
                                     "--tasks",
                                     "common-errors,capitalization",
                                     "--code",
                                     "Latn-xa",
                                     "--enable",
                                     "Shared name",
                                     "--output",
                                     out,
                                     in});
    CHECK(ambiguous.exitCode == 1);
    CHECK_THAT(ambiguous.errors,
               ContainsSubstring("names patterns of several types: write type:name"));
    CHECK_FALSE(std::filesystem::exists(out));

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn-xa",
                  "--enable",
                  "common-error:Shared name",
                  "--output",
                  out,
                  in})
              .exitCode == 0);
    CHECK(contentOf(out) == srt({"the color"}));
}

TEST_CASE(
    "a task with no active pattern is refused, and the sound patterns are named like the others",
    "[e2e][CLI-CORRECT-06]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"Hi [laughs] there", "[Door]"}));
    const std::string out = scratch.of("o.srt");

    // The shipped `.conf` of `Latn` switches every mention pattern off.
    const CliRun idle =
        invoke({"correct", "--tasks", "mentions", "--code", "Latn", "--output", out, in});
    CHECK(idle.exitCode == 1);
    CHECK_THAT(idle.errors,
               ContainsSubstring("mentions: no pattern is active under the code Latn"));
    CHECK_FALSE(std::filesystem::exists(out));

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "mentions",
                  "--code",
                  "Latn",
                  "--enable",
                  "Sound in brackets",
                  "--output",
                  out,
                  in})
              .exitCode == 0);
    CHECK(contentOf(out) == srt({"Hi there"}));
}

TEST_CASE("a pattern the user dropped is added to the shipped ones", "[e2e][CLI-CORRECT-07]") {
    const Scratch scratch;
    const UserPatterns patterns = dropped("Latn-xb.common-error");
    const std::string out = scratch.of("o.srt");

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn-xb",
                  "--output",
                  out,
                  writeFile(scratch, "a.srt", srt({"the colour", "eﬀet"}))})
              .exitCode == 0);
    // Hers, and the shipped ligature beside it.
    CHECK(contentOf(out) == srt({"the color", "effet"}));
}

TEST_CASE("a pattern that cannot be applied is named with its subtitle, and the file is written",
          "[e2e][CLI-CORRECT-08]") {
    const Scratch scratch;
    const std::string out = scratch.of("o.srt");

    SECTION("a pattern that never finishes") {
        const UserPatterns patterns = dropped("Latn-xt.common-error");
        const CliRun run = invoke({"correct",
                                   "--tasks",
                                   "common-errors",
                                   "--code",
                                   "Latn-xt",
                                   "--output",
                                   out,
                                   writeFile(scratch, "a.srt", srt({"zzz", kCatastrophic}))});

        // Not a failure of the file: its code is that of a file that was written.
        CHECK(run.exitCode == 0);
        CHECK_THAT(
            run.errors,
            ContainsSubstring(
                "pattern \"Catastrophic backtracking\" (timed out) was not applied to subtitle 2"));
        // The other pattern did its part, and the text no pattern could read is as it was.
        CHECK(contentOf(out) == srt({"yyy", kCatastrophic}));
    }

    SECTION("a pattern that cannot be translated") {
        const UserPatterns patterns = dropped("Latn-xu.common-error");
        const CliRun run = invoke({"correct",
                                   "--tasks",
                                   "common-errors",
                                   "--code",
                                   "Latn-xu",
                                   "--output",
                                   out,
                                   writeFile(scratch, "a.srt", srt({"zzz"}))});

        CHECK(run.exitCode == 0);
        CHECK_THAT(run.errors,
                   ContainsSubstring("pattern \"Group name with an underscore\" (cannot be "
                                     "translated) was not applied\n"));
        CHECK(contentOf(out) == srt({"yyy"}));
    }

    SECTION("the subtitle is the file's own, whatever an earlier task removed") {
        const UserPatterns patterns = dropped("Latn-xt.common-error");
        const CliRun run =
            invoke({"correct",
                    "--tasks",
                    "mentions,common-errors",
                    "--code",
                    "Latn-xt",
                    "--enable",
                    "Sound in brackets",
                    "--output",
                    out,
                    writeFile(scratch, "a.srt", srt({"[Door]", "zzz", kCatastrophic}))});

        CHECK(run.exitCode == 0);
        CHECK_THAT(run.errors, ContainsSubstring("was not applied to subtitle 3"));
        // The first subtitle is gone, and the others keep the positions they had.
        CHECK(contentOf(out) == "1\n00:00:03,000 --> 00:00:04,000\nyyy\n\n"
                                "2\n00:00:05,000 --> 00:00:06,000\n" +
                                    kCatastrophic + "\n\n");
    }

    SECTION("the record says it too, as a warning") {
        const UserPatterns patterns = dropped("Latn-xt.common-error");
        const CliRun run = invoke({"--format",
                                   "json",
                                   "correct",
                                   "--tasks",
                                   "common-errors",
                                   "--code",
                                   "Latn-xt",
                                   "--output",
                                   out,
                                   writeFile(scratch, "a.srt", srt({"zzz", kCatastrophic}))});

        CHECK(run.exitCode == 0);
        CHECK_THAT(
            run.output,
            ContainsSubstring(
                "\"warnings\":[{\"kind\":\"pattern-failed\",\"detail\":\"pattern \\\"Catastrophic "
                "backtracking\\\" (timed out) was not applied to subtitle 2\"}]"));
        CHECK_THAT(run.output, ContainsSubstring("\"counts\":{\"corrected\":1,\"removed\":0}"));
    }
}

TEST_CASE("the subtitles the correction empties are removed, or kept when asked",
          "[e2e][CLI-CORRECT-12]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"[Door]", "Hello"}));
    const std::string removed = scratch.of("removed.srt");
    const std::string kept = scratch.of("kept.srt");

    const CliRun run = invoke({"correct",
                               "--tasks",
                               "mentions",
                               "--code",
                               "Latn",
                               "--enable",
                               "Sound in brackets",
                               "--output",
                               removed,
                               in});
    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("Edited 0 and removed 1 subtitles"));
    CHECK(contentOf(removed) == "1\n00:00:03,000 --> 00:00:04,000\nHello\n\n");

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "mentions",
                  "--code",
                  "Latn",
                  "--enable",
                  "Sound in brackets",
                  "--keep-blank-subtitles",
                  "--output",
                  kept,
                  in})
              .exitCode == 0);
    CHECK(contentOf(kept) == srt({"", "Hello"}));
}

TEST_CASE("a dry run lists the texts before and after, and a removal has no after",
          "[e2e][CLI-CORRECT-03]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"[Door]", "eﬀet"}));

    const CliRun run = invoke({"correct",
                               "--tasks",
                               "mentions,common-errors",
                               "--code",
                               "Latn",
                               "--enable",
                               "Sound in brackets",
                               "--dry-run",
                               in});

    CHECK(run.exitCode == 0);
    // The whole of what it proposes, byte for byte, and nothing else on standard output.
    std::string proposed = run.output;
    for (std::size_t at = proposed.find(scratch.path()); at != std::string::npos;
         at = proposed.find(scratch.path(), at + 9)) {
        proposed.replace(at, scratch.path().size(), "<scratch>");
    }
    CHECK_THAT(proposed, MatchesFile(corpus("attendus/correct/dry-run.txt")));
    CHECK_THAT(run.errors,
               ContainsSubstring("Edited 1 and removed 1 subtitles (dry run, nothing written)"));
    // And the input is as it was: nothing was written over it.
    CHECK(contentOf(in) == srt({"[Door]", "eﬀet"}));
}

TEST_CASE("a translation a mention would empty stays, empty, and only it is written",
          "[e2e][CLI-CORRECT-12]") {
    // The translation of `paires/temoin`, with the mentions of
    // `translation_test.cpp`: the same input, the same rule, the same expected file.
    const Scratch scratch;
    const std::string translation =
        writeFile(scratch,
                  "traduction.srt",
                  "1\n00:00:01,000 --> 00:00:03,000\n[Un oiseau chante]\n\n"
                  "2\n00:00:04,000 --> 00:00:06,000\nRien ne bouge [il tousse] sur l'eau.\n\n"
                  "3\n00:00:08,000 --> 00:00:11,000\nVoir [1] la note.\n\n"
                  "4\n00:00:12,000 --> 00:00:15,000\nPuis la lumière vient.\n\n");
    const std::string main = corpus("paires/temoin/principal.srt");
    const std::string out = scratch.of("nettoyee.srt");

    const CliRun run = invoke({"--quiet",
                               "correct",
                               "--tasks",
                               "mentions",
                               "--code",
                               "Latn-fr",
                               "--enable",
                               "Sound in brackets",
                               "--document",
                               "translation",
                               "-t",
                               translation,
                               "--output",
                               out,
                               main});

    CHECK(run.exitCode == 0);
    CHECK_THAT(
        contentOf(out),
        MatchesFile(corpus("attendus/translation/mentions.hearing-impaired.traduction.srt")));
}

TEST_CASE("correct reads no setting of the user", "[e2e][CLI-CORRECT-13]") {
    // The window's activations would switch « Ellipses » on, and a script would
    // then depend on what its author once ticked. Written here as the window
    // writes it, and not read.
    const std::filesystem::path settings =
        std::filesystem::path{configHome()} / "subedit" / "settings.conf";
    std::filesystem::create_directories(settings.parent_path());
    std::ofstream{settings} << "correction.activations = common-error:Zyyy:Ellipses:true\n";

    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"Attends…"}));
    const std::string untouched = scratch.of("untouched.srt");
    const std::string asked = scratch.of("asked.srt");

    const CliRun first = invoke({"--quiet",
                                 "correct",
                                 "--tasks",
                                 "common-errors",
                                 "--code",
                                 "Zyyy",
                                 "--output",
                                 untouched,
                                 in});
    const CliRun second = invoke({"--quiet",
                                  "correct",
                                  "--tasks",
                                  "common-errors",
                                  "--code",
                                  "Zyyy",
                                  "--enable",
                                  "Ellipses",
                                  "--output",
                                  asked,
                                  in});
    const std::string kept = contentOf(settings.string());
    std::error_code ignored;
    std::filesystem::remove_all(settings.parent_path(), ignored);

    CHECK(first.exitCode == 0);
    CHECK(contentOf(untouched) == srt({"Attends…"}));
    CHECK(second.exitCode == 0);
    CHECK(contentOf(asked) == srt({"Attends..."}));
    // And it wrote nothing back.
    CHECK(kept == "correction.activations = common-error:Zyyy:Ellipses:true\n");
}

TEST_CASE("line-break needs a length in characters, and says why it has no default",
          "[e2e][CLI-CORRECT-09]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"A text"}));
    const std::string out = scratch.of("o.srt");

    const CliRun missing =
        invoke({"correct", "--tasks", "line-break", "--code", "Latn-en", "--output", out, in});
    CHECK(missing.exitCode == 1);
    CHECK_THAT(missing.errors,
               ContainsSubstring("--max-length is required by the task line-break"));
    CHECK_THAT(missing.errors, ContainsSubstring("24 is a width in ems"));
    CHECK_FALSE(std::filesystem::exists(out));

    for (const char* bad : {"wide", "0", "-3", "12 "}) {
        INFO(bad);
        const CliRun run = invoke({"correct",
                                   "--tasks",
                                   "line-break",
                                   "--code",
                                   "Latn-en",
                                   "--max-length",
                                   bad,
                                   "--output",
                                   out,
                                   in});
        CHECK(run.exitCode == 1);
        CHECK_THAT(run.errors, ContainsSubstring("--max-length: "));
    }

    const CliRun lines = invoke({"correct",
                                 "--tasks",
                                 "line-break",
                                 "--code",
                                 "Latn-en",
                                 "--max-length",
                                 "20",
                                 "--max-lines",
                                 "2.5",
                                 "--output",
                                 out,
                                 in});
    CHECK(lines.exitCode == 1);
    CHECK_THAT(lines.errors, ContainsSubstring("--max-lines: \"2.5\" is not a number of lines"));

    const CliRun skip = invoke({"correct",
                                "--tasks",
                                "line-break",
                                "--code",
                                "Latn-en",
                                "--max-length",
                                "20",
                                "--skip-length",
                                "soon",
                                "--output",
                                out,
                                in});
    CHECK(skip.exitCode == 1);
    CHECK_THAT(skip.errors, ContainsSubstring("--skip-length: \"soon\" is not a bound"));

    // The four options are the line break's, and nothing else's.
    const CliRun stray = invoke({"correct",
                                 "--tasks",
                                 "common-errors",
                                 "--code",
                                 "Latn",
                                 "--max-length",
                                 "20",
                                 "--output",
                                 out,
                                 in});
    CHECK(stray.exitCode == 1);
    CHECK_THAT(
        stray.errors,
        ContainsSubstring("--max-length is for the task line-break, which was not asked for"));
}

TEST_CASE("a subtitle within the bounds of the skip is left as it is, and off forces it",
          "[e2e][CLI-CORRECT-09]") {
    // Two lines, each of nine and thirty-eight characters: within 40 and 2.
    const std::string text = "The night\nwas cold and the road was long, so long";
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({text}));
    const std::string skipped = scratch.of("skipped.srt");
    const std::string forced = scratch.of("forced.srt");

    // By default the bounds of the skip are those of the break itself.
    const CliRun run = invoke({"correct",
                               "--tasks",
                               "line-break",
                               "--code",
                               "Zyyy",
                               "--max-length",
                               "40",
                               "--max-lines",
                               "2",
                               "--output",
                               skipped,
                               in});
    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("Edited 0 and removed 0 subtitles"));
    CHECK(contentOf(skipped) == srt({text}));

    // `off` puts the gate out: the text is read as one, and cut again.
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "line-break",
                  "--code",
                  "Zyyy",
                  "--max-length",
                  "40",
                  "--max-lines",
                  "2",
                  "--skip-length",
                  "off",
                  "--skip-lines",
                  "off",
                  "--output",
                  forced,
                  in})
              .exitCode == 0);
    CHECK(contentOf(forced) == srt({"The night was cold and\nthe road was long, so long"}));

    // A bound of its own: 30 is below the thirty-eight characters of the second line.
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "line-break",
                  "--code",
                  "Zyyy",
                  "--max-length",
                  "40",
                  "--max-lines",
                  "2",
                  "--skip-length",
                  "30",
                  "--output",
                  forced,
                  in})
              .exitCode == 0);
    CHECK(contentOf(forced) == srt({"The night was cold and\nthe road was long, so long"}));
}

TEST_CASE("the line break is made in characters, and its lines default to Gaupol's three",
          "[e2e][CLI-CORRECT-09]") {
    const Scratch scratch;
    const std::string in = writeFile(
        scratch,
        "a.srt",
        srt({"When the rain stopped, we walked home under the dark sky of the old town."}));
    const std::string out = scratch.of("o.srt");

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "line-break",
                  "--code",
                  "Latn-en",
                  "--max-length",
                  "24",
                  "--output",
                  out,
                  in})
              .exitCode == 0);
    // Three lines do not fit in twenty-four characters, so the break takes a fourth,
    // as Gaupol's own does: the lines are the three of its default, a measure and not a wall.
    CHECK(contentOf(out) ==
          srt({"When the rain\nstopped, we walked\nhome under the dark\nsky of the old town."}));
}

TEST_CASE("a word cut in two is joined by the dictionary, and only when one direction spells",
          "[e2e][CLI-CORRECT-10]") {
    // The dictionary is the test's own — `data/dictionnaires/zz.*`, six words —, and
    // the machine's is not asked: Enchant is pointed at the harness's directory.
    const TestDictionary dictionary;
    const Scratch scratch;
    // « hel lo » is « hello » and only that way round; « hello world » is already right.
    const std::string in = writeFile(scratch, "a.srt", srt({"hel lo world", "hello world"}));
    const std::string out = scratch.of("o.srt");

    const CliRun run =
        invoke({"correct", "--tasks", "join-words", "--language", "zz", "--output", out, in});

    INFO(run.errors << " — a machine whose Enchant has no Hunspell engine cannot open the "
                       "dictionary of the test");
    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("a.srt: Edited 1 and removed 0 subtitles -> "));
    CHECK(contentOf(out) == srt({"hello world", "hello world"}));
}

TEST_CASE(
    "a word run together is split by the dictionary, when exactly one suggestion is it spaced",
    "[e2e][CLI-CORRECT-10]") {
    const TestDictionary dictionary;
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"hellothere world"}));
    const std::string out = scratch.of("o.srt");

    const CliRun run =
        invoke({"correct", "--tasks", "split-words", "--language", "zz", "--output", out, in});

    INFO(run.errors);
    CHECK(run.exitCode == 0);
    CHECK(contentOf(out) == srt({"hello there world"}));
}

TEST_CASE("joining and splitting are two tasks, run in Gaupol's place among the others",
          "[e2e][CLI-CORRECT-10]") {
    const TestDictionary dictionary;
    const Scratch scratch;
    // The join first, then the split, then the capital: « hel lo » → « hello »,
    // « hellothere » → « hello there », and the first letter of the document.
    const std::string in = writeFile(scratch, "a.srt", srt({"hel lo world", "hellothere"}));
    const std::string out = scratch.of("o.srt");

    const CliRun run = invoke({"--quiet",
                               "correct",
                               "--tasks",
                               "capitalization,split-words,join-words",
                               "--language",
                               "zz",
                               "--code",
                               "Latn",
                               "--output",
                               out,
                               in});

    INFO(run.errors);
    CHECK(run.exitCode == 0);
    // The capital is the one of the first subtitle, which a task of patterns gives it.
    CHECK(contentOf(out) == srt({"Hello world", "hello there"}));
}

TEST_CASE("the replacement list of the window is neither read nor written",
          "[e2e][CLI-CORRECT-10]") {
    const TestDictionary dictionary;
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"hel lo"}));

    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "join-words,split-words",
                  "--language",
                  "zz",
                  "--output",
                  scratch.of("o.srt"),
                  in})
              .exitCode == 0);

    // No `spell-check/<language>.repl` anywhere, and the configuration home stays
    // empty. Enchant itself leaves its own empty personal files beside the
    // dictionary it was asked for: they are its business, in the directory the
    // harness gave it, and not the replacement list of the window.
    CHECK(std::filesystem::is_empty(configHome()));
    for (const std::string& home : {enchantHome(), subedit::e2e::dataHome()}) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator{home}) {
            INFO(entry.path().string());
            CHECK(entry.path().extension() != ".repl");
            CHECK(entry.path().filename() != "spell-check");
        }
    }
}

TEST_CASE("a language nobody has is refused, with the window's sentence, and nothing is written",
          "[e2e][CLI-CORRECT-11]") {
    // Not one of the dictionaries of the machine, whichever it has.
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"hel lo"}));
    const std::string out = scratch.of("o.srt");

    const CliRun run =
        invoke({"correct", "--tasks", "join-words", "--language", "qq_QQ", "--output", out, in});

    CHECK(run.exitCode == 1);
    CHECK(run.errors == "no dictionary for qq_QQ\n");
    CHECK(run.output.empty());
    CHECK_FALSE(std::filesystem::exists(out));

    // The tasks of patterns, which need no dictionary, are not asked for one.
    CHECK(invoke({"--quiet",
                  "correct",
                  "--tasks",
                  "common-errors",
                  "--code",
                  "Latn",
                  "--output",
                  out,
                  in})
              .exitCode == 0);
}

TEST_CASE("a language is needed by the tasks that check words, and by no other",
          "[e2e][CLI-CORRECT-11]") {
    const Scratch scratch;
    const std::string in = writeFile(scratch, "a.srt", srt({"hel lo"}));
    const std::string out = scratch.of("o.srt");

    const CliRun missing = invoke({"correct", "--tasks", "split-words", "--output", out, in});
    CHECK(missing.exitCode == 1);
    CHECK_THAT(
        missing.errors,
        ContainsSubstring("--language is required by the tasks that check words: split-words"));

    const CliRun malformed =
        invoke({"correct", "--tasks", "join-words", "--language", "French", "--output", out, in});
    CHECK(malformed.exitCode == 1);
    CHECK_THAT(malformed.errors, ContainsSubstring("--language: \"French\" is not a language"));

    const CliRun stray = invoke({"correct",
                                 "--tasks",
                                 "common-errors",
                                 "--code",
                                 "Latn",
                                 "--language",
                                 "zz",
                                 "--output",
                                 out,
                                 in});
    CHECK(stray.exitCode == 1);
    CHECK_THAT(stray.errors,
               ContainsSubstring("--language is for the tasks join-words and split-words"));

    // A code, with tasks that read no pattern, is a mistake as well.
    const CliRun code = invoke({"correct",
                                "--tasks",
                                "join-words",
                                "--language",
                                "zz",
                                "--code",
                                "Latn",
                                "--output",
                                out,
                                in});
    CHECK(code.exitCode == 1);
    CHECK_THAT(
        code.errors,
        ContainsSubstring("--code is for the tasks that read patterns, and none was asked for"));
    CHECK_FALSE(std::filesystem::exists(out));
}
