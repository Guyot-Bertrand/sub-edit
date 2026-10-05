// The append of several files into one, on an in-memory file system.

#include <subedit/cli/appending.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::appendAll;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;

namespace {

const std::string kOne = "1\n00:00:01,000 --> 00:00:02,000\none\n\n";

} // namespace

TEST_CASE("each file is shifted from the end of what precedes it, not from the base's",
          "[cli][appending][CLI-APPEND-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    files.addFile("b.srt", kOne);
    files.addFile("c.srt", kOne);
    std::ostringstream errors;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "b.srt", "c.srt"},
                                    std::nullopt,
                                    Destination::from("all.srt", "", false, 1).value(),
                                    Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    // The second starts where the first ends (2 s), the third where the second does (4 s).
    CHECK(files.contentOf("all.srt").value_or("") == "1\n00:00:01,000 --> 00:00:02,000\none\n\n"
                                                     "2\n00:00:03,000 --> 00:00:04,000\none\n\n"
                                                     "3\n00:00:05,000 --> 00:00:06,000\none\n\n");
    CHECK_THAT(errors.str(), ContainsSubstring("3 subtitles from 3 files -> all.srt"));
}

TEST_CASE("the record carries the totals and one entry per appended file",
          "[cli][appending][CLI-APPEND-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    files.addFile("b.srt", kOne);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "b.srt"},
                                    std::nullopt,
                                    Destination::from("all.srt", "", false, 1).value(),
                                    Reporter{errors, 1}.withRecords(records).forCommand("append"));

    CHECK(code == ExitCode::Success);
    CHECK_THAT(records.str(),
               ContainsSubstring("\"counts\":{\"files\":2,\"subtitles\":2,\"appended\":1,"));
    CHECK_THAT(records.str(),
               ContainsSubstring("\"inputs\":[{\"file\":\"b.srt\",\"counts\":{\"appended\":1,"));
}

TEST_CASE("a file that fails stops the run before anything is written",
          "[cli][appending][CLI-APPEND-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    files.addFile("c.srt", kOne);
    std::ostringstream errors;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "missing.srt", "c.srt"},
                                    std::nullopt,
                                    Destination::from("all.srt", "", false, 1).value(),
                                    Reporter{errors, 1});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(), ContainsSubstring("missing.srt"));
    CHECK_FALSE(files.contentOf("all.srt").has_value());
}

TEST_CASE("a destination that is an input is a usage error", "[cli][appending][CLI-APPEND-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    files.addFile("b.srt", kOne);
    std::ostringstream errors;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "b.srt"},
                                    std::nullopt,
                                    Destination::from("b.srt", "", false, 1).value(),
                                    Reporter{errors, 1});

    CHECK(code == ExitCode::Usage);
    CHECK(files.contentOf("b.srt").value_or("") == kOne);
}

TEST_CASE("a dry run works the result out and writes nothing", "[cli][appending][CLI-DRYRUN-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kOne);
    files.addFile("b.srt", kOne);
    std::ostringstream errors;

    const ExitCode code = appendAll(files,
                                    {"a.srt", "b.srt"},
                                    std::nullopt,
                                    Destination::from("", "", false, 1, true).value(),
                                    Reporter{errors, 1});

    CHECK(code == ExitCode::Success);
    CHECK_THAT(errors.str(), ContainsSubstring("(dry run, nothing written)"));
}

TEST_CASE("a base that cannot be read, a directory that cannot be made and a disk that refuses "
          "all fail the run",
          "[cli][appending][CLI-APPEND-01]") {
    std::ostringstream errors;

    InMemoryFileSystem noBase;
    noBase.addFile("b.srt", kOne);
    CHECK(appendAll(noBase,
                    {"missing.srt", "b.srt"},
                    std::nullopt,
                    Destination::from("all.srt", "", false, 1).value(),
                    Reporter{errors, 1}) == ExitCode::AllFailed);

    InMemoryFileSystem noDirectory;
    noDirectory.addFile("a.srt", kOne);
    noDirectory.addFile("b.srt", kOne);
    noDirectory.failNextCreateDirectories(subedit::core::FileErrorKind::PermissionDenied);
    CHECK(appendAll(noDirectory,
                    {"a.srt", "b.srt"},
                    std::nullopt,
                    Destination::from("out/all.srt", "", false, 1).value(),
                    Reporter{errors, 1}) == ExitCode::AllFailed);
    CHECK_FALSE(noDirectory.contentOf("out/all.srt").has_value());

    InMemoryFileSystem noDisk;
    noDisk.addFile("a.srt", kOne);
    noDisk.addFile("b.srt", kOne);
    noDisk.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);
    CHECK(appendAll(noDisk,
                    {"a.srt", "b.srt"},
                    std::nullopt,
                    Destination::from("all.srt", "", false, 1).value(),
                    Reporter{errors, 1}) == ExitCode::AllFailed);
    CHECK_FALSE(noDisk.contentOf("all.srt").has_value());
}
