// The cut of a file in two, on an in-memory file system.

#include <subedit/cli/reporter.hpp>
#include <subedit/cli/splitting.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::ExitCode;
using subedit::cli::Reporter;
using subedit::cli::splitFile;
using subedit::cli::SplitRequest;
using subedit::core::InMemoryFileSystem;

namespace {

const std::string kThree = "1\n00:00:01,000 --> 00:00:02,000\none\n\n"
                           "2\n00:00:03,000 --> 00:00:04,000\ntwo\n\n"
                           "3\n00:00:05,000 --> 00:00:06,000\nthree\n\n";

SplitRequest cutAt(std::size_t at) {
    return SplitRequest{.input = "a.srt", .at = at, .head = "out/h.srt", .tail = "out/t.srt"};
}

} // namespace

TEST_CASE("the tail is brought back by the end of the head, and both halves are written",
          "[cli][splitting][CLI-PSPLIT-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kThree);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code =
        splitFile(files,
                  cutAt(3),
                  std::nullopt,
                  Reporter{errors, 1}.withRecords(records).forCommand("split-file"));

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/h.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:02,000\none\n\n2\n00:00:03,000 --> 00:00:04,000\ntwo\n\n");
    // Five seconds less the four the head ends at.
    CHECK(files.contentOf("out/t.srt").value_or("") ==
          "1\n00:00:01,000 --> 00:00:02,000\nthree\n\n");
    CHECK_THAT(records.str(),
               ContainsSubstring("\"counts\":{\"subtitles\":3,\"head\":2,\"tail\":1}"));
    CHECK_THAT(records.str(), ContainsSubstring("\"tail\":\"out/t.srt\""));
}

TEST_CASE("a cut that is not between two subtitles is refused", "[cli][splitting][CLI-PSPLIT-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kThree);
    files.addFile("one.srt", "1\n00:00:01,000 --> 00:00:02,000\none\n\n");
    std::ostringstream errors;

    CHECK(splitFile(files, cutAt(1), std::nullopt, Reporter{errors, 1}) == ExitCode::AllFailed);
    CHECK(splitFile(files, cutAt(4), std::nullopt, Reporter{errors, 1}) == ExitCode::AllFailed);
    SplitRequest lone = cutAt(2);
    lone.input = "one.srt";
    CHECK(splitFile(files, lone, std::nullopt, Reporter{errors, 1}) == ExitCode::AllFailed);
    CHECK_FALSE(files.contentOf("out/h.srt").has_value());
    CHECK_FALSE(files.contentOf("out/t.srt").has_value());
}

TEST_CASE("a head or a tail that is the input, or that are one file, is a usage error",
          "[cli][splitting][CLI-PSPLIT-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kThree);
    std::ostringstream errors;

    SplitRequest overInput = cutAt(2);
    overInput.tail = "a.srt";
    SplitRequest same = cutAt(2);
    same.tail = same.head;

    CHECK(splitFile(files, overInput, std::nullopt, Reporter{errors, 1}) == ExitCode::Usage);
    CHECK(splitFile(files, same, std::nullopt, Reporter{errors, 1}) == ExitCode::Usage);
    CHECK(files.contentOf("a.srt").value_or("") == kThree);
}

TEST_CASE("a file that cannot be read, a directory that cannot be made and a disk that refuses "
          "all fail the run",
          "[cli][splitting][CLI-PSPLIT-01]") {
    std::ostringstream errors;

    InMemoryFileSystem noInput;
    CHECK(splitFile(noInput, cutAt(2), std::nullopt, Reporter{errors, 1}) == ExitCode::AllFailed);

    InMemoryFileSystem noDirectory;
    noDirectory.addFile("a.srt", kThree);
    noDirectory.failNextCreateDirectories(subedit::core::FileErrorKind::PermissionDenied);
    CHECK(splitFile(noDirectory, cutAt(2), std::nullopt, Reporter{errors, 1}) ==
          ExitCode::AllFailed);
    CHECK_FALSE(noDirectory.contentOf("out/h.srt").has_value());

    InMemoryFileSystem noDisk;
    noDisk.addFile("a.srt", kThree);
    noDisk.failNextWrite(subedit::core::FileErrorKind::PermissionDenied);
    CHECK(splitFile(noDisk, cutAt(2), std::nullopt, Reporter{errors, 1}) == ExitCode::AllFailed);
}

TEST_CASE("a dry run works the cut out and writes nothing", "[cli][splitting][CLI-DRYRUN-01]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kThree);
    std::ostringstream errors;
    SplitRequest request = cutAt(2);
    request.head.clear();
    request.tail.clear();
    request.dryRun = true;

    CHECK(splitFile(files, request, std::nullopt, Reporter{errors, 1}) == ExitCode::Success);
    CHECK_THAT(errors.str(), ContainsSubstring("(dry run, nothing written)"));
}

TEST_CASE("two halves that overlap are refused, and nothing is written",
          "[cli][splitting][CLI-PSPLIT-01]") {
    InMemoryFileSystem files;
    // The second subtitle ends at six seconds, after the third starts.
    files.addFile("a.srt",
                  "1\n00:00:00,000 --> 00:00:10,000\na\n\n"
                  "2\n00:00:05,000 --> 00:00:06,000\nb\n\n"
                  "3\n00:00:05,500 --> 00:00:06,000\nc\n\n");
    std::ostringstream errors;

    const ExitCode code = splitFile(files, cutAt(3), std::nullopt, Reporter{errors, 1});

    CHECK(code == ExitCode::AllFailed);
    CHECK_THAT(errors.str(),
               ContainsSubstring("Cannot split at subtitle 3: subtitle 3 would fall"));
    CHECK_FALSE(files.contentOf("out/h.srt").has_value());
}

TEST_CASE("sorting on request cuts on the time order, and the cut counts in it",
          "[cli][splitting][CLI-SORT-03]") {
    InMemoryFileSystem files;
    files.addFile("a.srt",
                  "1\n00:00:05,000 --> 00:00:06,000\nlast\n\n"
                  "2\n00:00:01,000 --> 00:00:02,000\nfirst\n\n"
                  "3\n00:00:03,000 --> 00:00:04,000\nsecond\n\n");
    std::ostringstream errors;
    std::ostringstream records;
    SplitRequest request = cutAt(3);
    request.sort = true;

    const ExitCode code =
        splitFile(files,
                  request,
                  std::nullopt,
                  Reporter{errors, 1}.withRecords(records).forCommand("split-file"));

    CHECK(code == ExitCode::Success);
    // In time order the head is « first » and « second », the tail « last ».
    CHECK_THAT(files.contentOf("out/h.srt").value_or(""), ContainsSubstring("first"));
    CHECK_THAT(files.contentOf("out/h.srt").value_or(""), ContainsSubstring("second"));
    CHECK_THAT(files.contentOf("out/t.srt").value_or(""), ContainsSubstring("last"));
    CHECK_THAT(errors.str(), ContainsSubstring("a.srt: 3 subtitles moved"));
    CHECK_THAT(records.str(), ContainsSubstring("\"moved\":3"));
}
