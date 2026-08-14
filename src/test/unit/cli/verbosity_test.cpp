#include <subedit/cli/verbosity.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::levelFrom;

TEST_CASE("no flag at all is the default level", "[cli][verbosity]") {
    const auto level = levelFrom(false, 0);

    REQUIRE(level.has_value());
    CHECK(*level == 1);
}

TEST_CASE("a single -v is the default level too", "[cli][verbosity]") {
    // Saying -v when it is already the default is not an error, it is a caller
    // being explicit.
    CHECK(levelFrom(false, 1).value() == levelFrom(false, 0).value());
}

TEST_CASE("each further -v raises the level", "[cli][verbosity]") {
    CHECK(levelFrom(false, 2).value() == 2);
    CHECK(levelFrom(false, 3).value() == 3);
}

TEST_CASE("quiet is level zero", "[cli][verbosity]") {
    CHECK(levelFrom(true, 0).value() == 0);
}

TEST_CASE("asking for silence and detail at once is refused", "[cli][verbosity]") {
    const auto level = levelFrom(true, 2);

    REQUIRE_FALSE(level.has_value());
    // The message names both, so that the caller sees which two requests were
    // read as opposite.
    CHECK_THAT(level.error(), ContainsSubstring("--quiet"));
    CHECK_THAT(level.error(), ContainsSubstring("-v"));
}
