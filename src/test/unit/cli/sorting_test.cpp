// The sort of a batch, on an in-memory file system.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/sorting.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::sortAll;
using subedit::core::InMemoryFileSystem;

namespace {

const std::string kDisorder = "1\n00:00:05,000 --> 00:00:06,000\nlate\n\n"
                              "2\n00:00:01,000 --> 00:00:02,000\nearly\n\n";

} // namespace

TEST_CASE("a sort puts the earlier subtitle first and counts the places that changed",
          "[cli][sorting][CLI-SORT-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kDisorder);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = sortAll(files,
                                  {"a.srt"},
                                  std::nullopt,
                                  Destination::from("", "out", false, 1).value(),
                                  Reporter{errors, 1}.withRecords(records).forCommand("sort"));

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:02,000\nearly\n\n2\n00:00:05,000 --> 00:00:06,000\nlate\n\n");
    CHECK_THAT(errors.str(), ContainsSubstring("2 subtitles moved"));
    CHECK_THAT(records.str(), ContainsSubstring("\"counts\":{\"subtitles\":2,\"moved\":2}"));
}

TEST_CASE("a file in order is written as it was and says so", "[cli][sorting][CLI-SORT-02]") {
    InMemoryFileSystem files;
    const std::string ordered = "1\n00:00:01,000 --> 00:00:02,000\none\n\n";
    files.addFile("a.srt", ordered);
    std::ostringstream errors;

    const ExitCode code = sortAll(files,
                                  {"a.srt"},
                                  std::nullopt,
                                  Destination::from("", "out", false, 1).value(),
                                  Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == ordered);
    CHECK_THAT(errors.str(), ContainsSubstring("already in order"));
}
