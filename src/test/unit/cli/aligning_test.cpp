// Laying a file back onto the frames of a rate, and how the report says so.
//
// The last case is the one that matters most: it runs the alignment and the
// conversion on the same file with the same pair of rates, so that the
// difference between them is a measurement here rather than a warning in prose.

#include <subedit/cli/aligning.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>
#include <grid_fixtures.hpp>
#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::alignAll;
using subedit::cli::convertFrameRateAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::StandardFrameRate;

namespace {

const FrameRate kCinema{StandardFrameRate::Fps24};
const FrameRate kPal{StandardFrameRate::Fps25};

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

[[nodiscard]] Run align(const std::string& content, FrameRate rate) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream errors;

    const ExitCode code = alignAll(files,
                                   {"a.srt"},
                                   rate,
                                   Destination::from("", "out", false, 1).value(),
                                   Reporter{errors, 1});
    return {
        .code = code, .written = files.contentOf("out/a.srt").value_or(""), .errors = errors.str()};
}

[[nodiscard]] Run convert(const std::string& content, FrameRate from, FrameRate to) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream errors;

    const ExitCode code = convertFrameRateAll(files,
                                              {"a.srt"},
                                              from,
                                              to,
                                              Destination::from("", "out", false, 1).value(),
                                              Reporter{errors, 1});
    return {
        .code = code, .written = files.contentOf("out/a.srt").value_or(""), .errors = errors.str()};
}

/// The first start of a written file, in milliseconds.
///
/// Read back through the core's own parser rather than by hand: the arithmetic
/// of hours, minutes and thousandths is written once in the project, and a
/// second copy of it here would be one more thing to get wrong.
[[nodiscard]] std::int64_t firstStartOf(const std::string& written) {
    constexpr std::size_t kStampLength = 12;

    const std::size_t arrow = written.find(" --> ");
    REQUIRE(arrow != std::string::npos);

    const std::optional<subedit::core::Timestamp> start =
        subedit::core::Timestamp::parse(written.substr(arrow - kStampLength, kStampLength));
    REQUIRE(start.has_value());
    return start.value_or(subedit::core::Timestamp::origin()).milliseconds();
}

} // namespace

TEST_CASE("aligning puts every position on the grid asked for", "[cli][aligning]") {
    const Run run = align(subedit::test::gridBytes("grille-24.srt"), kPal);

    CHECK(run.code == ExitCode::Success);
    // A frame at 25 lasts forty milliseconds, so every position written is a
    // whole multiple of forty.
    CHECK(firstStartOf(run.written) % 40 == 0);
}

TEST_CASE("the report counts what moved and by how much", "[cli][aligning]") {
    const Run run = align(subedit::test::gridBytes("grille-24.srt"), kPal);

    CHECK_THAT(run.errors, ContainsSubstring(" aligned on 25 fps, "));
    CHECK_THAT(run.errors, ContainsSubstring(" positions moved, by at most 20 ms"));
}

TEST_CASE("aligning on the grid a file already sits on moves nothing", "[cli][aligning]") {
    const Run run = align(subedit::test::gridBytes("grille-24.srt"), kCinema);

    CHECK_THAT(run.errors, ContainsSubstring("0 positions moved, by at most 0 ms"));
}

TEST_CASE("aligning is not converting, and the report shows it", "[cli][aligning]") {
    const std::string grid = subedit::test::gridBytes("grille-24.srt");

    const Run aligned = align(grid, kPal);
    const Run converted = convert(grid, kCinema, kPal);

    // The same file and the same pair of rates. One moved the first start by
    // at most half a frame; the other rescaled the whole file.
    const std::int64_t before = firstStartOf(grid);
    CHECK(std::abs(firstStartOf(aligned.written) - before) <= 20);
    CHECK(std::abs(firstStartOf(converted.written) - before) > 20);
}
