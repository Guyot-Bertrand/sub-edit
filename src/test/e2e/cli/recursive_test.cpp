// Directories as inputs: what a walk takes, in which order, and where it writes.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <filesystem>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;
using subedit::e2e::srtText;
using subedit::e2e::writeFile;
using subedit::e2e::writeSrt;

namespace {

/// A tree of depth 3, built to hold everything a walk must tell apart: the same
/// name in several branches, a hidden file and a hidden directory, a file of no
/// known format, a text file, and two links.
void writeTree(const Scratch& scratch) {
    writeSrt(scratch, "films/x.srt", 1);
    writeSrt(scratch, "films/a/x.srt", 1);
    writeSrt(scratch, "films/b/x.srt", 1);
    writeSrt(scratch, "films/a/deep/er/x.srt", 1);
    writeSrt(scratch, "films/.hidden.srt", 1);
    writeSrt(scratch, "films/.cache/x.srt", 1);
    writeFile(scratch, "films/README.txt", srtText("readme", 1));
    writeFile(scratch, "films/a/notes.dat", srtText("notes", 1));
    std::filesystem::create_directory_symlink(scratch.of("films/a"), scratch.of("films/link-dir"));
    std::filesystem::create_symlink(scratch.of("films/x.srt"), scratch.of("films/link-file.srt"));
}

[[nodiscard]] std::size_t countOf(const std::string& text, const std::string& what) {
    std::size_t count = 0;
    for (std::size_t at = text.find(what); at != std::string::npos; at = text.find(what, at + 1))
        ++count;
    return count;
}

} // namespace

TEST_CASE("a directory without --recursive is a usage error, and nothing is written",
          "[e2e][CLI-BATCH-08]") {
    const Scratch scratch;
    writeTree(scratch);

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), scratch.of("films")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors,
               ContainsSubstring(scratch.of("films") + ": is a directory: use --recursive"));
    CHECK(!std::filesystem::exists(scratch.of("out")));

    // inspect reads a tree as readily as the others write one, and refuses alike.
    CHECK(invoke({"inspect", scratch.of("films")}).exitCode == 1);
}

TEST_CASE("a walk keeps the files of known formats, and only those", "[e2e][CLI-BATCH-09]") {
    const Scratch scratch;
    writeTree(scratch);

    const CliRun run = invoke({"shift",
                               "--by",
                               "1",
                               "--recursive",
                               "--output-dir",
                               scratch.of("out"),
                               scratch.of("films")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("4 of 4 files shifted\n"));
    // Not taken: hidden file and directory, text file, unknown extension, links.
    for (const char* absent : {".hidden.srt", ".cache", "README.txt", "notes.dat", "link"})
        CHECK(!std::filesystem::exists(scratch.of(std::string{"out/"} + absent)));
    CHECK(!std::filesystem::exists(scratch.of("out/a/notes.dat")));
    CHECK(!std::filesystem::exists(scratch.of("out/link-dir")));
    CHECK(!std::filesystem::exists(scratch.of("out/link-file.srt")));
}

TEST_CASE("what the walk left aside is counted, and said at level 2", "[e2e][CLI-BATCH-09]") {
    const Scratch scratch;
    writeTree(scratch);

    const CliRun quiet = invoke({"inspect", "-r", scratch.of("films")});
    const CliRun detailed = invoke({"-vv", "inspect", "-r", scratch.of("films")});

    CHECK_THAT(quiet.errors, !ContainsSubstring("left aside"));
    // README.txt and a/notes.dat.
    CHECK_THAT(detailed.errors,
               ContainsSubstring(scratch.of("films") +
                                 ": 2 files left aside, with no extension of a known format"));
}

TEST_CASE("the walk is in the order of the names, bytes compared, depth first",
          "[e2e][CLI-BATCH-09]") {
    const Scratch scratch;
    writeSrt(scratch, "films/b.srt", 1);
    writeSrt(scratch, "films/a.srt", 1);
    writeSrt(scratch, "films/a/z.srt", 1);
    writeSrt(scratch, "films/B.srt", 1);

    const CliRun run = invoke({"inspect", "-r", scratch.of("films")});

    // Uppercase before lowercase; the directory `a` where its name stands,
    // before `a.srt` — its name is a prefix of the other, and the shorter is first.
    const std::string base = scratch.of("films") + "/";
    const std::size_t upper = run.output.find(base + "B.srt");
    const std::size_t inner = run.output.find(base + "a/z.srt");
    const std::size_t flat = run.output.find(base + "a.srt");
    const std::size_t lower = run.output.find(base + "b.srt");
    REQUIRE(upper != std::string::npos);
    REQUIRE(inner != std::string::npos);
    REQUIRE(flat != std::string::npos);
    REQUIRE(lower != std::string::npos);
    CHECK(upper < inner);
    CHECK(inner < flat);
    CHECK(flat < lower);
}

TEST_CASE("two runs of a walk give the same bytes", "[e2e][CLI-BATCH-09]") {
    const Scratch scratch;
    writeTree(scratch);

    const CliRun first = invoke({"-vv", "inspect", "-r", scratch.of("films")});
    const CliRun second = invoke({"-vv", "inspect", "-r", scratch.of("films")});

    CHECK(first.exitCode == 0);
    CHECK(first.output == second.output);
    CHECK(first.errors == second.errors);
}

TEST_CASE("the tree below the given directory is kept under --output-dir", "[e2e][CLI-BATCH-10]") {
    const Scratch scratch;
    writeTree(scratch);

    const CliRun run = invoke(
        {"shift", "-r", "--by", "1", "--output-dir", scratch.of("out"), scratch.of("films")});

    CHECK(run.exitCode == 0);
    // The name of the given directory is not part of the path: `rsync`, not `cp -r`.
    for (const char* kept : {"x.srt", "a/x.srt", "b/x.srt", "a/deep/er/x.srt"})
        CHECK(std::filesystem::exists(scratch.of(std::string{"out/"} + kept)));
    CHECK(!std::filesystem::exists(scratch.of("out/films")));
    // Same names in several branches, and none overwrote another.
    CHECK(contentOf(scratch.of("out/a/x.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\n" + std::string{"films/a/x.srt"} + " 1\n\n");
    CHECK(contentOf(scratch.of("out/b/x.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\n" + std::string{"films/b/x.srt"} + " 1\n\n");
}

TEST_CASE("a conversion over a tree changes the extension and keeps the tree",
          "[e2e][CLI-BATCH-10]") {
    const Scratch scratch;
    writeSrt(scratch, "films/a/x.srt", 1);

    const CliRun run = invoke(
        {"convert", "--to", "vtt", "-r", "--output-dir", scratch.of("out"), scratch.of("films")});

    CHECK(run.exitCode == 0);
    CHECK(std::filesystem::exists(scratch.of("out/a/x.vtt")));
}

TEST_CASE("with two directories, the collision of a destination decides", "[e2e][CLI-BATCH-10]") {
    const Scratch scratch;
    writeSrt(scratch, "one/a/x.srt", 1);
    writeSrt(scratch, "two/a/x.srt", 1);
    writeSrt(scratch, "two/a/y.srt", 1);

    const CliRun run = invoke({"shift",
                               "-r",
                               "--by",
                               "1",
                               "--output-dir",
                               scratch.of("out"),
                               scratch.of("one"),
                               scratch.of("two")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors,
               ContainsSubstring(scratch.of("out/a/x.srt") + ": would be written by both " +
                                 scratch.of("one/a/x.srt") + " and " + scratch.of("two/a/x.srt")));
    CHECK(!std::filesystem::exists(scratch.of("out")));
}

TEST_CASE("an output directory inside the tree is not walked, so a second run reads no output",
          "[e2e][CLI-BATCH-11]") {
    const Scratch scratch;
    writeSrt(scratch, "films/x.srt", 1);
    writeSrt(scratch, "films/a/y.srt", 1);
    const std::string out = scratch.of("films/out");

    const CliRun first =
        invoke({"shift", "-r", "--by", "1", "--output-dir", out, scratch.of("films")});
    const CliRun second =
        invoke({"shift", "-r", "--by", "1", "--output-dir", out, scratch.of("films")});

    CHECK(first.exitCode == 0);
    CHECK_THAT(first.errors, ContainsSubstring("2 of 2 files shifted\n"));
    // Not 4: what the first run wrote was not read by the second.
    CHECK(second.exitCode == 0);
    CHECK_THAT(second.errors, ContainsSubstring("2 of 2 files shifted\n"));
    CHECK(!std::filesystem::exists(scratch.of("films/out/out")));
    CHECK(countOf(second.errors, "films/out/") == 2);
}

TEST_CASE("writing in place over a walk rewrites each file where it stands",
          "[e2e][CLI-BATCH-09]") {
    const Scratch scratch;
    writeSrt(scratch, "films/x.srt", 1);
    writeSrt(scratch, "films/a/y.srt", 1);

    const CliRun run = invoke({"shift", "-r", "--by", "1", "--in-place", scratch.of("films")});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(scratch.of("films/a/y.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\nfilms/a/y.srt 1\n\n");
}

TEST_CASE("a directory holding nothing to walk says so, and is not an error",
          "[e2e][CLI-BATCH-09]") {
    const Scratch scratch;
    writeFile(scratch, "films/README.txt", "nothing");

    const CliRun run = invoke({"inspect", "-r", scratch.of("films")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("holds no file in a format this tool walks for"));
}
