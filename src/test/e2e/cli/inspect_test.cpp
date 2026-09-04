// What `inspect` reports about a file, without touching it.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::corpus;
using subedit::e2e::invoke;

TEST_CASE("the help lists the subcommand", "[e2e][CLI-USAGE-01]") {
    const std::string help = invoke({}).output;

    // The listing, not the prose: the description sentence also contains the
    // word, and asserting on it would pass before the subcommand existed.
    CHECK_THAT(help, ContainsSubstring("Subcommands:"));
    CHECK_THAT(help, ContainsSubstring("\n  inspect"));
}

TEST_CASE("inspect reports what a file is made of", "[e2e][CLI-INSPECT-01]") {
    const CliRun run = invoke({"inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  format: SubRip\n"));
    CHECK_THAT(run.output, ContainsSubstring("  encoding: UTF-8, detected\n"));
    CHECK_THAT(run.output, ContainsSubstring("  byte order mark: absent\n"));
    CHECK_THAT(run.output, ContainsSubstring("  line endings: LF\n"));
    CHECK_THAT(run.output, ContainsSubstring("  subtitles: 2\n"));
    CHECK_THAT(run.output, ContainsSubstring("  span: 00:00:01.000 -> 00:00:06.200\n"));
}

TEST_CASE("inspect names the file it is reporting on", "[e2e][CLI-INSPECT-01]") {
    const std::string path = corpus("valides/minimal.srt");

    CHECK_THAT(invoke({"inspect", path}).output, ContainsSubstring(path + "\n"));
}

TEST_CASE("inspect sees a byte order mark and Windows line endings", "[e2e][CLI-INSPECT-01]") {
    const CliRun run = invoke({"inspect", corpus("valides/crlf-bom.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  byte order mark: present\n"));
    CHECK_THAT(run.output, ContainsSubstring("  line endings: CRLF\n"));
}

TEST_CASE("inspect recognises WebVTT", "[e2e][CLI-INSPECT-01]") {
    CHECK_THAT(invoke({"inspect", corpus("valides/minimal.vtt")}).output,
               ContainsSubstring("  format: WebVTT\n"));
}

TEST_CASE("inspect says a file has nothing wrong with it", "[e2e][CLI-INSPECT-01]") {
    CHECK_THAT(invoke({"inspect", corpus("valides/minimal.srt")}).output,
               ContainsSubstring("  anomalies: none\n"));
}

TEST_CASE("inspect signals mixed line endings with their line", "[e2e][CLI-INSPECT-02]") {
    const CliRun run = invoke({"inspect", corpus("malformes/fins-de-ligne-melangees.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  line endings: CRLF, mixed from line 5\n"));
}

TEST_CASE("a file that cannot be read is named, and nothing is reported on it",
          "[e2e][CLI-BATCH-01]") {
    const std::string path = corpus("malformes/vide.srt");
    const CliRun run = invoke({"inspect", path});

    // An empty file matches no format, and ADR 0008 makes that a failure
    // rather than an empty result: losing a file without noticing is the
    // outcome that rule exists to prevent.
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring(path));
}

TEST_CASE("a file no encoding decodes is refused, and the reason says so", "[e2e][CLI-ENC-06]") {
    const std::string path = corpus("malformes/bom-utf8-menteur.srt");
    const CliRun run = invoke({"inspect", path});

    // A mark that says UTF-8 over bytes that are not. The mark settles the
    // encoding — it is the only declaration a subtitle file carries — so no
    // other is tried, and the reading fails. The old wording said "is not valid
    // UTF-8", which read as "this file is broken"; with several encodings the
    // truth is narrower, and the sentence has to be too.
    CHECK(run.exitCode != 0);
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring(path));
    CHECK_THAT(run.errors, ContainsSubstring("cannot be decoded in the chosen encoding"));
}

TEST_CASE("a file that is not UTF-8 opens, and the encoding read is said", "[e2e][CLI-ENC-01]") {
    // Refused through seven phases. What the user sees now is the file, and the
    // encoding it was read in — the detection is a proposal, and one the user
    // can weigh only if it is written down.
    const CliRun run = invoke({"-vv", "inspect", corpus("encodages/koi8-r.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  subtitles: 3\n"));
    CHECK_THAT(run.errors, ContainsSubstring("SubRip, KOI8-R, no BOM, LF line endings"));
}

TEST_CASE("an encoding nobody declared is one of the diagnostics of the reading",
          "[e2e][CLI-ENC-01]") {
    // Where a reading says what it had to settle — the same list the window
    // shows under the table. It speaks about the whole file rather than about
    // one of its lines, and is the first diagnostic that does: no "line 0"
    // precedes it.
    const CliRun run = invoke({"-vvv", "inspect", corpus("encodages/cp1250.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors,
               ContainsSubstring("an encoding nothing declared (\"windows-1250\"), "
                                 "settled by the reader"));
}

TEST_CASE("inspect says the encoding, and where the answer came from", "[e2e][CLI-ENC-07]") {
    // Two answers, and the difference is what a reader needs: a mark is a
    // declaration, a detection is a proposal that Latin-1 and CP1252 can both
    // fit.
    CHECK_THAT(invoke({"inspect", corpus("encodages/cp1252.srt")}).output,
               ContainsSubstring("  encoding: windows-1252, detected\n"));
    CHECK_THAT(invoke({"inspect", corpus("encodages/utf-16-be-bom.srt")}).output,
               ContainsSubstring("  encoding: UTF-16BE, from its byte order mark\n"));
}

TEST_CASE("the encoding asked for is the one read, and the report says so", "[e2e][CLI-ENC-02]") {
    // Latin-1 and CP1252 are the same bytes on most texts, and this file is one
    // of them: the detection proposes the first, and the user may know better.
    // What they asked for is what is read, and the report says it was asked.
    const CliRun run = invoke({"--encoding", "cp1252", "inspect", corpus("encodages/latin1.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  encoding: windows-1252, as asked for\n"));
}

TEST_CASE("a name that is no encoding is refused before anything is read", "[e2e][CLI-ENC-02]") {
    const CliRun run =
        invoke({"--encoding", "klingon-1", "inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring("klingon-1"));
}

TEST_CASE("reading in an encoding that writes its own mark is refused too", "[e2e][CLI-ENC-02]") {
    // **One rule, at the model, and both surfaces meet it.** A converter that
    // writes a mark of its own has no place in this model, and the refusal is
    // not made at the door that happens to lead to writing: `--encoding UTF-16`
    // is answered here in the same words, before a file is read.
    const CliRun run = invoke({"--encoding", "UTF-16", "inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring("\"UTF-16\" writes a byte order mark of its own"));
}

TEST_CASE("a mark wins over the encoding asked for, and the gap is said", "[e2e][CLI-ENC-03]") {
    // The only declaration a subtitle file carries. It is obeyed against the
    // option — and the reading says so rather than settling it in silence.
    const CliRun run =
        invoke({"-vvv", "--encoding", "cp1252", "inspect", corpus("encodages/utf-16-le-bom.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  encoding: UTF-16LE, from its byte order mark\n"));
    CHECK_THAT(run.errors,
               ContainsSubstring("a byte order mark that contradicts the encoding asked for "
                                 "(\"UTF-16LE\")"));
}

TEST_CASE("inspect writes its report even when asked for silence", "[e2e][CLI-OUTPUT-02]") {
    const CliRun run = invoke({"--quiet", "inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    // The report is the result, not narration: silencing it would leave the
    // subcommand with nothing to say.
    CHECK_THAT(run.output, ContainsSubstring("  format: SubRip\n"));
    CHECK(run.errors.empty());
}

TEST_CASE("the report names the subtitles out of place", "[e2e][CLI-INSPECT-04]") {
    const CliRun run = invoke({"inspect", corpus("malformes/desordre.srt")});

    CHECK(run.exitCode == 0);
    // **By subtitle number, not by line** — ADR 0018. The overlap comes with the
    // disorder: a subtitle that starts before the previous one started also
    // starts before it ended, and the two are fixed differently.
    CHECK_THAT(run.output,
               ContainsSubstring("  anomalies: subtitle 2 starts before the previous one ends, "
                                 "subtitle 2 starts before the previous one starts\n"));
}

TEST_CASE("a sound file has no anomaly to report", "[e2e][CLI-INSPECT-04]") {
    const CliRun run = invoke({"inspect", corpus("valides/trois.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  anomalies: none\n"));
}

TEST_CASE("a value outside a closed set is refused", "[e2e][CLI-USAGE-02]") {
    // `inspect --order-report` used to carry this case; it went with the option.
    // The property is the command line's, not that option's, so it moved to the
    // nearest closed set rather than disappearing with its example.
    const CliRun run = invoke({"convert", "--to", "sideways", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    // The closed set is named, so that the caller learns what was expected
    // rather than only that they were wrong.
    CHECK_THAT(run.errors, ContainsSubstring("srt"));
    CHECK_THAT(run.errors, ContainsSubstring("vtt"));
}

TEST_CASE("inspect reports the grid a file was written on", "[e2e][CLI-INSPECT-05]") {
    const CliRun run = invoke({"inspect", corpus("grilles/grille-24.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: 24 fps, clean ("));
}

TEST_CASE("inspect reports a partial grid as partial", "[e2e][CLI-INSPECT-05]") {
    // Two thirds on a grid at 29.97, the last third retimed. Saying « clean »
    // or « none » would both be wrong, and the number of runs is what tells a
    // retimed section from positions corrected one by one.
    const CliRun run = invoke({"inspect", corpus("grilles/melange-groupe.srt")});

    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: 30000/1001 fps, partial ("));
    CHECK_THAT(run.output, ContainsSubstring("  off the grid: "));
    CHECK_THAT(run.output, ContainsSubstring(" runs\n"));
}

TEST_CASE("inspect reports the offset of a shifted grid", "[e2e][CLI-INSPECT-05]") {
    const CliRun run = invoke({"inspect", corpus("grilles/grille-24-decalee.srt")});

    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: 24 fps, clean ("));
    CHECK_THAT(run.output, ContainsSubstring("  grid offset: "));
}

TEST_CASE("a file on no grid at all says so, and names no rate", "[e2e][CLI-INSPECT-06]") {
    // 26.3 frames per second: perfectly regular, and none of the eight
    // candidates. Reporting the least wrong of them would be a bad answer
    // where « I do not know » is the right one — and it is that property which
    // makes the deduction usable at all.
    const CliRun run = invoke({"inspect", corpus("grilles/grille-absurde.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: none (best candidate at "));
    CHECK_THAT(run.output, !ContainsSubstring(" fps,"));
}

TEST_CASE("too few subtitles say so rather than nothing", "[e2e][CLI-INSPECT-06]") {
    // Two starts always look concentrated: the noise floor is one over the
    // square root of the count. The two causes of a silent verdict are not the
    // same answer, and the report distinguishes them.
    const CliRun run = invoke({"inspect", corpus("valides/minimal.srt")});

    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: none (too few subtitles"));
}

TEST_CASE("a harmonic ambiguity is named, and the lower rate kept", "[e2e][CLI-INSPECT-07]") {
    // A grid at 25 is included in a grid at 50, so a file written on 25 scores
    // full marks on both. The implication holds one way only, which is why the
    // lower rate is the one reported — and the other is said rather than hidden.
    const CliRun run = invoke({"inspect", corpus("grilles/grille-25.srt")});

    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: 25 fps, clean ("));
    CHECK_THAT(run.output, ContainsSubstring("  also fits: 50 fps"));
}

TEST_CASE("a file truly on the higher rate keeps it, with no ambiguity", "[e2e][CLI-INSPECT-07]") {
    const CliRun run = invoke({"inspect", corpus("grilles/grille-50.srt")});

    CHECK_THAT(run.output, ContainsSubstring("  frame rate grid: 50 fps, clean ("));
    CHECK_THAT(run.output, !ContainsSubstring("  also fits: "));
}

TEST_CASE("too short a span to separate two rates says which", "[e2e][CLI-INSPECT-05]") {
    // Ten seconds on a grid at 24. The candidate at 24000/1001 drifts by a
    // millisecond per second of film, so it is still at ninety-odd per cent —
    // answering « 24 » without saying so would be lying by omission.
    const CliRun run = invoke({"inspect", corpus("grilles/grille-24-courte.srt")});

    CHECK_THAT(run.output, ContainsSubstring("  too short a span to separate: 24000/1001 fps"));
}
