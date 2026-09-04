#include <subedit/cli/destination.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/transforming.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reference;
using subedit::cli::Reporter;
using subedit::cli::Transform;
using subedit::cli::transformAll;
using subedit::core::InMemoryFileSystem;
using subedit::core::Timestamp;

namespace {

/// Three subtitles, the middle one starting on an odd millisecond so that the
/// scaling has something to round.
const std::string kThree = "1\n"
                           "00:00:01,000 --> 00:00:03,000\n"
                           "First.\n"
                           "\n"
                           "2\n"
                           "00:00:05,001 --> 00:00:07,000\n"
                           "Second.\n"
                           "\n"
                           "3\n"
                           "00:00:09,000 --> 00:00:11,000\n"
                           "Third.\n"
                           "\n";

Reference at(std::size_t number, std::int64_t milliseconds) {
    return Reference{.number = number, .target = Timestamp::fromMilliseconds(milliseconds)};
}

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

Run transform(const std::string& content, Reference first, Reference last) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream errors;

    const ExitCode code = transformAll(files,
                                       {"a.srt"},
                                       std::nullopt,
                                       Transform::between(first, last).value(),
                                       Destination::from("", "out", false, 1).value(),
                                       Reporter{errors, 1});
    return {
        .code = code, .written = files.contentOf("out/a.srt").value_or(""), .errors = errors.str()};
}

} // namespace

TEST_CASE("two references define a transform", "[cli][transforming]") {
    CHECK(Transform::between(at(1, 1'000), at(3, 10'000)).has_value());
    // Which is named first does not matter: they are two points, not a range.
    CHECK(Transform::between(at(3, 10'000), at(1, 1'000)).has_value());
}

TEST_CASE("two references on one subtitle define none", "[cli][transforming]") {
    const auto refused = Transform::between(at(2, 1'000), at(2, 4'000));

    REQUIRE_FALSE(refused.has_value());
    // Naming it is what tells the caller which of the two options to change.
    CHECK_THAT(refused.error(), ContainsSubstring("subtitle 2"));
}

TEST_CASE("each reference lands exactly on the position asked for", "[cli][transforming]") {
    // The whole point of the form of ADR 0013: the two references are exact,
    // not merely close.
    const Run run = transform(kThree, at(1, 1'000), at(3, 10'000));

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("00:00:01,000 --> "));
    CHECK_THAT(run.written, ContainsSubstring("00:00:10,000 --> "));
}

TEST_CASE("the positions between the references follow", "[cli][transforming]") {
    // Worked out by hand. r = (10000 - 1000) / (9000 - 1000) = 9/8, and
    // t' = 1000 + (t - 1000) × 9/8:
    //   5001 -> 1000 + 4001 × 9/8 = 1000 + 4501.125 -> 5501
    //   7000 -> 1000 + 6000 × 9/8 = 7750
    const Run run = transform(kThree, at(1, 1'000), at(3, 10'000));

    CHECK_THAT(run.written, ContainsSubstring("00:00:05,501 --> 00:00:07,750"));
}

TEST_CASE("the positions outside the references follow too", "[cli][transforming]") {
    // Nothing stops at the second reference: the end of the third subtitle,
    // 11000, lands at 1000 + 10000 × 9/8 = 12250.
    const Run run = transform(kThree, at(1, 1'000), at(3, 10'000));

    CHECK_THAT(run.written, ContainsSubstring("00:00:10,000 --> 00:00:12,250"));
    // And the end of the first, 3000, at 1000 + 2000 × 9/8 = 3250.
    CHECK_THAT(run.written, ContainsSubstring("00:00:01,000 --> 00:00:03,250"));
}

TEST_CASE("a subtitle past the end is refused, naming the bound", "[cli][transforming]") {
    const Run run = transform(kThree, at(1, 1'000), at(9, 10'000));

    CHECK(run.code == ExitCode::AllFailed);
    CHECK(run.written.empty());
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 9"));
    // The bound, so that the caller knows what they may write instead.
    CHECK_THAT(run.errors, ContainsSubstring("3 subtitles"));
}

TEST_CASE("the first reference is held to the bound as well as the last", "[cli][transforming]") {
    // Both are checked, and separately: whichever is wrong is the one named.
    const Run run = transform(kThree, at(7, 1'000), at(3, 10'000));

    CHECK(run.code == ExitCode::AllFailed);
    CHECK(run.written.empty());
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 7"));
    CHECK_THAT(run.errors, ContainsSubstring("3 subtitles"));
}

TEST_CASE("two subtitles starting together define no transform either", "[cli][transforming]") {
    // Two distinct indices, one position: the denominator is still zero. The
    // file is what tells them apart, so this refusal cannot come earlier.
    const std::string together = "1\n"
                                 "00:00:01,000 --> 00:00:03,000\n"
                                 "First.\n"
                                 "\n"
                                 "2\n"
                                 "00:00:01,000 --> 00:00:04,000\n"
                                 "At the same moment.\n"
                                 "\n";
    const Run run = transform(together, at(1, 2'000), at(2, 5'000));

    CHECK(run.code == ExitCode::AllFailed);
    CHECK_THAT(run.errors, ContainsSubstring("same moment"));
}

TEST_CASE("a transform that would go before the origin is refused", "[cli][transforming]") {
    // r = (1000 - 100) / (9000 - 5001) = 300/1333, and the first subtitle sits
    // before the first reference: 100 + (1000 - 5001) × 300/1333 = -800.
    const Run run = transform(kThree, at(2, 100), at(3, 1'000));

    CHECK(run.code == ExitCode::AllFailed);
    CHECK(run.written.empty());
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 1"));
    CHECK_THAT(run.errors, ContainsSubstring("before the origin"));
}

TEST_CASE("the format of the file read is kept", "[cli][transforming]") {
    InMemoryFileSystem files;
    files.addFile(
        "a.vtt", "WEBVTT\n\n00:01.000 --> 00:03.000\nFirst.\n\n00:05.000 --> 00:07.000\nSecond.\n");
    std::ostringstream errors;

    CHECK(transformAll(files,
                       {"a.vtt"},
                       std::nullopt,
                       Transform::between(at(1, 2'000), at(2, 10'000)).value(),
                       Destination::from("", "out", false, 1).value(),
                       Reporter{errors, 0}) == ExitCode::Success);

    CHECK_THAT(files.contentOf("out/a.vtt").value_or(""), ContainsSubstring("WEBVTT"));
}

TEST_CASE("the narration says what moved and onto what", "[cli][transforming]") {
    CHECK_THAT(transform(kThree, at(1, 1'000), at(3, 10'000)).errors,
               ContainsSubstring("a.srt: 3 subtitles transformed onto 1=00:00:01.000 and "
                                 "3=00:00:10.000 -> out/a.srt\n"));
}

TEST_CASE("a batch transforms what it can and counts the rest", "[cli][transforming]") {
    InMemoryFileSystem files;
    files.addFile("good.srt", kThree);
    std::ostringstream errors;

    const ExitCode code = transformAll(files,
                                       {"absent.srt", "good.srt"},
                                       std::nullopt,
                                       Transform::between(at(1, 1'000), at(3, 10'000)).value(),
                                       Destination::from("", "out", false, 2).value(),
                                       Reporter{errors, 1});

    CHECK(code == ExitCode::SomeFailed);
    CHECK(files.contentOf("out/good.srt").has_value());
}
