// What a document remembers of its file, beyond its subtitles and its header.
//
// Three things, and ADR 0030 says why they are one family rather than three
// special cases: each is something a *writer* has to read back rather than
// repeat, so none of them fits in the free header string.

#include <subedit/core/format/read_result.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <variant>
#include <vector>

namespace {

using subedit::core::FileExtras;
using subedit::core::FrameRate;
using subedit::core::MicroDvdFile;
using subedit::core::SourceFile;
using subedit::core::StandardFrameRate;
using subedit::core::SubStationAlphaFile;
using subedit::core::TMPlayerFile;

} // namespace

TEST_CASE("a file declares nothing of its own by default", "[model][file-extras]") {
    const FileExtras extras;

    CHECK(std::holds_alternative<std::monostate>(extras));
}

TEST_CASE("a Sub Station Alpha file carries the order of its event fields",
          "[model][file-extras]") {
    // The order varies from one producer to the next, and writing composes each
    // `Dialogue:` line from it. Parsing it back out of the header at every
    // writing is what keeping it here avoids.
    const SubStationAlphaFile file{
        .eventFields = {"Marked",
                        "Start",
                        "End",
                        "Style",
                        "Name",
                        "MarginL",
                        "MarginR",
                        "MarginV",
                        "Effect",
                        "Text"},
    };

    REQUIRE(file.eventFields.size() == 10);
    CHECK(file.eventFields.front() == "Marked");
    CHECK(file.eventFields.back() == "Text");
}

TEST_CASE("a TMPlayer file carries the shape of its hour", "[model][file-extras]") {
    // `0:00:12:` and `00:00:12:` are both TMPlayer, and nothing else in the
    // file says which it uses. Two digits is the shape Gaupol writes when it
    // has nothing to go on.
    const TMPlayerFile fromNowhere;
    CHECK(fromNowhere.twoDigitHour);

    const TMPlayerFile oneDigit{.twoDigitHour = false};
    CHECK_FALSE(oneDigit.twoDigitHour);
}

TEST_CASE("a MicroDVD file carries the rate it was read at", "[model][file-extras]") {
    // **No MicroDVD file declares a rate**, so the one used to read it is the
    // only thing that can put its frame numbers back. The default is Gaupol's,
    // and iso-functionality is the whole reason — ADR 0030.
    const MicroDvdFile fromNowhere;
    CHECK(fromNowhere.rate == FrameRate{StandardFrameRate::Fps23976});

    const MicroDvdFile pal{.rate = FrameRate{StandardFrameRate::Fps25}};
    CHECK(pal.rate == FrameRate{StandardFrameRate::Fps25});
}

TEST_CASE("what a file declared survives into what the model keeps", "[model][file-extras]") {
    // The one path that matters: a reader fills `ReadResult`, and `sourceFileOf`
    // is what the model keeps of it. Anything dropped there is dropped for
    // good, and the writer puts back a file that is not the one that was read.
    subedit::core::ReadResult result;
    result.format = subedit::core::SubtitleFormat::TMPlayer;
    result.extras = TMPlayerFile{.twoDigitHour = false};

    const SourceFile kept = sourceFileOf(result, "film.txt");

    REQUIRE(std::holds_alternative<TMPlayerFile>(kept.extras));
    CHECK_FALSE(std::get<TMPlayerFile>(kept.extras).twoDigitHour);
}
