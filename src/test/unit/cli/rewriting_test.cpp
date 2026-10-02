// The loop of a batch that writes: what it hands an operation, and what it
// does when `--range` does not fit a file.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/selection.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::ChangingOperation;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::OperationOutcome;
using subedit::cli::OperationResult;
using subedit::cli::Range;
using subedit::cli::Reporter;
using subedit::cli::Request;
using subedit::cli::rewriteAll;
using subedit::core::IndexRange;
using subedit::core::InMemoryFileSystem;
using subedit::core::SubtitleIndex;

namespace {

/// A SubRip file of `count` subtitles.
std::string srt(int count) {
    std::string text;
    for (int i = 1; i <= count; ++i) {
        const std::string second = std::to_string(i);
        text += second;
        text += "\n00:00:0";
        text += second;
        text += ",000 --> 00:00:0";
        text += second;
        text += ",500\nLine ";
        text += second;
        text += "\n\n";
    }
    return text;
}

/// An operation that changes nothing and records what it was handed.
struct Spy {
    std::vector<std::vector<IndexRange>> selections;

    [[nodiscard]] ChangingOperation operation() {
        return [this](subedit::core::Session&, const Request& request) -> OperationOutcome {
            const auto ranges = request.selection.ranges();
            selections.emplace_back(ranges.begin(), ranges.end());
            return OperationResult{.sentence = "looked at", .counts = {}};
        };
    }
};

IndexRange runOf(std::size_t first, std::size_t last) {
    return {.first = SubtitleIndex::fromValue(first), .last = SubtitleIndex::fromValue(last)};
}

} // namespace

TEST_CASE("an operation is handed the whole file when no range is given",
          "[cli][rewriting][CLI-RANGE-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", srt(4));
    std::ostringstream errors;
    Spy spy;

    const ExitCode code = rewriteAll(files,
                                     {"a.srt"},
                                     std::nullopt,
                                     Destination::from("", "out", false, 1).value(),
                                     Reporter{errors, 1},
                                     "looked at",
                                     spy.operation());

    CHECK(code == ExitCode::Success);
    REQUIRE(spy.selections.size() == 1);
    CHECK(spy.selections.front() == std::vector<IndexRange>{runOf(0, 3)});
}

TEST_CASE("an operation is handed what the range names, from one",
          "[cli][rewriting][CLI-RANGE-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", srt(5));
    std::ostringstream errors;
    Spy spy;

    const ExitCode code = rewriteAll(files,
                                     {"a.srt"},
                                     std::nullopt,
                                     Destination::from("", "out", false, 1).value(),
                                     Reporter{errors, 1},
                                     "looked at",
                                     spy.operation(),
                                     Range{.first = 2, .last = 4});

    CHECK(code == ExitCode::Success);
    REQUIRE(spy.selections.size() == 1);
    CHECK(spy.selections.front() == std::vector<IndexRange>{runOf(1, 3)});
}

TEST_CASE("a range holds for every file of the batch, each against its own length",
          "[cli][rewriting][CLI-RANGE-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", srt(5));
    files.addFile("b.srt", srt(9));
    std::ostringstream errors;
    Spy spy;

    const ExitCode code = rewriteAll(files,
                                     {"a.srt", "b.srt"},
                                     std::nullopt,
                                     Destination::from("", "out", false, 2).value(),
                                     Reporter{errors, 1},
                                     "looked at",
                                     spy.operation(),
                                     Range{.first = 3, .last = std::nullopt});

    CHECK(code == ExitCode::Success);
    REQUIRE(spy.selections.size() == 2);
    CHECK(spy.selections[0] == std::vector<IndexRange>{runOf(2, 4)});
    CHECK(spy.selections[1] == std::vector<IndexRange>{runOf(2, 8)});
}

TEST_CASE("a range a file does not hold fails that file before the operation, and only it",
          "[cli][rewriting][CLI-RANGE-02]") {
    InMemoryFileSystem files;
    files.addFile("long.srt", srt(9));
    files.addFile("short.srt", srt(3));
    std::ostringstream errors;
    Spy spy;

    const ExitCode code = rewriteAll(files,
                                     {"short.srt", "long.srt"},
                                     std::nullopt,
                                     Destination::from("", "out", false, 2).value(),
                                     Reporter{errors, 0},
                                     "looked at",
                                     spy.operation(),
                                     Range{.first = 2, .last = 6});

    CHECK(code == ExitCode::SomeFailed);
    // The operation never saw the short file, and nothing was written for it.
    CHECK(spy.selections.size() == 1);
    CHECK_FALSE(files.contentOf("out/short.srt").has_value());
    CHECK(files.contentOf("out/long.srt").has_value());
    // Said even quietly, naming the file and the bound it does not have.
    CHECK_THAT(
        errors.str(),
        ContainsSubstring("short.srt: range 2-6 ends after the last subtitle: the file has 3"));
}

TEST_CASE("a range a file does not hold is a failure of kind range-out-of-bounds in json",
          "[cli][rewriting][CLI-RANGE-02]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", srt(3));
    std::ostringstream errors;
    std::ostringstream records;
    Spy spy;

    CHECK(rewriteAll(files,
                     {"a.srt"},
                     std::nullopt,
                     Destination::from("", "out", false, 1).value(),
                     Reporter{errors, 1}.withRecords(records).forCommand("test"),
                     "looked at",
                     spy.operation(),
                     Range{.first = 4, .last = std::nullopt}) == ExitCode::AllFailed);

    CHECK_THAT(records.str(), ContainsSubstring("\"ok\":false"));
    CHECK_THAT(records.str(), ContainsSubstring("\"kind\":\"range-out-of-bounds\""));
    CHECK(spy.selections.empty());
}
