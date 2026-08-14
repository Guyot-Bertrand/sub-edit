#include <subedit/cli/index_grammar.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::parseReference;
using subedit::cli::parseSubtitleNumber;
using subedit::cli::Reference;

TEST_CASE("a subtitle is named by the number it shows", "[cli][index]") {
    CHECK(parseSubtitleNumber("1").value() == 1);
    CHECK(parseSubtitleNumber("42").value() == 42);
}

TEST_CASE("zero names no subtitle", "[cli][index]") {
    const auto read = parseSubtitleNumber("0");

    REQUIRE_FALSE(read.has_value());
    // The off-by-one this project has a type for: the user reads 1 on the first
    // line of a SubRip file, so 1 is what they write here.
    CHECK_THAT(read.error(), ContainsSubstring("counted from 1"));
}

TEST_CASE("what is not a whole number names no subtitle", "[cli][index]") {
    CHECK_FALSE(parseSubtitleNumber("").has_value());
    CHECK_FALSE(parseSubtitleNumber("-1").has_value());
    CHECK_FALSE(parseSubtitleNumber("1.5").has_value());
    CHECK_FALSE(parseSubtitleNumber("premier").has_value());
    CHECK_FALSE(parseSubtitleNumber("1 ").has_value());
}

TEST_CASE("a refusal names what was given", "[cli][index]") {
    CHECK_THAT(parseSubtitleNumber("premier").error(), ContainsSubstring("premier"));
}

TEST_CASE("a number too large for the count is refused rather than wrapped", "[cli][index]") {
    // Ten nonillion subtitles is not a file, it is an overflow waiting for a
    // multiplication.
    CHECK_FALSE(parseSubtitleNumber("99999999999999999999999999").has_value());
}

TEST_CASE("a reference pairs a subtitle with where its start belongs", "[cli][index]") {
    const Reference reference = parseReference("3=00:00:10.000").value();

    CHECK(reference.number == 3);
    CHECK(reference.target.milliseconds() == 10'000);
}

TEST_CASE("a reference takes its time in seconds too", "[cli][index]") {
    // The same grammar as --by: one way of writing time in the whole tool.
    CHECK(parseReference("1=2.999").value().target.milliseconds() == 2'999);
}

TEST_CASE("a reference without an equals sign is refused", "[cli][index]") {
    const auto read = parseReference("3 00:00:10.000");

    REQUIRE_FALSE(read.has_value());
    CHECK_THAT(read.error(), ContainsSubstring("<index>=<time>"));
}

TEST_CASE("a reference whose index is not one is refused", "[cli][index]") {
    CHECK_THAT(parseReference("0=1.000").error(), ContainsSubstring("counted from 1"));
    CHECK_THAT(parseReference("=1.000").error(), ContainsSubstring("<index>=<time>"));
}

TEST_CASE("a reference whose time is not one is refused", "[cli][index]") {
    CHECK_THAT(parseReference("3=1,000").error(), ContainsSubstring("decimal point"));
    CHECK_THAT(parseReference("3=").error(), ContainsSubstring("<index>=<time>"));
}

TEST_CASE("only the first equals sign separates the two", "[cli][index]") {
    // A second one belongs to neither side, and the time grammar says so.
    CHECK_FALSE(parseReference("3=1=2").has_value());
}
