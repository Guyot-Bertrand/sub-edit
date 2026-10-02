#include <subedit/cli/reporter.hpp>
#include <subedit/core/wording/counts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using subedit::cli::Reporter;

TEST_CASE("a line is written when the level reaches its own", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 2};

    reporter.say(1, "coarse");
    reporter.say(2, "finer");

    CHECK(errors.str() == "coarse\nfiner\n");
}

TEST_CASE("a line above the level is dropped", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 1};

    reporter.say(2, "detail");
    reporter.say(3, "debug");

    CHECK(errors.str().empty());
}

TEST_CASE("silence drops every narration", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 0};

    reporter.say(1, "what happened");

    CHECK(errors.str().empty());
}

TEST_CASE("silence keeps the failures", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 0};

    reporter.failed("file.srt: does not exist");

    // A command failing in silence leaves its exit code as the only clue, and
    // turns every incident into an investigation.
    CHECK(errors.str() == "file.srt: does not exist\n");
}

TEST_CASE("each level keeps every line of the one below", "[cli][reporter]") {
    const auto narrate = [](int level) {
        std::ostringstream errors;
        const Reporter reporter{errors, level};
        reporter.say(3, "debug");
        reporter.say(2, "detail");
        reporter.say(1, "outcome");
        return errors.str();
    };

    CHECK(narrate(1) == "outcome\n");
    CHECK(narrate(2) == "detail\noutcome\n");
    CHECK(narrate(3) == "debug\ndetail\noutcome\n");
}

TEST_CASE("the level is readable", "[cli][reporter]") {
    std::ostringstream errors;

    CHECK(Reporter{errors, 3}.level() == 3);
}

TEST_CASE("a count agrees with its noun", "[cli][wording]") {
    CHECK(subedit::core::countOf(0, "subtitle") == "0 subtitles");
    CHECK(subedit::core::countOf(1, "subtitle") == "1 subtitle");
    CHECK(subedit::core::countOf(2, "subtitle") == "2 subtitles");
}

TEST_CASE("a result goes to the text output, as it is", "[cli][reporter][CLI-DRYRUN-04]") {
    std::ostringstream errors;
    std::ostringstream out;
    const Reporter reporter = Reporter{errors, 1}.withTextOutput(out);

    reporter.result("a: subtitle 1\n- x\n");

    CHECK(out.str() == "a: subtitle 1\n- x\n");
    CHECK(errors.str().empty());
}

TEST_CASE("a reporter with nowhere to put a result drops it", "[cli][reporter][CLI-DRYRUN-04]") {
    std::ostringstream errors;

    Reporter{errors, 1}.result("text");

    CHECK(errors.str().empty());
}

TEST_CASE("the records are the result when they are wanted, and the text is not written",
          "[cli][reporter][CLI-DRYRUN-05]") {
    std::ostringstream errors;
    std::ostringstream out;
    const Reporter reporter = Reporter{errors, 1}.withTextOutput(out).withRecords(out);

    reporter.result("text\n");

    CHECK(out.str().empty());
}
