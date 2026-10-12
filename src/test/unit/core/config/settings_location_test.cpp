#include <subedit/core/config/settings_location.hpp>

#include <catch2/catch_test_macros.hpp>

using subedit::core::userSettingsLocation;

TEST_CASE("The settings file lives under XDG_CONFIG_HOME when it is set", "[settings][location]") {
    CHECK(userSettingsLocation("/x/config", "/home/u") == "/x/config/subedit/settings.conf");
}

TEST_CASE("The settings file falls back to ~/.config", "[settings][location]") {
    CHECK(userSettingsLocation("", "/home/u") == "/home/u/.config/subedit/settings.conf");
}

TEST_CASE("A relative XDG_CONFIG_HOME is ignored, as Qt and the XDG spec do",
          "[settings][location]") {
    CHECK(userSettingsLocation("relative/dir", "/home/u") ==
          "/home/u/.config/subedit/settings.conf");
}

TEST_CASE("With neither variable there is no location", "[settings][location]") {
    CHECK(userSettingsLocation("", "").empty());
}
