#include <subedit/cli/reporter.hpp>
#include <subedit/cli/wording.hpp>

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
    CHECK(subedit::cli::countOf(0, "subtitle") == "0 subtitles");
    CHECK(subedit::cli::countOf(1, "subtitle") == "1 subtitle");
    CHECK(subedit::cli::countOf(2, "subtitle") == "2 subtitles");
}
