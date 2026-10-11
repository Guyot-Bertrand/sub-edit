// Joining the repeated subtitles of a batch, and the chain that goes on from
// there, on an in-memory file system.

#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/joining.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::convertAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::joinAll;
using subedit::cli::repairAll;
using subedit::cli::Reporter;
using subedit::core::DisjointSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::SubtitleFormat;

namespace {

/// One text in three pieces that follow each other, and another over the whole
/// range, out of order.
const std::string kRepeats = "1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n"
                             "2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n"
                             "3\n00:00:01,000 --> 00:00:06,000\nBeta\n\n"
                             "4\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n";

[[nodiscard]] Destination into(const char* directory) {
    return Destination::from("", directory, false, 1).value();
}

} // namespace

TEST_CASE("repeated subtitles are joined, and the count says how many",
          "[cli][joining][CLI-JOIN-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kRepeats);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code =
        joinAll(files,
                {"a.srt"},
                std::nullopt,
                into("out"),
                Reporter{errors, 1}.withRecords(records).forCommand("join-repeats"));

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:06,000\nAlpha\n\n2\n00:00:01,000 --> 00:00:06,000\nBeta\n\n");
    CHECK_THAT(errors.str(), ContainsSubstring("2 repeated subtitles joined"));
    CHECK_THAT(records.str(), ContainsSubstring("\"joined\":2"));
    CHECK_THAT(records.str(), ContainsSubstring("\"chains\":1"));
}

TEST_CASE("a file with no repeat is written as it was and says so", "[cli][joining][CLI-JOIN-01]") {
    InMemoryFileSystem files;
    const std::string clean = "1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n";
    files.addFile("a.srt", clean);
    std::ostringstream errors;

    const ExitCode code = joinAll(files, {"a.srt"}, std::nullopt, into("out"), Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == clean);
    CHECK_THAT(errors.str(), ContainsSubstring("no repeated subtitles"));
}

TEST_CASE("repair joins, sorts and makes the ranges disjoint, in that order",
          "[cli][joining][CLI-JOIN-02]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kRepeats);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = repairAll(files,
                                    {"a.srt"},
                                    std::nullopt,
                                    DisjointSettings{},
                                    into("out"),
                                    Reporter{errors, 1}.withRecords(records).forCommand("repair"));

    CHECK(code == ExitCode::Success);
    // Joined first: the two texts then share one range, and are one subtitle.
    CHECK(files.contentOf("out/a.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:06,000\nAlpha\nBeta\n\n");
    CHECK_THAT(errors.str(), ContainsSubstring("2 repeated subtitles joined; already in order; "));
    CHECK_THAT(records.str(), ContainsSubstring("\"joined\":2"));
    CHECK_THAT(records.str(), ContainsSubstring("\"moved\":0"));
    CHECK_THAT(records.str(), ContainsSubstring("\"merged\":1"));
}

TEST_CASE("repair sorts a file that is out of order before it cuts",
          "[cli][joining][CLI-JOIN-02]") {
    InMemoryFileSystem files;
    files.addFile("a.srt",
                  "1\n00:00:03,000 --> 00:00:08,000\nB\n\n"
                  "2\n00:00:00,000 --> 00:00:05,000\nA\n\n");
    std::ostringstream errors;

    const ExitCode code = repairAll(
        files, {"a.srt"}, std::nullopt, DisjointSettings{}, into("out"), Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(files.contentOf("out/a.srt").value_or(""),
               ContainsSubstring("00:00:03,000 --> 00:00:05,000\nA\nB\n"));
    CHECK_THAT(errors.str(), ContainsSubstring("no repeated subtitles; 2 subtitles moved; "));
}

TEST_CASE("the join option on convert joins the repeats before writing, and counts them",
          "[cli][conversion][CLI-JOIN-03]") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", kRepeats);
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
                                     false,
                                     true);

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:06,000\nAlpha\n\n2\n00:00:01,000 --> 00:00:06,000\nBeta\n\n");
    CHECK_THAT(errors.str(), ContainsSubstring("a.srt: 2 repeated subtitles joined"));
    CHECK_THAT(records.str(), ContainsSubstring("\"joined\":2"));
}

TEST_CASE("the three options together are what repair does, on convert",
          "[cli][conversion][CLI-JOIN-03]") {
    InMemoryFileSystem files;
    files.addFile("in/a.srt", kRepeats);
    std::ostringstream errors;

    const ExitCode code = convertAll(files,
                                     {"in/a.srt"},
                                     subedit::core::ReadingChoices{},
                                     SubtitleFormat::SubRip,
                                     {},
                                     into("out"),
                                     Reporter{errors, 1},
                                     true,
                                     true,
                                     true);

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:06,000\nAlpha\nBeta\n\n");
    // In the order they are made: the repeats, the sort, the cut.
    const std::string said = errors.str();
    CHECK(said.find("repeated subtitles joined") < said.find("already in order"));
    CHECK(said.find("already in order") < said.find("subtitles split"));
}
