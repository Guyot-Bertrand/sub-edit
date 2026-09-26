// Where the patterns are — issue #498, ADR 0037.

#include <subedit/core/text/pattern_paths.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

using subedit::core::shippedPatternsPath;
using subedit::core::userPatternsPath;

TEST_CASE("the shipped patterns are beside the executable's prefix, whatever the prefix",
          "[text][pattern]") {
    // The package's `/usr`, a `/usr/local`, `~/.local`: the layout is the one
    // `GNUInstallDirs` produces, so it is right for all of them.
    CHECK(shippedPatternsPath("/usr/bin") == std::filesystem::path{"/usr/share/subedit/patterns"});
    CHECK(shippedPatternsPath("/home/x/.local/bin") ==
          std::filesystem::path{"/home/x/.local/share/subedit/patterns"});
    CHECK(shippedPatternsPath("/tmp/prefix/bin/") ==
          std::filesystem::path{"/tmp/prefix/share/subedit/patterns"});
}

TEST_CASE("a user's patterns follow XDG_DATA_HOME, and fall back on the home", "[text][pattern]") {
    CHECK(userPatternsPath("/data", "/home/x") == std::filesystem::path{"/data/subedit/patterns"});

    const std::filesystem::path fallback{"/home/x/.local/share/subedit/patterns"};
    CHECK(userPatternsPath("", "/home/x") == fallback);
    // The XDG specification says a relative value is to be ignored.
    CHECK(userPatternsPath("relative/data", "/home/x") == fallback);
}

TEST_CASE("a machine with neither has no directory of user patterns", "[text][pattern]") {
    CHECK(userPatternsPath("", "").empty());
    CHECK(userPatternsPath("relative", "").empty());
}
