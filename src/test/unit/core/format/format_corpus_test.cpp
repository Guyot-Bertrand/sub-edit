// The scene, in nine formats, and what each format is promised.
//
// `src/test/data/formats/` holds one scene written nine times over: four
// subtitles, at the same positions, two of them on two lines and one in
// italics. Only the format changes from file to file, which is the discipline
// `encodages/` follows for encodings — it is what lets one reading be compared
// to another rather than two different texts.
//
// **The table below is the point of this file.** A round trip on bytes was one
// question asked of every file, and on two formats it was enough. On nine it
// goes quiet where it passes: LRC and TMPlayer write only starts, so their
// bytes come back identical whatever a reading invented for their ends. Each
// format therefore carries here what it may claim, and a file of the directory
// that claims nothing fails.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::RealFileSystem;
using subedit::core::Subtitle;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

/// What a file of one format is promised when it is read and written back.
enum class RoundTrip {
    /// The bytes come back identical, and that means something.
    Bytes,
    /// The bytes come back identical, and that on its own proves nothing.
    ///
    /// LRC and TMPlayer hold one position per line. A reading has to invent
    /// every end, and a writing puts none of them back — so the comparison
    /// would pass however wrong the invention was. What these two formats
    /// promise is therefore said twice: once on the bytes, once on the ends.
    BytesEmptily,
    /// The bytes come back only under something the file does not say.
    ///
    /// MicroDVD counts in frames, the model counts in milliseconds since
    /// ADR 0006, and no MicroDVD file states a rate. Whatever lets a writing
    /// find the frame again is a scoping decision; what the harness must know
    /// is that this format cannot claim the bytes on its own.
    Conditional,
};

/// Whether a reader of that format exists yet.
///
/// **A ratchet, and it is meant to be moved.** The issue that writes a reader
/// moves its format to `Readable`, and every assertion below starts applying
/// to it at once. Leaving it behind turns the last case red.
enum class Support {
    Readable,
    NotYet,
};

/// One line of the promise table: one format, one rendering of the scene.
struct Promise {
    std::string_view file;
    std::string_view name;
    RoundTrip roundTrip;
    Support support;
    /// What a reading of that rendering gives back, subtitle by subtitle.
    std::span<const std::string_view> texts;
    /// The ends, whether the file carries them or a reading invents them.
    std::span<const std::int64_t> ends;
};

/// The four starts, shared by the nine renderings.
///
/// **All on a whole second**, which the poorest of the nine dictates: TMPlayer
/// counts in whole seconds. They also fall on a frame at 25 per second, which
/// is the rate the MicroDVD rendering is written for.
constexpr std::array<std::int64_t, 4> kStarts = {1000, 4000, 8000, 12000};

/// The ends, for the seven formats that hold them.
constexpr std::array<std::int64_t, 4> kCarriedEnds = {3000, 6000, 11000, 15000};

/// The ends a reading invents for the two formats that hold none.
///
/// **Written out rather than computed.** They follow Gaupol's rule — the next
/// start, and five seconds for the last — but a rule checked against itself
/// checks nothing, so the four numbers are here in full.
constexpr std::array<std::int64_t, 4> kInventedEnds = {4000, 8000, 12000, 17000};

// The texts a reading gives back. Subtitles 1, 2 and 4 are the same in every
// format; the third one differs, and only because each format spells italics
// its own way. LRC differs twice over, having neither italics nor line breaks.

constexpr std::array<std::string_view, 4> kHtmlItalics = {
    "The harbour wakes before\nthe town does.",
    "Nothing moves on the water.",
    "<i>A gull turns once above the mast\nand settles.</i>",
    "Then the light comes.",
};

constexpr std::array<std::string_view, 4> kBraceItalics = {
    "The harbour wakes before\nthe town does.",
    "Nothing moves on the water.",
    R"({\i1}A gull turns once above the mast)"
    "\n"
    R"(and settles.{\i0})",
    "Then the light comes.",
};

constexpr std::array<std::string_view, 4> kMicroDvdItalics = {
    "The harbour wakes before\nthe town does.",
    "Nothing moves on the water.",
    "{Y:i}A gull turns once above the mast\nand settles.",
    "Then the light comes.",
};

constexpr std::array<std::string_view, 4> kMpl2Italics = {
    "The harbour wakes before\nthe town does.",
    "Nothing moves on the water.",
    "/A gull turns once above the mast\n/and settles.",
    "Then the light comes.",
};

constexpr std::array<std::string_view, 4> kNoItalics = {
    "The harbour wakes before\nthe town does.",
    "Nothing moves on the water.",
    "A gull turns once above the mast\nand settles.",
    "Then the light comes.",
};

constexpr std::array<std::string_view, 4> kOneLine = {
    "The harbour wakes before the town does.",
    "Nothing moves on the water.",
    "A gull turns once above the mast and settles.",
    "Then the light comes.",
};

constexpr std::array<Promise, 9> kPromises = {
    Promise{.file = "scene.srt",
            .name = "SubRip",
            .roundTrip = RoundTrip::Bytes,
            .support = Support::Readable,
            .texts = kHtmlItalics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.vtt",
            .name = "WebVTT",
            .roundTrip = RoundTrip::Bytes,
            .support = Support::Readable,
            .texts = kHtmlItalics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.subviewer2.sub",
            .name = "SubViewer 2",
            .roundTrip = RoundTrip::Bytes,
            .support = Support::Readable,
            .texts = kHtmlItalics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.ssa",
            .name = "Sub Station Alpha",
            .roundTrip = RoundTrip::Bytes,
            .support = Support::Readable,
            .texts = kBraceItalics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.ass",
            .name = "Advanced SSA",
            .roundTrip = RoundTrip::Bytes,
            .support = Support::Readable,
            .texts = kBraceItalics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.microdvd.sub",
            .name = "MicroDVD",
            .roundTrip = RoundTrip::Conditional,
            .support = Support::NotYet,
            .texts = kMicroDvdItalics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.mpl2.txt",
            .name = "MPL2",
            .roundTrip = RoundTrip::Bytes,
            .support = Support::Readable,
            .texts = kMpl2Italics,
            .ends = kCarriedEnds},
    Promise{.file = "scene.tmplayer.txt",
            .name = "TMPlayer",
            .roundTrip = RoundTrip::BytesEmptily,
            .support = Support::NotYet,
            .texts = kNoItalics,
            .ends = kInventedEnds},
    Promise{.file = "scene.lrc",
            .name = "LRC",
            .roundTrip = RoundTrip::BytesEmptily,
            .support = Support::NotYet,
            .texts = kOneLine,
            .ends = kInventedEnds},
};

/// The one file of the directory that is not a rendering of the scene.
///
/// **Named here rather than guessed from its extension.** `valides/` has no
/// such exemption — every regular file it holds has to open — which is why the
/// scene lives in a directory of its own.
constexpr std::string_view kReadme = "LISEZMOI.md";

[[nodiscard]] std::filesystem::path sceneDirectory() {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "formats";
}

[[nodiscard]] std::string bytesOf(std::string_view file) {
    const std::filesystem::path path = sceneDirectory() / file;
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content = files.readFile(path);
    if (!content.has_value()) {
        FAIL("rendu de la scène introuvable : " + path.string());
        return {};
    }
    return *std::move(content);
}

/// The renderings the directory holds, the readme aside.
[[nodiscard]] std::vector<std::string> renderings() {
    std::vector<std::string> found;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{sceneDirectory()}) {
        if (!entry.is_regular_file())
            continue;
        const std::string name = entry.path().filename().string();
        if (name != kReadme)
            found.push_back(name);
    }

    std::ranges::sort(found);
    return found;
}

} // namespace

TEST_CASE("the scene directory and the promise table name the same files",
          "[format][corpus][scene]") {
    // Both ways round, and that is the whole guard. A rendering added without
    // a promise would be read by nothing; a promise left without its rendering
    // would assert on a file that is no longer there. Neither shows up in a
    // diff, and both would keep the suite green.
    std::vector<std::string> promised;
    promised.reserve(kPromises.size());
    for (const Promise& promise : kPromises)
        promised.emplace_back(promise.file);
    std::ranges::sort(promised);

    CHECK(renderings() == promised);
}

TEST_CASE("the promise table says the same thing twice, and agrees with itself",
          "[format][corpus][scene]") {
    // A format that holds no end has to declare the ends a reading invents;
    // one that holds them has nothing to invent. The two columns are written
    // by hand, so they can disagree, and a disagreement is what would make the
    // next case assert on the wrong numbers.
    for (const Promise& promise : kPromises) {
        INFO("format : " << promise.name);
        const bool invents = promise.roundTrip == RoundTrip::BytesEmptily;
        CHECK(std::ranges::equal(promise.ends, invents ? kInventedEnds : kCarriedEnds));
        CHECK(promise.texts.size() == kStarts.size());
    }
}

TEST_CASE("every rendering that can be read gives back the same scene", "[format][corpus][scene]") {
    for (const Promise& promise : kPromises) {
        if (promise.support != Support::Readable)
            continue;

        INFO("format : " << promise.name);
        const std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf(promise.file));
        REQUIRE(result.has_value());
        REQUIRE(result->subtitles.size() == kStarts.size());

        for (std::size_t index = 0; index < kStarts.size(); ++index) {
            INFO("sous-titre " << index + 1);
            const Subtitle& subtitle = result->subtitles[index];
            CHECK(subtitle.start.milliseconds() == kStarts[index]);
            CHECK(subtitle.end.milliseconds() == promise.ends[index]);
            CHECK(subtitle.mainText == promise.texts[index]);
        }
    }
}

TEST_CASE("a rendering promised its bytes gives them back", "[format][corpus][scene]") {
    // MicroDVD is left out on purpose: what it promises depends on a rate it
    // does not state, so it cannot be asked this question without being told
    // that rate first.
    for (const Promise& promise : kPromises) {
        if (promise.support != Support::Readable || promise.roundTrip == RoundTrip::Conditional)
            continue;

        INFO("format : " << promise.name);
        const std::string original = bytesOf(promise.file);
        const std::expected<ReadResult, ReadError> result = readSubtitles(original);
        REQUIRE(result.has_value());

        const std::expected<std::string, WriteError> written =
            writeSubtitles(result->format,
                           WriteRequest{
                               .subtitles = result->subtitles,
                               .newline = result->newline,
                               .encoding = result->encoding,
                               .header = result->header,
                           });

        REQUIRE(written.has_value());
        CHECK(*written == original);
    }
}

TEST_CASE("a format with no reader yet is refused rather than guessed", "[format][corpus][scene]") {
    // **The ratchet.** Seven renderings sit in the directory before anything
    // can read them, and this is what keeps them from sitting there unnoticed:
    // the day a reader lands, this case fails until its format is moved to
    // `Readable`, and the three cases above take it over.
    for (const Promise& promise : kPromises) {
        if (promise.support != Support::NotYet)
            continue;

        INFO("format : " << promise.name);
        const std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf(promise.file));

        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().kind == ReadErrorKind::UnknownFormat);
    }
}
