#include <subedit/cli/batch.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <sstream>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::arrange;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::outcomeOf;
using subedit::cli::Reporter;
using subedit::cli::summaryOf;
using subedit::cli::tally;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;

TEST_CASE("everything succeeding is success", "[cli][batch]") {
    CHECK(outcomeOf(3, 3) == ExitCode::Success);
    CHECK(outcomeOf(0, 0) == ExitCode::Success);
}

TEST_CASE("nothing surviving is told apart from something surviving", "[cli][batch]") {
    // The two are distinct so that a script can act on "nothing worked" and on
    // "one is missing" without reading the output back.
    CHECK(outcomeOf(0, 3) == ExitCode::AllFailed);
    CHECK(outcomeOf(2, 3) == ExitCode::SomeFailed);
}

TEST_CASE("a single file gets no summary", "[cli][batch]") {
    // "1 of 1 files inspected" would repeat the line just above it.
    CHECK(summaryOf("inspected", 1, 1).empty());
    CHECK(summaryOf("inspected", 0, 1).empty());
}

TEST_CASE("the summary counts what succeeded", "[cli][batch]") {
    CHECK(summaryOf("inspected", 2, 2) == "2 of 2 files inspected");
}

TEST_CASE("the summary counts the failures when there are any", "[cli][batch]") {
    CHECK(summaryOf("inspected", 1, 3) == "1 of 3 files inspected, 2 failed");
}

TEST_CASE("the tally writes the summary and returns the code", "[cli][batch]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 1};

    CHECK(tally(reporter, "inspected", 1, 2) == ExitCode::SomeFailed);
    CHECK(errors.str() == "1 of 2 files inspected, 1 failed\n");
}

TEST_CASE("the tally says nothing under silence", "[cli][batch]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 0};

    CHECK(tally(reporter, "inspected", 2, 2) == ExitCode::Success);
    CHECK(errors.str().empty());
}

TEST_CASE("the exit codes are the ones a script reads", "[cli][batch]") {
    // Written out rather than derived: these four numbers are the contract with
    // every caller, and a renumbering must break a test rather than a script.
    CHECK(subedit::cli::toInt(ExitCode::Success) == 0);
    CHECK(subedit::cli::toInt(ExitCode::Usage) == 1);
    CHECK(subedit::cli::toInt(ExitCode::AllFailed) == 2);
    CHECK(subedit::cli::toInt(ExitCode::SomeFailed) == 3);
}

TEST_CASE("arranging a batch gives the jobs and makes each directory once",
          "[cli][batch][CLI-BATCH-05]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    const Reporter reporter{errors, 1};
    const Destination destination = Destination::from("", "out/deep", false, 2).value();

    const auto jobs = arrange(files, destination, {"a/one.srt", "b/two.srt"}, "", reporter);

    REQUIRE(jobs.has_value());
    CHECK(jobs->size() == 2);
    CHECK(files.directoriesAsked() == std::vector<std::filesystem::path>{"out/deep"});
    CHECK(errors.str().empty());
}

TEST_CASE("arranging an in-place batch makes no directory", "[cli][batch][CLI-BATCH-05]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    const Reporter reporter{errors, 1};
    const Destination destination = Destination::from("", "", true, 1).value();

    CHECK(arrange(files, destination, {"in/a.srt"}, "", reporter).has_value());
    CHECK(files.directoriesAsked().empty());
}

TEST_CASE("a refused plan is a usage error, said even when quiet, and makes no directory",
          "[cli][batch][CLI-BATCH-03]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    const Reporter reporter{errors, 0};
    const Destination destination = Destination::from("", "out", false, 2).value();

    const auto jobs = arrange(files, destination, {"a/film.srt", "b/film.srt"}, "", reporter);

    REQUIRE_FALSE(jobs.has_value());
    CHECK(jobs.error() == ExitCode::Usage);
    CHECK_THAT(errors.str(), ContainsSubstring("would be written by both"));
    CHECK(files.directoriesAsked().empty());
}

TEST_CASE("a directory that cannot be made stops the batch with code 2, said once",
          "[cli][batch][CLI-BATCH-05]") {
    InMemoryFileSystem files;
    files.failNextCreateDirectories(FileErrorKind::PermissionDenied);
    std::ostringstream errors;
    const Reporter reporter{errors, 1};
    const Destination destination = Destination::from("", "out", false, 2).value();

    const auto jobs = arrange(files, destination, {"a/one.srt", "b/two.srt"}, "", reporter);

    REQUIRE_FALSE(jobs.has_value());
    CHECK(jobs.error() == ExitCode::AllFailed);
    CHECK(errors.str() == "out: cannot be created: permission denied\n");
}
