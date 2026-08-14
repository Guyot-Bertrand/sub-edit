#include <subedit/cli/destination.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;

TEST_CASE("no destination at all is refused", "[cli][destination]") {
    const auto destination = Destination::from("", "", false, 1);

    REQUIRE_FALSE(destination.has_value());
    // A harness that overwrites its own input without being asked is a harness
    // one stops using: the refusal names the three ways to say where to write.
    CHECK_THAT(destination.error(), ContainsSubstring("--output"));
    CHECK_THAT(destination.error(), ContainsSubstring("--output-dir"));
    CHECK_THAT(destination.error(), ContainsSubstring("--in-place"));
}

TEST_CASE("two destinations at once are refused", "[cli][destination]") {
    CHECK_FALSE(Destination::from("a.vtt", "out", false, 1).has_value());
    CHECK_FALSE(Destination::from("a.vtt", "", true, 1).has_value());
    CHECK_FALSE(Destination::from("", "out", true, 1).has_value());
}

TEST_CASE("a single output file cannot take several inputs", "[cli][destination]") {
    const auto destination = Destination::from("a.vtt", "", false, 2);

    REQUIRE_FALSE(destination.has_value());
    // Writing the last input over the previous ones is the outcome this
    // refusal exists to prevent, so the message points at the option that works.
    CHECK_THAT(destination.error(), ContainsSubstring("--output-dir"));
}

TEST_CASE("a named output file is used as it is", "[cli][destination]") {
    const Destination destination = Destination::from("out/renamed.vtt", "", false, 1).value();

    // The caller named the file; nothing is added to it, not even an extension
    // that would match the format.
    CHECK(destination.pathFor("input/a.srt", ".vtt") == "out/renamed.vtt");
}

TEST_CASE("in place writes back over the input", "[cli][destination]") {
    const Destination destination = Destination::from("", "", true, 3).value();

    CHECK(destination.pathFor("input/a.srt", "") == "input/a.srt");
    CHECK(destination.isInPlace());
}

TEST_CASE("a directory keeps the base name", "[cli][destination]") {
    const Destination destination = Destination::from("", "out", false, 2).value();

    CHECK(destination.pathFor("input/a.srt", "") == "out/a.srt");
}

TEST_CASE("a directory takes the extension of the format written", "[cli][destination]") {
    const Destination destination = Destination::from("", "out", false, 2).value();

    // Writing WebVTT into a file named .srt would produce a file that lies
    // about itself — every other tool would trip on it.
    CHECK(destination.pathFor("input/a.srt", ".vtt") == "out/a.vtt");
}

TEST_CASE("a directory accepts a single input too", "[cli][destination]") {
    CHECK(Destination::from("", "out", false, 1).has_value());
}
