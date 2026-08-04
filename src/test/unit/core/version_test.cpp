#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

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
