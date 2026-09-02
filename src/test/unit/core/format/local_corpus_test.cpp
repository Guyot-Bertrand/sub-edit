// A second corpus, proper to each machine and kept out of the repository.
//
// The test reads whatever `src/data/` holds and declares itself skipped when
// the directory is absent, which is the case in continuous integration.
//
// What it buys: the versioned corpus covers the shapes we thought of, this one
// meets the shapes we did not.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/real_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::RealFileSystem;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

[[nodiscard]] std::vector<std::filesystem::path> localFiles() {
    const std::filesystem::path directory{SUBEDIT_LOCAL_DATA_DIR};
    std::error_code ignored;
    if (!std::filesystem::is_directory(directory, ignored))
        return {};

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator{directory, ignored})
        if (entry.is_regular_file())
            files.push_back(entry.path());
    return files;
}

[[nodiscard]] std::string roundTrip(const ReadResult& result) {
    return writeSubtitles(result.format,
                          WriteRequest{
                              .subtitles = result.subtitles,
                              .newline = result.newline,
                              .encoding = result.encoding,
                              .header = result.header,
                          });
}

} // namespace

TEST_CASE("local files open, or fail for a reason we can name", "[format][local]") {
    const std::vector<std::filesystem::path> files = localFiles();
    if (files.empty())
        SKIP("aucun fichier local dans src/data/");

    const RealFileSystem disk;
    for (const std::filesystem::path& path : files) {
        const std::expected<std::string, subedit::core::FileError> bytes = disk.readFile(path);
        REQUIRE(bytes.has_value());

        const std::expected<ReadResult, ReadError> result = readSubtitles(*bytes);
        INFO("fichier : " << path.filename().string());

        if (!result.has_value()) {
            // The only failure allowed for now: an encoding this phase does
            // not handle. Anything else is a defect.
            CHECK(result.error().kind == ReadErrorKind::Undecodable);
            continue;
        }

        CHECK_FALSE(result->subtitles.empty());
    }
}

TEST_CASE("a local file that opens survives being saved twice", "[format][local]") {
    // Byte for byte is not promised here: these files come from other tools,
    // with their own idea of the trailing blank line. What must hold is that
    // the first save settles the shape and no later save moves it.
    const std::vector<std::filesystem::path> files = localFiles();
    if (files.empty())
        SKIP("aucun fichier local dans src/data/");

    const RealFileSystem disk;
    for (const std::filesystem::path& path : files) {
        const std::expected<std::string, subedit::core::FileError> bytes = disk.readFile(path);
        REQUIRE(bytes.has_value());

        const std::expected<ReadResult, ReadError> result = readSubtitles(*bytes);
        if (!result.has_value())
            continue;

        INFO("fichier : " << path.filename().string());
        const std::string once = roundTrip(*result);

        const std::expected<ReadResult, ReadError> reread = readSubtitles(once);
        REQUIRE(reread.has_value());
        CHECK(reread->subtitles.size() == result->subtitles.size());
        CHECK(roundTrip(*reread) == once);
    }
}
