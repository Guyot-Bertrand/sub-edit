// A translation laid a constant time away from the main document — issue #621, decision D10.
//
// **The truth is posed, not found.** `src/test/data/decalages/` is made by
// `src/scripts/shifted-pairs.py`, which lays a shift on a translation by arithmetic and writes
// it next to the pair; what the detection finds is confronted with that file. The eight pairs
// of `paires/` are the guard the other way: none may be offered a shift it does not have.

#include <subedit/core/edit/translation.hpp>
#include <subedit/core/edit/translation_drift.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/translation_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>

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

using subedit::core::ConstantShift;
using subedit::core::findConstantShift;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::openTranslation;
using subedit::core::previewTranslation;
using subedit::core::Project;
using subedit::core::RealFileSystem;
using subedit::core::shiftedBack;
using subedit::core::TranslationFile;
using subedit::core::TranslationMethod;

/// One directory of cases, opened.
struct Pair {
    Project project;
    TranslationFile translation;
    std::filesystem::path directory;
};

[[nodiscard]] Pair pairOf(const std::filesystem::path& directory) {
    const RealFileSystem files;
    std::expected<OpenedFile, subedit::core::OpenError> opened =
        openProject(files, directory / "principal.srt");
    REQUIRE(opened.has_value());
    std::expected<TranslationFile, subedit::core::TranslationError> translation =
        openTranslation(files, opened->project, directory / "traduction.srt");
    REQUIRE(translation.has_value());
    return Pair{
        .project = std::move(opened->project),
        .translation = std::move(*translation),
        .directory = directory,
    };
}

/// What the generator wrote as the truth: milliseconds, or nothing at all.
[[nodiscard]] std::optional<std::int64_t> truthOf(const std::filesystem::path& directory) {
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content =
        files.readFile(directory / "verite.txt");
    REQUIRE(content.has_value());
    if (content->starts_with("aucun"))
        return std::nullopt;
    return std::stoll(*content);
}

[[nodiscard]] std::filesystem::path shiftedPairs() {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "decalages";
}

constexpr std::array<std::string_view, 6> kShiftedCases = {
    "sans-decalage",
    "constant-plus-2000",
    "constant-moins-1500",
    "constant-plus-2340",
    "derive",
    "partiel",
};

constexpr std::array<std::string_view, 8> kPairs = {
    "temoin",
    "traduction-plus-courte",
    "traduction-dans-le-desordre",
    "une-ligne-de-moins-au-milieu",
    "une-ligne-de-plus-a-la-fin",
    "une-ligne-dans-un-intervalle-vide",
    "deux-lignes-dans-un-meme-sous-titre",
    "positions-decalees",
};

} // namespace

TEST_CASE("A constant shift is found as it was laid", "[core][translation][GUI-DRIFT-01]") {
    for (const std::string_view name : kShiftedCases) {
        DYNAMIC_SECTION(name) {
            const Pair pair = pairOf(shiftedPairs() / name);
            const std::optional<std::int64_t> truth = truthOf(pair.directory);
            const std::optional<ConstantShift> found =
                findConstantShift(pair.project, pair.translation.lines);

            // A shift is found exactly when one was laid, and it is the one that was.
            CHECK(found.has_value() == truth.has_value());
            if (found.has_value() && truth.has_value()) {
                CHECK(found->lateBy.milliseconds() == *truth);
                CHECK(found->ifShifted.isClean());
                CHECK_FALSE(found->asOpened.isClean());
            }
        }
    }
}

TEST_CASE("A drift or a partial shift is offered nothing", "[core][translation][GUI-DRIFT-01]") {
    for (const std::string_view name : {"derive", "partiel"}) {
        DYNAMIC_SECTION(name) {
            const Pair pair = pairOf(shiftedPairs() / name);
            CHECK_FALSE(findConstantShift(pair.project, pair.translation.lines).has_value());
        }
    }
}

TEST_CASE("No pair of the corpus is offered a shift but the one that has it",
          "[core][translation][GUI-DRIFT-01]") {
    for (const std::string_view name : kPairs) {
        DYNAMIC_SECTION(name) {
            const Pair pair =
                pairOf(std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "paires" / name);
            const std::optional<ConstantShift> found =
                findConstantShift(pair.project, pair.translation.lines);

            // Only the pair written to be two seconds late has a shift: the rest are missing
            // lines, extra lines and lines out of order, none of which a shift mends.
            CHECK(found.has_value() == (name == "positions-decalees"));
            if (found.has_value() && name == "positions-decalees")
                CHECK(found->lateBy.milliseconds() == 2000);
        }
    }
}

TEST_CASE("The shift says what opening would say before and after",
          "[core][translation][GUI-DRIFT-01]") {
    const Pair pair =
        pairOf(std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "paires" / "positions-decalees");
    const std::optional<ConstantShift> offered =
        findConstantShift(pair.project, pair.translation.lines);
    REQUIRE(offered.has_value());
    const ConstantShift found = offered.value_or(ConstantShift{});

    // The judgement is the opening's own, and the same one the window reports.
    CHECK(found.asOpened ==
          previewTranslation(pair.project, pair.translation.lines, TranslationMethod::Position));
    CHECK(found.ifShifted == previewTranslation(pair.project,
                                                shiftedBack(pair.translation.lines, found.lateBy),
                                                TranslationMethod::Position));
    CHECK(found.ifShifted.attached == 4);
    CHECK(found.asOpened.attached == 1);
}

TEST_CASE("One line is too little to call a shift", "[core][translation][GUI-DRIFT-01]") {
    Pair pair = pairOf(shiftedPairs() / "constant-plus-2000");
    pair.translation.lines.resize(1);
    CHECK_FALSE(findConstantShift(pair.project, pair.translation.lines).has_value());
}

TEST_CASE("Two shifts that both mend the opening leave none to offer",
          "[core][translation][GUI-DRIFT-01]") {
    // Subtitles three seconds long, four apart. A line a second long, 2.8 s into a subtitle (the
    // first three) or 2.9 s (the last three), falls in the gap and finds none. Moved back by
    // either gap, every line lands inside its subtitle: two shifts mend it, so none is the one.
    constexpr int kPeriod = 4000;
    constexpr int kLength = 3000;
    constexpr int kEarlyInto = 2800;
    constexpr int kLateInto = 2900;
    constexpr int kHalf = 3;
    const auto stamp = [](int milliseconds) {
        const std::string seconds = std::to_string(milliseconds / 1000);
        const std::string thousandths = std::to_string(1000 + (milliseconds % 1000)).substr(1);
        return "00:00:" + std::string(seconds.size() < 2 ? "0" : "") + seconds + "," + thousandths;
    };
    std::string main;
    std::string lines;
    for (int rank = 0; rank < 2 * kHalf; ++rank) {
        const int start = rank * kPeriod;
        const int at = start + (rank < kHalf ? kEarlyInto : kLateInto);
        const std::string number = std::to_string(rank + 1) + "\n";
        main += number + stamp(start) + " --> " + stamp(start + kLength) + "\nMain.\n\n";
        lines += number + stamp(at) + " --> " + stamp(at + 1000) + "\nLine.\n\n";
    }
    InMemoryFileSystem files;
    files.addFile("p.srt", main);
    files.addFile("t.srt", lines);
    auto opened = openProject(files, "p.srt");
    REQUIRE(opened.has_value());
    auto translation = openTranslation(files, opened->project, "t.srt");
    REQUIRE(translation.has_value());

    // Each shift alone would clean the opening: that is what makes the refusal the right one.
    for (const std::int64_t gap : {std::int64_t{kEarlyInto}, std::int64_t{kLateInto}}) {
        const auto moved =
            shiftedBack(translation->lines, subedit::core::Duration::fromMilliseconds(gap));
        CHECK(previewTranslation(opened->project, moved, TranslationMethod::Position).isClean());
    }
    CHECK_FALSE(findConstantShift(opened->project, translation->lines).has_value());
}
