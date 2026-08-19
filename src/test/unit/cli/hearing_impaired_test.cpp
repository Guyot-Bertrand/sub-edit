// The subcommand, on an in-memory file system.
//
// The rule is settled by `mentions.cas` and the command by its own unit tests;
// what is proved here is the layer between them and a file — the report it
// words, and the fact that a file with nothing to clean is written all the
// same.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::removeHearingImpairedIn;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;

namespace {

/// Four subtitles: one emptied, one rewritten, one reference left alone, one
/// untouched.
const std::string kMentions = "1\n"
                              "00:00:01,000 --> 00:00:03,000\n"
                              "[Bruit de pas]\n"
                              "\n"
                              "2\n"
                              "00:00:04,000 --> 00:00:06,000\n"
                              "Attends [il tousse] Marie.\n"
                              "\n"
                              "3\n"
                              "00:00:07,000 --> 00:00:09,000\n"
                              "Voir [1] la note.\n"
                              "\n"
                              "4\n"
                              "00:00:10,000 --> 00:00:12,000\n"
                              "Rien à signaler.\n"
                              "\n";

const std::string kNothingToClean = "1\n"
                                    "00:00:01,000 --> 00:00:03,000\n"
                                    "Rien à signaler.\n"
                                    "\n";

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

Run clean(const std::string& content) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    std::ostringstream errors;

    const ExitCode code = removeHearingImpairedIn(
        files, {"a.srt"}, Destination::from("", "out", false, 1).value(), Reporter{errors, 1});
    return {
        .code = code, .written = files.contentOf("out/a.srt").value_or(""), .errors = errors.str()};
}

} // namespace

TEST_CASE("cleaning takes the mentions out and leaves the references", "[cli][hearing]") {
    const Run run = clean(kMentions);

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.written, ContainsSubstring("Attends Marie."));
    CHECK_THAT(run.written, ContainsSubstring("Voir [1] la note."));
    CHECK_THAT(run.written, !ContainsSubstring("Bruit de pas"));
}

TEST_CASE("a subtitle the cleaning empties leaves the file", "[cli][hearing]") {
    const Run run = clean(kMentions);

    // Three of the four survive, renumbered from one.
    CHECK_THAT(run.written, ContainsSubstring("3\n00:00:10,000"));
    CHECK_THAT(run.written, !ContainsSubstring("4\n"));
}

TEST_CASE("the report counts what changed and what went", "[cli][hearing]") {
    const Run run = clean(kMentions);

    CHECK_THAT(run.errors, ContainsSubstring("1 subtitle cleaned, 1 removed"));
}

TEST_CASE("a file with nothing to clean is written all the same", "[cli][hearing]") {
    // The rule of every subcommand that writes: a destination given is a
    // destination written. Making this the exception would force a script to
    // know which subcommands sometimes produce a file and sometimes nothing.
    const Run run = clean(kNothingToClean);

    CHECK(run.code == ExitCode::Success);
    CHECK(run.written == kNothingToClean);
    CHECK_THAT(run.errors, ContainsSubstring("no mention to remove"));
}

TEST_CASE("a file that cannot be read is named, and the code says so", "[cli][hearing]") {
    InMemoryFileSystem files;
    std::ostringstream errors;

    const ExitCode code = removeHearingImpairedIn(
        files, {"absent.srt"}, Destination::from("", "out", false, 1).value(), Reporter{errors, 1});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("absent.srt"));
}
