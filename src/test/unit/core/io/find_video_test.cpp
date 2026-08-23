// The naming convention of D5: it proposes a film, it never decides one.

#include <subedit/core/io/find_video.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>

namespace {

using subedit::core::findVideoBeside;
using subedit::core::InMemoryFileSystem;

/// A directory holding a subtitle file and whatever else the case needs.
[[nodiscard]] InMemoryFileSystem directoryHolding(std::initializer_list<const char*> names) {
    InMemoryFileSystem files;
    for (const char* name : names)
        files.addFile(std::filesystem::path{"/films"} / name, "");
    return files;
}

} // namespace

TEST_CASE("the film beside the subtitle file is found", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.fr.srt"});

    CHECK(findVideoBeside(files, "/films/film.fr.srt") == std::filesystem::path{"/films/film.mkv"});
}

TEST_CASE("a subtitle file named exactly like the film is served too", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.srt"});

    CHECK(findVideoBeside(files, "/films/film.srt") == std::filesystem::path{"/films/film.mkv"});
}

TEST_CASE("however many segments the subtitle name adds", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.en.forced.srt"});

    CHECK(findVideoBeside(files, "/films/film.en.forced.srt") ==
          std::filesystem::path{"/films/film.mkv"});
}

TEST_CASE("a directory with no film proposes nothing", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.fr.srt", "notes.txt", "film.vtt"});

    CHECK_FALSE(findVideoBeside(files, "/films/film.fr.srt").has_value());
}

// The prefix is read segment by segment, and not character by character.
// Gaupol compares raw strings, so `fil.mkv` matches `film.fr.srt` there; that
// is a film proposed for a subtitle file it has nothing to do with, and the
// user only finds out by watching the wrong picture.
TEST_CASE("a name that stops mid-segment is not a prefix", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"fil.mkv", "film.fr.srt"});

    CHECK_FALSE(findVideoBeside(files, "/films/film.fr.srt").has_value());
}

TEST_CASE("the longest matching name wins", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.fr.mkv", "film.fr.srt"});

    // `film.fr.srt` names `film.fr` before it names `film`: the closer of the
    // two is the one the user meant, and it is the only one that can be said
    // to have been meant at all.
    CHECK(findVideoBeside(files, "/films/film.fr.srt") ==
          std::filesystem::path{"/films/film.fr.mkv"});
}

// The case the issue asked to settle, and the rule Gaupol does not carry: two
// films answer to the same name, and nothing distinguishes them. Taking the
// first the file system lists is not an answer — that order is not stable —
// and taking the first of our extension list would be an answer nobody can
// read. So the convention keeps quiet, and D5 does the rest: the user chooses,
// once, and their choice is never overwritten.
TEST_CASE("two films of the same name propose nothing", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.mp4", "film.fr.srt"});

    CHECK_FALSE(findVideoBeside(files, "/films/film.fr.srt").has_value());
}

TEST_CASE("ambiguity at one level does not hide a better match", "[video][filesystem]") {
    const InMemoryFileSystem files =
        directoryHolding({"film.mkv", "film.mp4", "film.fr.mkv", "film.fr.srt"});

    CHECK(findVideoBeside(files, "/films/film.fr.srt") ==
          std::filesystem::path{"/films/film.fr.mkv"});
}

// A subtitle file whose own directory cannot be read — taken away, or refused
// — proposes nothing. The convention has no opinion to offer about a place it
// cannot look at, and saying so is not the same as saying there is no film.
TEST_CASE("a directory that cannot be listed proposes nothing", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.fr.srt"});

    CHECK_FALSE(findVideoBeside(files, "/ailleurs/film.fr.srt").has_value());
}

// A directory handed in where a subtitle file was expected names no film, and
// says so rather than reading the first thing it finds.
TEST_CASE("a path naming no file proposes nothing", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.mkv", "film.fr.srt"});

    CHECK_FALSE(findVideoBeside(files, "/films/").has_value());
}

TEST_CASE("the extension is recognised whatever its case", "[video][filesystem]") {
    const InMemoryFileSystem files = directoryHolding({"film.MKV", "film.fr.srt"});

    CHECK(findVideoBeside(files, "/films/film.fr.srt") == std::filesystem::path{"/films/film.MKV"});
}

TEST_CASE("only the directory of the subtitle file is looked at", "[video][filesystem]") {
    InMemoryFileSystem files;
    files.addFile("/films/film.fr.srt", "");
    files.addFile("/ailleurs/film.mkv", "");

    CHECK_FALSE(findVideoBeside(files, "/films/film.fr.srt").has_value());
}

// A subtitle file named without a directory is in the current one, and the
// convention has no reason to refuse it. In memory there is no current
// directory, so « no parent » *is* it — which is the same statement.
TEST_CASE("a subtitle file named without a directory is served", "[video][filesystem]") {
    InMemoryFileSystem files;
    files.addFile("film.mkv", "");
    files.addFile("film.fr.srt", "");

    CHECK(findVideoBeside(files, "film.fr.srt") == std::filesystem::path{"film.mkv"});
}
