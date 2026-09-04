#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>
#include <grid_fixtures.hpp>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::shiftAll;
using subedit::core::Duration;
using subedit::core::InMemoryFileSystem;

namespace {

const std::string kTwo = "1\n"
                         "00:00:10,000 --> 00:00:12,000\n"
                         "First.\n"
                         "\n"
                         "2\n"
                         "00:00:20,000 --> 00:00:22,000\n"
                         "Second.\n"
                         "\n";

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

Run shift(const std::string& content, std::int64_t milliseconds) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream errors;

    const ExitCode code = shiftAll(files,
                                   {"a.srt"},
                                   std::nullopt,
                                   Duration::fromMilliseconds(milliseconds),
                                   Destination::from("", "out", false, 1).value(),
                                   Reporter{errors, 1});
    return {
        .code = code, .written = files.contentOf("out/a.srt").value_or(""), .errors = errors.str()};
}

} // namespace

TEST_CASE("a shift forward moves every position", "[cli][shifting]") {
    const Run run = shift(kTwo, 2'999);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:12,999 --> 00:00:14,999"));
    CHECK_THAT(run.written, ContainsSubstring("00:00:22,999 --> 00:00:24,999"));
}

TEST_CASE("a shift backwards moves every position", "[cli][shifting]") {
    const Run run = shift(kTwo, -7'001);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:02,999 --> 00:00:04,999"));
}

TEST_CASE("a shift keeps every subtitle on screen as long", "[cli][shifting]") {
    // What makes a shift a shift: both ends move by the same amount.
    CHECK_THAT(shift(kTwo, 5'000).written, ContainsSubstring("00:00:15,000 --> 00:00:17,000"));
}

TEST_CASE("the format of the file read is kept", "[cli][shifting]") {
    // Changing format is the business of convert, and a subcommand doing both
    // would make the errors of one indistinguishable from the errors of the
    // other.
    InMemoryFileSystem files;
    files.addFile("a.vtt", "WEBVTT\n\n00:10.000 --> 00:12.000\nOnly one.\n");
    std::ostringstream errors;

    CHECK(shiftAll(files,
                   {"a.vtt"},
                   std::nullopt,
                   Duration::fromMilliseconds(1'000),
                   Destination::from("", "out", false, 1).value(),
                   Reporter{errors, 0}) == ExitCode::Success);

    CHECK_THAT(files.contentOf("out/a.vtt").value_or(""), ContainsSubstring("WEBVTT"));
}

TEST_CASE("a shift that would go before the origin is refused", "[cli][shifting]") {
    const Run run = shift(kTwo, -11'000);

    CHECK(run.code == ExitCode::AllFailed);
    // Nothing written: a file holding "-00:00:01,000" is one no player reads,
    // and producing it would be worse than refusing.
    CHECK(run.written.empty());
}

TEST_CASE("the refusal names the subtitle that could not take it", "[cli][shifting]") {
    const Run run = shift(kTwo, -11'000);

    // Counted from one, as it is shown. Naming it is what tells the caller how
    // far they can actually go.
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 1"));
}

TEST_CASE("a shift landing exactly on the origin is allowed", "[cli][shifting]") {
    const Run run = shift(kTwo, -10'000);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:00,000 --> 00:00:02,000"));
}

TEST_CASE("the narration says what moved and by how much", "[cli][shifting]") {
    CHECK_THAT(shift(kTwo, 2'999).errors,
               ContainsSubstring("a.srt: 2 subtitles shifted by 2.999 s -> out/a.srt\n"));
}

TEST_CASE("a backward shift is narrated with its sign", "[cli][shifting]") {
    CHECK_THAT(shift(kTwo, -7'001).errors, ContainsSubstring("shifted by -7.001 s"));
}

TEST_CASE("a batch shifts what it can and counts the rest", "[cli][shifting]") {
    InMemoryFileSystem files;
    files.addFile("good.srt", kTwo);
    std::ostringstream errors;

    const ExitCode code = shiftAll(files,
                                   {"absent.srt", "good.srt"},
                                   std::nullopt,
                                   Duration::fromMilliseconds(1'000),
                                   Destination::from("", "out", false, 2).value(),
                                   Reporter{errors, 1});

    CHECK(code == ExitCode::SomeFailed);
    CHECK(files.contentOf("out/good.srt").has_value());
}

TEST_CASE("a file in no known format is refused by the shift too", "[cli][shifting]") {
    // The read failure of the rewrite path, told apart from a file that is
    // simply not there.
    InMemoryFileSystem files;
    files.addFile("a.srt", "nothing any reader claims\n");
    std::ostringstream errors;

    const ExitCode code = shiftAll(files,
                                   {"a.srt"},
                                   std::nullopt,
                                   Duration::fromMilliseconds(1'000),
                                   Destination::from("", "out", false, 1).value(),
                                   Reporter{errors, 0});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("is in no format this tool knows"));
}

TEST_CASE("a shift that cannot be written is reported and counted", "[cli][shifting]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);
    files.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    const ExitCode code = shiftAll(files,
                                   {"a.srt"},
                                   std::nullopt,
                                   Duration::fromMilliseconds(1'000),
                                   Destination::from("", "out", false, 1).value(),
                                   Reporter{errors, 0});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("permission denied"));
}

TEST_CASE("a backward shift under one second keeps its sign", "[cli][shifting]") {
    // "-0.500", not "0.500": the whole part is zero, so the sign has nowhere
    // else to show.
    CHECK_THAT(shift(kTwo, -500).errors, ContainsSubstring("shifted by -0.500 s"));
}

TEST_CASE("shifting onto the grid measures its own amount", "[cli][shifting]") {
    // A grid at 24 translated by 2999 milliseconds. The nearest grid line is
    // the next one, so the correction goes forwards by one millisecond rather
    // than back by almost a whole frame.
    InMemoryFileSystem files;
    files.addFile("a.srt", subedit::test::gridBytes("grille-24-decalee.srt"));
    std::ostringstream errors;

    const ExitCode code =
        subedit::cli::shiftOntoGridAll(files,
                                       {"a.srt"},
                                       std::nullopt,
                                       Destination::from("", "out", false, 1).value(),
                                       Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(errors.str(), ContainsSubstring(" shifted by 0.001 s onto their 24 fps grid"));
}

TEST_CASE("shifting onto the grid refuses a file that has none", "[cli][shifting]") {
    // 26.3 frames per second: regular, and none of the eight candidates. A
    // phase measured on noise would move the file by an arbitrary amount, which
    // is the silent failure this phase exists to avoid.
    InMemoryFileSystem files;
    files.addFile("a.srt", subedit::test::gridBytes("grille-absurde.srt"));
    std::ostringstream errors;

    const ExitCode code =
        subedit::cli::shiftOntoGridAll(files,
                                       {"a.srt"},
                                       std::nullopt,
                                       Destination::from("", "out", false, 1).value(),
                                       Reporter{errors, 1});

    CHECK(code != ExitCode::Success);
    CHECK_THAT(errors.str(), ContainsSubstring("no frame rate grid was found"));
    CHECK_FALSE(files.contentOf("out/a.srt").has_value());
}

namespace {

/// A file on a 25 fps grid offset by five milliseconds, whose **first** start
/// was moved by hand to sit nearer the origin than that offset.
///
/// It takes a partial file to reach this: on a clean one the smallest start is
/// itself a grid line plus the phase, so shifting back by the phase lands it
/// exactly on the origin and never before. Only a position somebody edited can
/// be closer to zero than the phase measured around it.
[[nodiscard]] std::string gridNearTheOrigin() {
    const auto stamp = [](std::int64_t ms) {
        std::ostringstream text;
        text << std::setfill('0') << std::setw(2) << ms / 3'600'000 << ':' << std::setw(2)
             << ms / 60'000 % 60 << ':' << std::setw(2) << ms / 1'000 % 60 << ',' << std::setw(3)
             << ms % 1'000;
        return text.str();
    };

    std::string content;
    for (int index = 1; index <= 20; ++index) {
        // The first cue is the edited one, at two milliseconds; the rest sit on
        // the grid, five milliseconds past their frame.
        const std::int64_t start = index == 1 ? 2 : (index * 2'000) + 5;
        content += std::to_string(index) + "\n" + stamp(start) + " --> " + stamp(start + 1'000) +
                   "\nLigne.\n\n";
    }
    return content;
}

} // namespace

TEST_CASE("a correction that would cross the origin is refused", "[cli][shifting]") {
    // The rule the core has held since #132, reached here by the one route that
    // can: the grid says to move back by five milliseconds, and one subtitle is
    // only two from the origin. A file cannot hold a negative timestamp, so the
    // whole operation is refused rather than half written.
    InMemoryFileSystem files;
    files.addFile("a.srt", gridNearTheOrigin());
    std::ostringstream errors;

    const ExitCode code =
        subedit::cli::shiftOntoGridAll(files,
                                       {"a.srt"},
                                       std::nullopt,
                                       Destination::from("", "out", false, 1).value(),
                                       Reporter{errors, 1});

    CHECK(code != ExitCode::Success);
    CHECK_THAT(errors.str(), ContainsSubstring("would start before the origin"));
    CHECK_FALSE(files.contentOf("out/a.srt").has_value());
}
