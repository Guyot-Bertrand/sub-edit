#include <subedit/core/model/video_file.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

namespace {

using subedit::core::isVideoFile;

} // namespace

TEST_CASE("the extensions of the closed list are video files", "[video][model]") {
    CHECK(isVideoFile("film.mkv"));
    CHECK(isVideoFile("film.mp4"));
    CHECK(isVideoFile("film.avi"));
    CHECK(isVideoFile("film.webm"));
    CHECK(isVideoFile("/ailleurs/film.m2ts"));
}

TEST_CASE("anything else is not a video file", "[video][model]") {
    CHECK_FALSE(isVideoFile("film.srt"));
    CHECK_FALSE(isVideoFile("film.vtt"));
    CHECK_FALSE(isVideoFile("notes.txt"));
    CHECK_FALSE(isVideoFile("film.mkv.part"));
}

// A file named after a video extension and nothing else is a hidden file on
// Unix, not an extension: `.mkv` alone names a file whose whole name is
// `.mkv`, and `std::filesystem` says its extension is empty. Answering yes
// would offer a dotfile as a film.
TEST_CASE("a bare extension is a name, not an extension", "[video][model]") {
    CHECK_FALSE(isVideoFile(".mkv"));
}

TEST_CASE("a file with no extension at all is not a video file", "[video][model]") {
    CHECK_FALSE(isVideoFile("film"));
    CHECK_FALSE(isVideoFile(""));
}

// Files that came from elsewhere carry the case of elsewhere. Gaupol lowers
// the suffix before comparing, and so do we — on the extension only, the rest
// of the name being the user's business.
TEST_CASE("the extension is read whatever its case", "[video][model]") {
    CHECK(isVideoFile("FILM.MKV"));
    CHECK(isVideoFile("Film.Mp4"));
}

// The judgement is on the name, and the name alone: nothing here opens the
// file. A directory called `film.mkv` would pass, and it is the caller's
// business to have asked the file system for files.
TEST_CASE("the answer is about the name and never about the content", "[video][model]") {
    CHECK(isVideoFile("this-file-does-not-exist-anywhere.mkv"));
}
