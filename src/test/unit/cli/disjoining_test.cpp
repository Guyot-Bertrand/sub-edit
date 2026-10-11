// Making the ranges disjoint, on an in-memory file system: the command, its
// settings, and the `--disjoint` option of the commands that carry it.

#include <subedit/cli/appending.hpp>
#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/disjoining.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/splitting.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::appendAll;
using subedit::cli::convertAll;
using subedit::cli::Destination;
using subedit::cli::disjointAll;
using subedit::cli::DisjointOptions;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::settingsOf;
using subedit::cli::splitFile;
using subedit::cli::SplitRequest;
using subedit::core::DisjointSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::SubtitleFormat;

namespace {

/// A [0, 5] and B [3, 8], in order.
const std::string kOverlap = "1\n00:00:00,000 --> 00:00:05,000\nA\n\n"
                             "2\n00:00:03,000 --> 00:00:08,000\nB\n\n";

/// What cutting them gives.
const std::string kCut = "1\n00:00:00,000 --> 00:00:03,000\nA\n\n"
                         "2\n00:00:03,000 --> 00:00:05,000\nA\nB\n\n"
                         "3\n00:00:05,000 --> 00:00:08,000\nB\n\n";

/// The same two, the last one first.
const std::string kUnordered = "1\n00:00:03,000 --> 00:00:08,000\nB\n\n"
                               "2\n00:00:00,000 --> 00:00:05,000\nA\n\n";

[[nodiscard]] Destination into(const char* directory) {
    return Destination::from("", directory, false, 1).value();
}

} // namespace

TEST_CASE("the ranges are made disjoint, and the count says how",
          "[cli][disjoining][CLI-DISJOINT-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOverlap);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code =
        disjointAll(files,
                    {"a.srt"},
                    std::nullopt,
                    DisjointSettings{},
                    false,
                    into("out"),
                    Reporter{errors, 1}.withRecords(records).forCommand("disjoint"));

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == kCut);
    CHECK_THAT(errors.str(), ContainsSubstring("2 subtitles split, 1 merged"));
    CHECK_THAT(records.str(), ContainsSubstring("\"split\":2"));
    CHECK_THAT(records.str(), ContainsSubstring("\"merged\":1"));
    CHECK_THAT(records.str(), ContainsSubstring("\"subtitles\":3"));
}

TEST_CASE("a file that overlaps nothing is written as it was and says so",
          "[cli][disjoining][CLI-DISJOINT-01]") {
    InMemoryFileSystem files;
    const std::string clean = "1\n00:00:01,000 --> 00:00:02,000\nA\n\n";
    files.addFile("a.srt", clean);
    std::ostringstream errors;

    const ExitCode code = disjointAll(files,
                                      {"a.srt"},
                                      std::nullopt,
                                      DisjointSettings{},
                                      false,
                                      into("out"),
                                      Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == clean);
    CHECK_THAT(errors.str(), ContainsSubstring("already disjoint"));
}

TEST_CASE("a file out of order is refused unless the sort is asked for",
          "[cli][disjoining][CLI-DISJOINT-02]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kUnordered);

    SECTION("refused, and nothing is written") {
        std::ostringstream errors;
        std::ostringstream records;

        const ExitCode code =
            disjointAll(files,
                        {"a.srt"},
                        std::nullopt,
                        DisjointSettings{},
                        false,
                        into("out"),
                        Reporter{errors, 1}.withRecords(records).forCommand("disjoint"));

        CHECK(code != ExitCode::Success);
        CHECK_FALSE(files.contentOf("out/a.srt").has_value());
        CHECK_THAT(errors.str(), ContainsSubstring("sort them first"));
        CHECK_THAT(records.str(), ContainsSubstring("not-in-order"));
    }

    SECTION("sorted first, and the moves are counted") {
        std::ostringstream errors;
        std::ostringstream records;

        const ExitCode code =
            disjointAll(files,
                        {"a.srt"},
                        std::nullopt,
                        DisjointSettings{},
                        true,
                        into("out"),
                        Reporter{errors, 1}.withRecords(records).forCommand("disjoint"));

        CHECK(code == ExitCode::Success);
        CHECK(files.contentOf("out/a.srt").value_or("") == kCut);
        CHECK_THAT(records.str(), ContainsSubstring("\"moved\":2"));
    }
}

TEST_CASE("the settings are read from their options", "[cli][disjoining][CLI-DISJOINT-03]") {
    const auto none = settingsOf(DisjointOptions{});
    REQUIRE(none.has_value());
    CHECK(*none == DisjointSettings{});

    const auto all = settingsOf(DisjointOptions{.minimum = "0.2", .maxLines = "3", .dashes = true});
    REQUIRE(all.has_value());
    CHECK(all->shortest.milliseconds() == 200);
    CHECK(all->mostLines == 3);
    CHECK(all->dashes);

    const auto unlimited = settingsOf(DisjointOptions{.maxLines = "0"});
    REQUIRE(unlimited.has_value());
    CHECK(unlimited->mostLines == 0);
}

TEST_CASE("a setting that makes no sense is refused with the option's name",
          "[cli][disjoining][CLI-DISJOINT-03]") {
    const auto minimum = settingsOf(DisjointOptions{.minimum = "soon"});
    REQUIRE_FALSE(minimum.has_value());
    CHECK_THAT(minimum.error(), ContainsSubstring("--minimum"));

    const auto negative = settingsOf(DisjointOptions{.minimum = "-1"});
    REQUIRE_FALSE(negative.has_value());
    CHECK_THAT(negative.error(), ContainsSubstring("--minimum"));

    const auto lines = settingsOf(DisjointOptions{.maxLines = "two"});
    REQUIRE_FALSE(lines.has_value());
    CHECK_THAT(lines.error(), ContainsSubstring("--max-lines"));
}

TEST_CASE("the settings reach the operation: dashes, and a limit lifted",
          "[cli][disjoining][CLI-DISJOINT-03]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOverlap);
    std::ostringstream errors;

    const ExitCode code = disjointAll(files,
                                      {"a.srt"},
                                      std::nullopt,
                                      DisjointSettings{.mostLines = 0, .dashes = true},
                                      false,
                                      into("out"),
                                      Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(files.contentOf("out/a.srt").value_or(""), ContainsSubstring("- A\n- B"));
}

TEST_CASE("the disjoint option on convert cuts on a copy and counts it",
          "[cli][conversion][CLI-DISJOINT-04]") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", kOverlap);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = convertAll(files,
                                     {"in/a.srt"},
                                     subedit::core::ReadingChoices{},
                                     SubtitleFormat::SubRip,
                                     {},
                                     into("out"),
                                     Reporter{errors, 1}.withRecords(records).forCommand("convert"),
                                     false,
                                     true);

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == kCut);
    CHECK_THAT(errors.str(), ContainsSubstring("a.srt: 2 subtitles split, 1 merged"));
    CHECK_THAT(records.str(), ContainsSubstring("\"split\":2"));
}

TEST_CASE("the disjoint option on convert refuses a file out of order, and --sort puts it right",
          "[cli][conversion][CLI-DISJOINT-04]") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", kUnordered);

    std::ostringstream refusal;
    const ExitCode refused = convertAll(files,
                                        {"in/a.srt"},
                                        subedit::core::ReadingChoices{},
                                        SubtitleFormat::SubRip,
                                        {},
                                        into("out"),
                                        Reporter{refusal, 1},
                                        false,
                                        true);
    CHECK(refused != ExitCode::Success);
    CHECK_FALSE(files.contentOf("out/a.srt").has_value());
    CHECK_THAT(refusal.str(), ContainsSubstring("sort them first"));

    std::ostringstream errors;
    const ExitCode code = convertAll(files,
                                     {"in/a.srt"},
                                     subedit::core::ReadingChoices{},
                                     SubtitleFormat::SubRip,
                                     {},
                                     into("out"),
                                     Reporter{errors, 1},
                                     true,
                                     true);
    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == kCut);
}

TEST_CASE("the disjoint option on append cuts the whole once it is put together",
          "[cli][appending][CLI-DISJOINT-04]") {
    InMemoryFileSystem files;
    // The second file is shifted past the end of the first: nothing overlaps
    // between them, and the base overlaps itself.
    files.addFile("a.srt", kOverlap);
    files.addFile("b.srt", "1\n00:00:00,000 --> 00:00:01,000\nC\n\n");
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "b.srt"},
                                    std::nullopt,
                                    Destination::from("all.srt", "", false, 1).value(),
                                    Reporter{errors, 1}.withRecords(records).forCommand("append"),
                                    false,
                                    true);

    CHECK(code == ExitCode::Success);
    CHECK_THAT(files.contentOf("all.srt").value_or(""), ContainsSubstring("A\nB"));
    CHECK_THAT(errors.str(), ContainsSubstring("a.srt: 2 subtitles split, 1 merged"));
    CHECK_THAT(records.str(), ContainsSubstring("\"merged\":1"));
}

TEST_CASE("the disjoint option on append refuses what is out of order",
          "[cli][appending][CLI-DISJOINT-04]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kUnordered);
    files.addFile("b.srt", "1\n00:00:00,000 --> 00:00:01,000\nC\n\n");
    std::ostringstream errors;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "b.srt"},
                                    std::nullopt,
                                    Destination::from("all.srt", "", false, 1).value(),
                                    Reporter{errors, 1},
                                    false,
                                    true);

    CHECK(code != ExitCode::Success);
    CHECK_FALSE(files.contentOf("all.srt").has_value());
}

TEST_CASE("the disjoint option on split-file cuts before the cut is judged",
          "[cli][splitting][CLI-DISJOINT-04]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOverlap + "3\n00:00:09,000 --> 00:00:10,000\nC\n\n");
    std::ostringstream errors;
    std::ostringstream records;
    // Four subtitles once cut: the tail starts at the last.
    SplitRequest request{
        .input = "a.srt", .at = 4, .head = "out/h.srt", .tail = "out/t.srt", .dryRun = false};
    request.disjoint = true;

    const ExitCode code =
        splitFile(files,
                  request,
                  std::nullopt,
                  Reporter{errors, 1}.withRecords(records).forCommand("split-file"));

    CHECK(code == ExitCode::Success);
    CHECK_THAT(files.contentOf("out/h.srt").value_or(""), ContainsSubstring("A\nB"));
    CHECK_THAT(files.contentOf("out/t.srt").value_or(""), ContainsSubstring("C"));
    CHECK_THAT(errors.str(), ContainsSubstring("a.srt: 2 subtitles split, 1 merged"));
    CHECK_THAT(records.str(), ContainsSubstring("\"split\":2"));
}

TEST_CASE("the disjoint option on split-file refuses what is out of order",
          "[cli][splitting][CLI-DISJOINT-04]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kUnordered);
    std::ostringstream errors;
    SplitRequest request{
        .input = "a.srt", .at = 2, .head = "out/h.srt", .tail = "out/t.srt", .dryRun = false};
    request.disjoint = true;

    const ExitCode code = splitFile(files, request, std::nullopt, Reporter{errors, 1});

    CHECK(code != ExitCode::Success);
    CHECK_FALSE(files.contentOf("out/h.srt").has_value());
}
