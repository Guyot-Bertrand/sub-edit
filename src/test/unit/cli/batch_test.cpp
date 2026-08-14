#include <subedit/cli/batch.hpp>
#include <subedit/cli/reporter.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using subedit::cli::ExitCode;
using subedit::cli::outcomeOf;
using subedit::cli::Reporter;
using subedit::cli::summaryOf;
using subedit::cli::tally;

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
