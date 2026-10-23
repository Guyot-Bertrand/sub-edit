// Preparing the audio: the tracks of a container and their extraction, against
// stand-ins for ffprobe and ffmpeg.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/audio_source.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

#include "scratch_directory.hpp"

namespace {

using subedit::core::AudioPreparationErrorKind;
using subedit::core::ContainerAudioTrack;
using subedit::test::ScratchDirectory;

const std::filesystem::path kFake{SUBEDIT_FAKE_TRANSCRIBER_BINARY};
const std::string kProbeSample =
    std::string{SUBEDIT_TEST_DATA_DIR} + "/audio-tracks/ffprobe.audio-2.txt";

std::string contentOf(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

} // namespace

TEST_CASE("the tracks are read from what ffprobe really wrote", "[audio]") {
    const auto tracks = subedit::core::parseAudioTracks(contentOf(kProbeSample));

    CHECK(tracks ==
          std::vector<ContainerAudioTrack>{{0, "fra", "Original"}, {1, "eng", "Commentary"}});
}

TEST_CASE("a track with no tag has none, and a title keeps its commas", "[audio]") {
    const auto tracks = subedit::core::parseAudioTracks(
        "[STREAM]\nindex=1\n[/STREAM]\n[STREAM]\nindex=2\nTAG:title=One, two, three\n[/STREAM]\n");

    CHECK(tracks == std::vector<ContainerAudioTrack>{{0, "", ""}, {1, "", "One, two, three"}});
}

TEST_CASE("nothing written is no track", "[audio]") {
    CHECK(subedit::core::parseAudioTracks("").empty());
}

TEST_CASE("the tracks of a video come from ffprobe", "[audio]") {
    const ScratchDirectory scratch;
    std::ignore =
        scratch.standIn("ffprobe", kFake, "--no-echo --stdout-from='" + kProbeSample + "'");
    const subedit::core::RealFileSystem files;

    const auto tracks = subedit::core::readAudioTracks(files, scratch.path().string(), "film.mkv");

    REQUIRE(tracks.has_value());
    CHECK(tracks.value_or(std::vector<ContainerAudioTrack>{}).size() == 2);
}

TEST_CASE("without ffprobe, or with a file it refuses, the tracks are not known", "[audio]") {
    const ScratchDirectory scratch;
    const subedit::core::RealFileSystem files;

    CHECK_FALSE(
        subedit::core::readAudioTracks(files, scratch.path().string(), "film.mkv").has_value());

    std::ignore = scratch.standIn("ffprobe", kFake, "--no-echo --fail-with=1");
    CHECK_FALSE(
        subedit::core::readAudioTracks(files, scratch.path().string(), "film.mkv").has_value());
}

TEST_CASE("a track is extracted to where it is asked for", "[audio]") {
    const ScratchDirectory scratch;
    std::ignore =
        scratch.standIn("ffprobe", kFake, "--no-echo --stdout-from='" + kProbeSample + "'");
    const auto sample = scratch.write("sample.wav", "pcm");
    std::ignore = scratch.standIn("ffmpeg", kFake, "--output-from='" + sample.string() + "'");
    const subedit::core::RealFileSystem files;
    const auto destination = scratch.file("audio.wav");

    const auto done =
        subedit::core::extractAudio(files, scratch.path().string(), "film.mkv", 1, destination);

    REQUIRE(done.has_value());
    CHECK(contentOf(destination) == "pcm");
}

TEST_CASE("the first track is the default, and the tracks need not be countable", "[audio]") {
    const ScratchDirectory scratch;
    const auto sample = scratch.write("sample.wav", "pcm");
    std::ignore = scratch.standIn("ffmpeg", kFake, "--output-from='" + sample.string() + "'");
    const subedit::core::RealFileSystem files;
    const auto destination = scratch.file("audio.wav");

    const auto done = subedit::core::extractAudio(
        files, scratch.path().string(), "voice.mp3", std::nullopt, destination);

    REQUIRE(done.has_value());
    CHECK(contentOf(destination) == "pcm");
}

TEST_CASE("without ffmpeg the extraction says so", "[audio]") {
    const ScratchDirectory scratch;
    const subedit::core::RealFileSystem files;

    const auto done = subedit::core::extractAudio(
        files, scratch.path().string(), "film.mkv", 0, scratch.file("audio.wav"));

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == AudioPreparationErrorKind::ToolMissing);
}

TEST_CASE("a track the container does not have is told before anything runs", "[audio]") {
    const ScratchDirectory scratch;
    std::ignore =
        scratch.standIn("ffprobe", kFake, "--no-echo --stdout-from='" + kProbeSample + "'");
    std::ignore = scratch.standIn("ffmpeg", kFake, "");
    const subedit::core::RealFileSystem files;

    const auto done = subedit::core::extractAudio(
        files, scratch.path().string(), "film.mkv", 2, scratch.file("audio.wav"));

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == AudioPreparationErrorKind::NoSuchTrack);
    CHECK(done.error().detail == "2");
}

TEST_CASE("a failed extraction says what the tool said and leaves no file", "[audio]") {
    const ScratchDirectory scratch;
    std::ignore = scratch.standIn("ffmpeg", kFake, "--fail-with=1");
    const subedit::core::RealFileSystem files;
    const auto destination = scratch.write("audio.wav", "partial");

    const auto done =
        subedit::core::extractAudio(files, scratch.path().string(), "film.mkv", 0, destination);

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == AudioPreparationErrorKind::Failed);
    CHECK(done.error().detail.find("refusing") != std::string::npos);
    CHECK_FALSE(std::filesystem::exists(destination));
}

TEST_CASE("a tool that cannot be started is a failed extraction", "[audio]") {
    const ScratchDirectory scratch;
    std::ignore = scratch.write("ffmpeg", "not a program");
    std::filesystem::permissions(scratch.file("ffmpeg"),
                                 std::filesystem::perms::owner_all,
                                 std::filesystem::perm_options::add);
    const subedit::core::RealFileSystem files;

    const auto done = subedit::core::extractAudio(
        files, scratch.path().string(), "film.mkv", 0, scratch.file("audio.wav"));

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == AudioPreparationErrorKind::Failed);
}

TEST_CASE("asking to stop stops the extraction, and the lines are passed on", "[audio]") {
    const ScratchDirectory scratch;
    std::ignore = scratch.standIn("ffmpeg", kFake, "--linger=30");
    const subedit::core::RealFileSystem files;
    int lines = 0;
    subedit::core::StreamObserver observer;
    observer.cancelled = [&lines] { return lines >= 1; };
    observer.onLine = [&lines](const subedit::core::StreamedLine&) { ++lines; };

    const auto done = subedit::core::extractAudio(
        files, scratch.path().string(), "film.mkv", 0, scratch.file("audio.wav"), observer);

    REQUIRE_FALSE(done.has_value());
    CHECK(done.error().kind == AudioPreparationErrorKind::Cancelled);
}
