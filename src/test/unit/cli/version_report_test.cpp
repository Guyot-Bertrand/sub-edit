// What `--version` writes on the command line — issue #716.

#include <subedit/cli/version_report.hpp>
#include <subedit/core/libraries.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace {

const std::vector<subedit::core::Library> kLibraries{
    {.name = "ICU", .loaded = "74.2", .built = "74.1"},
    {.name = "CLI11", .loaded = "2.4.1", .built = "2.4.1"},
};

} // namespace

TEST_CASE("in text, the version writes the core's report", "[version]") {
    std::ostringstream out;
    subedit::cli::writeVersion(out, kLibraries, {"en", "fr"}, false);

    CHECK(out.str() == subedit::core::versionReport(kLibraries, {"en", "fr"}));
}

TEST_CASE("in json, the version is one object on one line", "[version]") {
    std::ostringstream out;
    subedit::cli::writeVersion(out, kLibraries, {"en", "fr"}, true);

    CHECK(out.str() == R"({"schema":1,"command":"version","version":")" +
                           subedit::core::versionString() +
                           R"(","libraries":[{"name":"ICU","loaded":"74.2","built":"74.1"},)"
                           R"({"name":"CLI11","loaded":"2.4.1","built":"2.4.1"}],)"
                           R"("languages":["en","fr"]})"
                           "\n");
}
