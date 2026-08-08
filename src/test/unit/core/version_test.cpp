#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {

using subedit::core::Version;

} // namespace

TEST_CASE("version reports the number the library was built with", "[version]") {
    const Version current = subedit::core::version();

    CHECK(current.majorNumber >= 0);
    CHECK(current.minorNumber >= 0);
    CHECK(current.patchNumber >= 0);
}

TEST_CASE("versionString formats the three numbers separated by dots", "[version]") {
    const Version current = subedit::core::version();

    const std::string expected = std::to_string(current.majorNumber) + "." +
                                 std::to_string(current.minorNumber) + "." +
                                 std::to_string(current.patchNumber);

    CHECK(subedit::core::versionString() == expected);
}

TEST_CASE("version is stable across calls", "[version]") {
    CHECK(subedit::core::version() == subedit::core::version());
    CHECK(subedit::core::versionString() == subedit::core::versionString());
}

TEST_CASE("the build date names the commit the library came from", "[version]") {
    // Empty is a legitimate answer: a source archive has no repository to ask.
    // Anything else is a date, and nothing in between.
    const std::string_view date = subedit::core::buildDate();

    if (date.empty())
        return;

    REQUIRE(date.size() == 10);
    CHECK(date[4] == '-');
    CHECK(date[7] == '-');
    CHECK(date.find_first_not_of("0123456789-") == std::string_view::npos);
}

TEST_CASE("the build date does not move between two calls", "[version]") {
    CHECK(subedit::core::buildDate() == subedit::core::buildDate());
}
