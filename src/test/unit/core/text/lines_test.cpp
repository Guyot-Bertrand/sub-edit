#include <subedit/core/text/lines.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::splitLines;

std::vector<std::string> asStrings(std::string_view content) {
    std::vector<std::string> lines;
    for (const std::string_view line : splitLines(content))
        lines.emplace_back(line);
    return lines;
}

} // namespace

TEST_CASE("lines are cut on the line feed", "[format][lines]") {
    CHECK(asStrings("un\ndeux\ntrois") == std::vector<std::string>{"un", "deux", "trois"});
}

TEST_CASE("a carriage return before the line feed is not part of the line", "[format][lines]") {
    // A file written on Windows must read like any other; putting the ending
    // back is a decision taken at writing time, not something the reader
    // should carry inside its text.
    CHECK(asStrings("un\r\ndeux\r\n") == std::vector<std::string>{"un", "deux"});
}

TEST_CASE("a lone carriage return also ends a line", "[format][lines]") {
    CHECK(asStrings("un\rdeux") == std::vector<std::string>{"un", "deux"});
}

TEST_CASE("endings may be mixed within one file", "[format][lines]") {
    // They should not be, and the reader says so; but saying so requires
    // having read the file first.
    CHECK(asStrings("un\r\ndeux\ntrois\r") == std::vector<std::string>{"un", "deux", "trois"});
}

TEST_CASE("a blank line is a line", "[format][lines]") {
    CHECK(asStrings("un\n\ndeux") == std::vector<std::string>{"un", "", "deux"});
}

TEST_CASE("a trailing ending does not add an empty line", "[format][lines]") {
    CHECK(asStrings("un\n") == std::vector<std::string>{"un"});
    CHECK(asStrings("un") == std::vector<std::string>{"un"});
}

TEST_CASE("nothing at all is no line", "[format][lines]") {
    CHECK(asStrings("").empty());
}
