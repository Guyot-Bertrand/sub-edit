// What translation costs at start-up (#684): reading a shipped catalogue, which every launch in
// a language other than English does once, and asking it for a message, which every label does.
//
// The catalogues are the ones of the build tree, compiled from `src/po/` as the installation
// lays them out. Under the `bench` preset that is a Release tree, the shipped mode.

#include <subedit/core/i18n/catalogue.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace {

using subedit::core::Catalogue;

std::filesystem::path shippedCatalogue(std::string_view language) {
    return std::filesystem::path{SUBEDIT_SHIPPED_LOCALE_DIR} / std::string{language} /
           "LC_MESSAGES" / "subedit.mo";
}

} // namespace

TEST_CASE("catalogue loading", "[benchmark]") {
    const std::filesystem::path french = shippedCatalogue("fr");
    if (!std::filesystem::exists(french))
        SKIP("the French catalogue is not built (" + french.string() + ")");

    BENCHMARK("loading the French catalogue") {
        return Catalogue::fromFile(french).has_value();
    };

    const std::filesystem::path russian = shippedCatalogue("ru");
    if (std::filesystem::exists(russian)) {
        BENCHMARK("loading the Russian catalogue") {
            return Catalogue::fromFile(russian).has_value();
        };
    }

    const auto loaded = Catalogue::fromFile(french);
    REQUIRE(loaded.has_value());
    BENCHMARK("looking a message up in the French catalogue") {
        return loaded->find("", "Open").has_value();
    };
}
