#include <subedit/cli/writing.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace {

using subedit::cli::writeSubtitlesTo;
using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::WriteRequest;

/// The encoding of that name, or a failed test.
[[nodiscard]] Encoding named(const char* name) {
    const std::optional<Encoding> encoding = Encoding::create(name, ByteOrderMark::Absent);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas cet encodage");
        return Encoding::utf8(ByteOrderMark::Absent);
    }
    return *encoding;
}

[[nodiscard]] std::vector<Subtitle> oneSaying(const std::string& text) {
    return {Subtitle{.start = Timestamp::fromMilliseconds(1000),
                     .end = Timestamp::fromMilliseconds(2000),
                     .mainText = text}};
}

} // namespace

TEST_CASE("writing puts the bytes on disk and says how many", "[cli][writing]") {
    InMemoryFileSystem files;
    const std::vector<Subtitle> subtitles = oneSaying("Un café.");

    const std::expected<std::size_t, std::string> written =
        writeSubtitlesTo(files,
                         "out.srt",
                         SubtitleFormat::SubRip,
                         WriteRequest{.subtitles = subtitles, .encoding = named("iso-8859-1")});

    REQUIRE(written.has_value());
    CHECK(*written == files.contentOf("out.srt").value_or("").size());
    // Latin-1: the accented letter is one byte, not two.
    CHECK(files.contentOf("out.srt").value_or("").find("caf\xE9") != std::string::npos);
}

TEST_CASE("a character the encoding cannot write stops the writing", "[cli][writing]") {
    // The first of the two steps, and the one only the user can settle: `ł` has
    // no place in Latin-1. Nothing reaches the disk — a file half written in an
    // encoding that cannot carry it would be worse than none.
    InMemoryFileSystem files;
    const std::vector<Subtitle> subtitles = oneSaying("Przyszedł późno.");

    const std::expected<std::size_t, std::string> written =
        writeSubtitlesTo(files,
                         "out.srt",
                         SubtitleFormat::SubRip,
                         WriteRequest{.subtitles = subtitles, .encoding = named("iso-8859-1")});

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error() == "holds a character the chosen encoding cannot write");
    CHECK_FALSE(files.contentOf("out.srt").has_value());
}

TEST_CASE("a disk that refuses the bytes says so, and names the destination", "[cli][writing]") {
    // The other step, and the message carries the destination: the file being
    // worked on and the file being written are two different paths, and a user
    // told only the first would look in the wrong place.
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);

    const std::expected<std::size_t, std::string> written = writeSubtitlesTo(
        files, "out.srt", SubtitleFormat::SubRip, WriteRequest{.subtitles = oneSaying("Un café.")});

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error() == "out.srt: cannot be opened: permission denied");
}
