// The helpers that generate a batch of files for the cases that write.
//
// They carry a little logic (paths, a counter, a copied file), and a harness
// nobody checks is one that stops working quietly.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

#include "cli_run.hpp"

using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::Scratch;
using subedit::e2e::srtText;
using subedit::e2e::writeFile;
using subedit::e2e::writeSrt;
using subedit::e2e::writeSrtBatch;
using subedit::e2e::writeUnreadable;

TEST_CASE("a generated file is a numbered SubRip text tagged with its name", "[e2e][harness]") {
    CHECK(srtText("t", 2) == "1\n00:00:01,000 --> 00:00:01,500\nt 1\n\n"
                             "2\n00:00:02,000 --> 00:00:02,500\nt 2\n\n");
    CHECK(srtText("t", 0).empty());
}

TEST_CASE("two generated files of the same base name differ by content", "[e2e][harness]") {
    const Scratch scratch;
    const std::string first = writeSrt(scratch, "a/film.srt");
    const std::string second = writeSrt(scratch, "b/film.srt");

    CHECK(first != second);
    CHECK(std::filesystem::path{first}.filename() == std::filesystem::path{second}.filename());
    CHECK(contentOf(first) != contentOf(second));
    CHECK(contentOf(first) == srtText("a/film.srt", 2));
}

TEST_CASE("a generated batch is numbered and in order", "[e2e][harness]") {
    const Scratch scratch;
    const auto paths = writeSrtBatch(scratch, 3);

    REQUIRE(paths.size() == 3);
    CHECK(paths[0] == scratch.of("in/file-1.srt"));
    CHECK(paths[2] == scratch.of("in/file-3.srt"));
    for (const std::string& path : paths)
        CHECK(std::filesystem::exists(path));
}

TEST_CASE("an unreadable file is a copy of the versioned empty one", "[e2e][harness]") {
    const Scratch scratch;
    const std::string path = writeUnreadable(scratch, "in/broken.srt");

    CHECK(contentOf(path) == contentOf(corpus("malformes/vide.srt")));
}

TEST_CASE("a written file keeps its bytes and gets its directories", "[e2e][harness]") {
    const Scratch scratch;
    const std::string path = writeFile(scratch, "x/y/z.txt", "a\r\nb");

    CHECK(contentOf(path) == "a\r\nb");
}
