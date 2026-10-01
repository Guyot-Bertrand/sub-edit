// What the expected-file comparison says when two texts differ.
//
// The message is the whole point of the helper: a test that only answered
// "not equal" would send its reader to a diff tool. So the cases below hold the
// text of the message, through the function that computes it — a `CHECK` that
// really fails would fail the run.

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string>

#include "cli_run.hpp"

using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::firstDifference;
using subedit::e2e::MatchesFile;
using subedit::e2e::Scratch;

namespace {

/// The message for two texts that must differ — an empty string when they do
/// not, which no message is, so that the comparison below fails with a plain
/// mismatch rather than reading an absent value.
std::string messageFor(const std::string& actual, const std::string& expected) {
    return firstDifference(actual, expected).value_or(std::string{});
}

} // namespace

TEST_CASE("equal texts have no difference", "[e2e][harness]") {
    CHECK(!firstDifference("a\nb\n", "a\nb\n").has_value());
    CHECK(!firstDifference("", "").has_value());
}

TEST_CASE("the first differing line is named, and only that one", "[e2e][harness]") {
    CHECK(messageFor("a\nB\nc\nD\n", "a\nb\nc\nd\n") ==
          "line 2 differs\n  expected: \"b\\n\"\n  actual:   \"B\\n\"");
}

TEST_CASE("a text that is the prefix of the other differs where it stops", "[e2e][harness]") {
    CHECK(messageFor("a\n", "a\nb\n") ==
          "line 2 differs\n  expected: \"b\\n\"\n  actual:   <end of text>");
    CHECK(messageFor("a\nb\n", "a\n") ==
          "line 2 differs\n  expected: <end of text>\n  actual:   \"b\\n\"");
}

TEST_CASE("a missing final line end is a difference on that line", "[e2e][harness]") {
    CHECK(messageFor("a\nb", "a\nb\n") ==
          "line 2 differs\n  expected: \"b\\n\"\n  actual:   \"b\"");
}

TEST_CASE("different line endings are shown as such", "[e2e][harness]") {
    CHECK(messageFor("a\r\nb\r\n", "a\nb\n") ==
          "line 1 differs\n  expected: \"a\\n\"\n  actual:   \"a\\r\\n\"");
}

TEST_CASE("a byte order mark is shown as such", "[e2e][harness]") {
    CHECK(messageFor("\xEF\xBB\xBF"
                     "1\n",
                     "1\n") ==
          "line 1 differs\n  expected: \"1\\n\"\n  actual:   \"\\xEF\\xBB\\xBF1\\n\"");
}

TEST_CASE("other control characters and quotes are escaped", "[e2e][harness]") {
    CHECK(messageFor(std::string{"a\0\"\t", 4}, "a") ==
          "line 1 differs\n  expected: \"a\"\n  actual:   \"a\\x00\\\"\\t\"");
}

TEST_CASE("the matcher compares against the bytes of a file", "[e2e][harness]") {
    const Scratch scratch;
    const std::string path = scratch.of("attendu.txt");
    {
        std::ofstream file{path, std::ios::binary};
        file << "a\r\nb\r\n";
    }

    CHECK_THAT(contentOf(path), MatchesFile(path));
    CHECK_THAT(std::string{"a\r\nb\r\n"}, MatchesFile(path));
    CHECK_THAT(std::string{"a\nb\n"}, !MatchesFile(path));
    CHECK_THAT(std::string{}, !MatchesFile(scratch.of("absent.txt")));
    CHECK_THAT(std::string{"a\nb\n"}, !MatchesFile(corpus("n-existe-pas")));
}
