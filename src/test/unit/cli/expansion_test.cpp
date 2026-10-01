#include <subedit/cli/destination.hpp>
#include <subedit/cli/expansion.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::cli::expandInputs;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;

namespace {

InMemoryFileSystem tree() {
    InMemoryFileSystem files;
    for (const char* path : {"films/b.srt",
                             "films/a.srt",
                             "films/B.vtt",
                             "films/a/z.ass",
                             "films/a/y.lrc",
                             "films/a/deep/w.sub",
                             "films/a/deep/v.ssa",
                             "films/README.txt",
                             "films/a/notes.dat",
                             "films/.hidden.srt",
                             "films/.cache/x.srt",
                             "films/out/old.srt"})
        files.addFile(path, "x");
    return files;
}

} // namespace

TEST_CASE("a file named is itself, whatever its extension", "[cli][expansion][CLI-BATCH-12]") {
    InMemoryFileSystem files;
    files.addFile("notes.txt", "x");
    std::ostringstream errors;

    const auto inputs =
        expandInputs(files, {"notes.txt", "absent.dat"}, false, "", Reporter{errors, 2});

    REQUIRE(inputs.has_value());
    CHECK(inputs->paths == std::vector<std::string>{"notes.txt", "absent.dat"});
    CHECK(inputs->roots.empty());
}

TEST_CASE("a directory without --recursive is refused", "[cli][expansion][CLI-BATCH-08]") {
    const InMemoryFileSystem files = tree();
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"films"}, false, "", Reporter{errors, 1});

    REQUIRE_FALSE(inputs.has_value());
    CHECK(inputs.error() == "films: is a directory: use --recursive");
}

TEST_CASE("a walk is sorted by bytes, depth first, hidden and unknown left out",
          "[cli][expansion][CLI-BATCH-09]") {
    const InMemoryFileSystem files = tree();
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"films"}, true, "", Reporter{errors, 2});

    REQUIRE(inputs.has_value());
    CHECK(inputs->paths == std::vector<std::string>{"films/B.vtt",
                                                    "films/a/deep/v.ssa",
                                                    "films/a/deep/w.sub",
                                                    "films/a/y.lrc",
                                                    "films/a/z.ass",
                                                    "films/a.srt",
                                                    "films/b.srt",
                                                    "films/out/old.srt"});
    REQUIRE(inputs->roots.size() == 1);
    CHECK(inputs->roots[0] == "films");
    // README.txt and notes.dat.
    CHECK_THAT(errors.str(), ContainsSubstring("films: 2 files left aside"));
}

TEST_CASE("the output directory is not walked", "[cli][expansion][CLI-BATCH-11]") {
    const InMemoryFileSystem files = tree();
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"films"}, true, "films/out", Reporter{errors, 1});

    REQUIRE(inputs.has_value());
    for (const std::string& path : inputs->paths)
        CHECK_FALSE(path.starts_with("films/out/"));
    CHECK(inputs->paths.size() == 7);
}

TEST_CASE("the extension of a walked file is read without regard to case",
          "[cli][expansion][CLI-BATCH-09]") {
    InMemoryFileSystem files;
    files.addFile("d/LOUD.SRT", "x");
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"d"}, true, "", Reporter{errors, 1});

    REQUIRE(inputs.has_value());
    CHECK(inputs->paths == std::vector<std::string>{"d/LOUD.SRT"});
}

TEST_CASE("a directory with nothing to walk says so at level 1", "[cli][expansion][CLI-BATCH-09]") {
    InMemoryFileSystem files;
    files.addFile("d/README.txt", "x");
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"d"}, true, "", Reporter{errors, 1});

    REQUIRE(inputs.has_value());
    CHECK(inputs->paths.empty());
    CHECK_THAT(errors.str(), ContainsSubstring("d: holds no file in a format this tool walks for"));
}

TEST_CASE("what is found under a root keeps its place under --output-dir",
          "[cli][expansion][CLI-BATCH-10]") {
    const InMemoryFileSystem files = tree();
    std::ostringstream errors;
    const auto inputs = expandInputs(files, {"films"}, true, "", Reporter{errors, 1});
    REQUIRE(inputs.has_value());

    const Destination destination =
        Destination::from("", "out", false, 8)->withRoots(inputs->roots);

    CHECK(destination.pathFor("films/a/deep/w.sub", "") == "out/a/deep/w.sub");
    CHECK(destination.pathFor("films/b.srt", ".vtt") == "out/b.vtt");
    // A file given by name, outside every root, keeps only its name.
    CHECK(destination.pathFor("elsewhere/c.srt", "") == "out/c.srt");
}

TEST_CASE("a link is not followed, and is not left aside either",
          "[cli][expansion][CLI-BATCH-09]") {
    InMemoryFileSystem files;
    files.addFile("d/a.srt", "x");
    files.addLink("d/link.srt");
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"d"}, true, "", Reporter{errors, 2});

    REQUIRE(inputs.has_value());
    CHECK(inputs->paths == std::vector<std::string>{"d/a.srt"});
    CHECK_THAT(errors.str(), !ContainsSubstring("left aside"));
}

TEST_CASE("a directory that cannot be listed stops the walk and is named",
          "[cli][expansion][CLI-BATCH-09]") {
    InMemoryFileSystem files;
    files.addFile("d/a.srt", "x");
    files.addFile("d/sub/b.srt", "x");
    files.failEntriesOf("d/sub", subedit::core::FileErrorKind::PermissionDenied);
    std::ostringstream errors;

    const auto inputs = expandInputs(files, {"d"}, true, "", Reporter{errors, 1});

    REQUIRE_FALSE(inputs.has_value());
    // The one that refused, and why, in the words of a file that cannot be read.
    CHECK(inputs.error() == "d/sub: cannot be opened: permission denied");
}
