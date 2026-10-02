#include <subedit/cli/index_grammar.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::parseRange;
using subedit::cli::parseReference;
using subedit::cli::parseSubtitleNumber;
using subedit::cli::Range;
using subedit::cli::Reference;
using subedit::cli::selectionOf;
using subedit::core::IndexRange;
using subedit::core::SubtitleIndex;

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

TEST_CASE("N-M names the subtitles from N to M, both included", "[cli][index][CLI-RANGE-01]") {
    CHECK(parseRange("3-7").value() == Range{.first = 3, .last = 7});
    // One subtitle is said N-N: the end is not optional by being equal.
    CHECK(parseRange("4-4").value() == Range{.first = 4, .last = 4});
    CHECK(parseRange("1-12").value() == Range{.first = 1, .last = 12});
}

TEST_CASE("N- goes to the end of the file, whatever it holds", "[cli][index][CLI-RANGE-01]") {
    CHECK(parseRange("3-").value() == Range{.first = 3, .last = std::nullopt});
}

TEST_CASE("a range that ends before it starts is refused", "[cli][index][CLI-RANGE-01]") {
    const auto read = parseRange("7-3");

    REQUIRE_FALSE(read.has_value());
    CHECK_THAT(read.error(), ContainsSubstring("\"7-3\""));
    CHECK_THAT(read.error(), ContainsSubstring("ends before it starts"));
}

TEST_CASE("zero is no end of a range", "[cli][index][CLI-RANGE-01]") {
    CHECK_THAT(parseRange("0-5").error(), ContainsSubstring("counted from 1"));
    CHECK_THAT(parseRange("1-0").error(), ContainsSubstring("counted from 1"));
}

TEST_CASE("what is not written N-M or N- is no range", "[cli][index][CLI-RANGE-01]") {
    // A lone number would mean one subtitle, or the rest: two readings, so none.
    const auto lone = parseRange("5");
    REQUIRE_FALSE(lone.has_value());
    CHECK_THAT(lone.error(), ContainsSubstring("write it N-M"));

    CHECK_FALSE(parseRange("").has_value());
    CHECK_FALSE(parseRange("-").has_value());
    CHECK_FALSE(parseRange("-5").has_value());
    CHECK_FALSE(parseRange("a-b").has_value());
    CHECK_FALSE(parseRange("1.5-3").has_value());
    CHECK_FALSE(parseRange("1-2-3").has_value());
    CHECK_FALSE(parseRange(" 1-3").has_value());
}

TEST_CASE("a range inside the file selects those subtitles", "[cli][index][CLI-RANGE-01]") {
    const auto selection = selectionOf(Range{.first = 2, .last = 4}, 5);

    REQUIRE(selection.has_value());
    // The numbers are one-based, the selection's indices are not.
    CHECK(selection->ranges().size() == 1);
    CHECK(selection->ranges().front() ==
          IndexRange{SubtitleIndex::fromValue(1), SubtitleIndex::fromValue(3)});
}

TEST_CASE("a range to the end selects up to the last subtitle", "[cli][index][CLI-RANGE-01]") {
    const auto selection = selectionOf(Range{.first = 3, .last = std::nullopt}, 5);

    REQUIRE(selection.has_value());
    CHECK(selection->ranges().front() ==
          IndexRange{SubtitleIndex::fromValue(2), SubtitleIndex::fromValue(4)});
}

TEST_CASE("the bounds are the file's own: the last subtitle is a valid end",
          "[cli][index][CLI-RANGE-01]") {
    CHECK(selectionOf(Range{.first = 5, .last = 5}, 5).has_value());
    CHECK(selectionOf(Range{.first = 1, .last = 5}, 5).has_value());
    CHECK(selectionOf(Range{.first = 5, .last = std::nullopt}, 5).has_value());
}

TEST_CASE("a start past the file is refused, naming how many it has",
          "[cli][index][CLI-RANGE-02]") {
    const auto selection = selectionOf(Range{.first = 7, .last = 9}, 5);

    REQUIRE_FALSE(selection.has_value());
    CHECK(selection.error() == "range 7-9 starts after the last subtitle: the file has 5");
    CHECK_THAT(selectionOf(Range{.first = 6, .last = std::nullopt}, 5).error(),
               ContainsSubstring("range 6- starts after the last subtitle"));
}

TEST_CASE("an end past the file is refused, naming how many it has", "[cli][index][CLI-RANGE-02]") {
    const auto selection = selectionOf(Range{.first = 3, .last = 9}, 5);

    REQUIRE_FALSE(selection.has_value());
    CHECK(selection.error() == "range 3-9 ends after the last subtitle: the file has 5");
}

TEST_CASE("a file with no subtitle holds no range", "[cli][index][CLI-RANGE-02]") {
    const auto selection = selectionOf(Range{.first = 1, .last = std::nullopt}, 0);

    REQUIRE_FALSE(selection.has_value());
    CHECK_THAT(selection.error(), ContainsSubstring("the file has 0"));
}
