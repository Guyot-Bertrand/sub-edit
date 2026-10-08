// The conversion that puts a file back on a grid — issue #622, decision D11 (and #386).
//
// **The truth is posed, not found.** `src/test/data/conversions-fausses/` is made by
// `src/scripts/wrong-rate-pairs.py`, which converts a file on a known grid by a wrong ratio, by
// arithmetic, and writes beside it the ratio that undoes it. The fifteen grid fixtures are the
// guard the other way: a file that is clean, partial or on no grid at all is offered nothing.

#include <subedit/core/analysis/grid_repair.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/ratio.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace {

using subedit::core::findGridRepair;
using subedit::core::GridRepair;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::Ratio;
using subedit::core::RealFileSystem;
using subedit::core::RepairOutcome;

[[nodiscard]] subedit::core::Project projectAt(const std::filesystem::path& file) {
    const RealFileSystem files;
    std::expected<OpenedFile, subedit::core::OpenError> opened = openProject(files, file);
    REQUIRE(opened.has_value());
    return std::move(opened->project);
}

[[nodiscard]] std::filesystem::path wrongConversions() {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "conversions-fausses";
}

[[nodiscard]] std::string truthOf(std::string_view name) {
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content =
        files.readFile(wrongConversions() / name / "verite.txt");
    REQUIRE(content.has_value());
    return content->substr(0, content->find('\n'));
}

[[nodiscard]] std::optional<Ratio> ratioOf(const std::string& text) {
    const std::size_t slash = text.find('/');
    if (slash == std::string::npos)
        return std::nullopt;
    return Ratio::create(std::stoll(text.substr(0, slash)), std::stoll(text.substr(slash + 1)));
}

constexpr std::array<std::string_view, 10> kCases = {
    "propre-24",
    "faux-25-vers-24",
    "faux-25-vers-60",
    "egalite-24-vers-25",
    "egalite-50-vers-30",
    "egalite-28-8",
    "egalite-23976",
    "egalite-courte",
    "hors-ensemble",
    "sans-grille",
};

} // namespace

TEST_CASE("the conversion that was posed is the one found", "[core][analysis][GUI-REPAIR-01]") {
    for (const std::string_view name : kCases) {
        DYNAMIC_SECTION(name) {
            const GridRepair repair =
                findGridRepair(projectAt(wrongConversions() / name / "fichier.srt"));
            const std::string truth = truthOf(name);

            if (const std::optional<Ratio> wanted = ratioOf(truth); wanted.has_value()) {
                // The ratio is the answer; the pair that makes it is one of several.
                REQUIRE(repair.outcome == RepairOutcome::Found);
                REQUIRE(repair.conversion.has_value());
                const subedit::core::RateConversion chosen =
                    repair.conversion.value_or(subedit::core::RateConversion{
                        .input = subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25},
                        .output =
                            subedit::core::FrameRate{subedit::core::StandardFrameRate::Fps25}});
                CHECK(chosen.input.conversionTo(chosen.output) == *wanted);
                CHECK(repair.concentration > repair.runnerUp);
                CHECK(repair.concentration > repair.asIs);
            } else if (truth == "egalite") {
                CHECK(repair.outcome == RepairOutcome::Tied);
                CHECK(repair.conversion.has_value());
                CHECK(repair.rival.has_value());
            } else {
                CHECK(repair.outcome != RepairOutcome::Found);
                CHECK(repair.outcome != RepairOutcome::Tied);
            }
        }
    }
}

TEST_CASE("a clean file has nothing to repair", "[core][analysis][GUI-REPAIR-01]") {
    const GridRepair repair =
        findGridRepair(projectAt(wrongConversions() / "propre-24" / "fichier.srt"));
    CHECK(repair.outcome == RepairOutcome::NotNeeded);
    CHECK_FALSE(repair.conversion.has_value());
}

TEST_CASE("a file on no grid is told so, and offered nothing", "[core][analysis][GUI-REPAIR-01]") {
    for (const std::string_view name : {"sans-grille", "hors-ensemble"}) {
        DYNAMIC_SECTION(name) {
            const GridRepair repair =
                findGridRepair(projectAt(wrongConversions() / name / "fichier.srt"));
            CHECK(repair.outcome == RepairOutcome::NothingFits);
            CHECK_FALSE(repair.conversion.has_value());
        }
    }
}

TEST_CASE("two conversions that fit equally are both named and neither proposed",
          "[core][analysis][GUI-REPAIR-01]") {
    const GridRepair repair =
        findGridRepair(projectAt(wrongConversions() / "egalite-courte" / "fichier.srt"));
    REQUIRE(repair.outcome == RepairOutcome::Tied);
    REQUIRE(repair.conversion.has_value());
    REQUIRE(repair.rival.has_value());
    CHECK_FALSE(repair.conversion == repair.rival);

    // A conversion is equal to itself on both of its rates, and to nothing else.
    const std::optional<subedit::core::RateConversion> copy = repair.conversion;
    CHECK(copy == repair.conversion);
}

TEST_CASE("no grid fixture is offered a conversion", "[core][analysis][GUI-REPAIR-01]") {
    const std::filesystem::path grids = std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "grilles";
    std::size_t seen = 0;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{grids}) {
        if (entry.path().extension() != ".srt")
            continue;
        ++seen;
        DYNAMIC_SECTION(entry.path().filename().string()) {
            const GridRepair repair = findGridRepair(projectAt(entry.path()));
            // A file on a grid is left alone, and the mixed and absurd ones are not mended by a
            // conversion either: whatever is proposed would be one these files do not have.
            CHECK(repair.outcome != RepairOutcome::Found);
        }
    }
    CHECK(seen == 15);
}

TEST_CASE("too few positions to judge by are not repaired", "[core][analysis][GUI-REPAIR-01]") {
    const GridRepair repair = findGridRepair(std::span<const subedit::core::Timestamp>{});
    CHECK(repair.outcome == RepairOutcome::NotNeeded);
}
