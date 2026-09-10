// What a conversion costs, post by post — the policy the phase 9 scoping set.
//
// **Two rules, and the second is the harder one.** A conversion never refuses,
// and it never keeps quiet about what it lost — but it does keep quiet when it
// lost nothing, because a report printed every time is a report nobody reads.

#include <subedit/core/format/degradation.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

using subedit::core::asWrittenBy;
using subedit::core::carriesEnds;
using subedit::core::ConversionLoss;
using subedit::core::convertFor;
using subedit::core::FrameRate;
using subedit::core::holdsLineBreaks;
using subedit::core::SourceFile;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;

constexpr FrameRate kPal{StandardFrameRate::Fps25};

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(start),
        .end = Timestamp::fromMilliseconds(end),
        .mainText = std::move(text),
    };
}

[[nodiscard]] SourceFile from(SubtitleFormat format, std::string header = {}) {
    return SourceFile{.format = format, .header = std::move(header)};
}

} // namespace

TEST_CASE("seven of the nine formats carry an end, and two do not", "[format][degradation]") {
    for (const SubtitleFormat format : subedit::core::kSubtitleFormats) {
        INFO("format : " << subedit::core::nameOf(format));
        const bool endless = format == SubtitleFormat::TMPlayer || format == SubtitleFormat::Lrc;
        CHECK(carriesEnds(format) == !endless);
        CHECK(holdsLineBreaks(format) == (format != SubtitleFormat::Lrc));
    }
}

TEST_CASE("a position lands where the arriving format's grain puts it", "[format][degradation]") {
    const Timestamp odd = Timestamp::fromMilliseconds(3999);

    CHECK(asWrittenBy(odd, SubtitleFormat::SubRip, kPal).milliseconds() == 3999);
    CHECK(asWrittenBy(odd, SubtitleFormat::SubViewer2, kPal).milliseconds() == 4000);
    CHECK(asWrittenBy(odd, SubtitleFormat::Mpl2, kPal).milliseconds() == 4000);
    CHECK(asWrittenBy(odd, SubtitleFormat::TMPlayer, kPal).milliseconds() == 4000);
    // Twenty-five frames a second is forty milliseconds each, so 3 999 falls on
    // the hundredth frame — four seconds exactly.
    CHECK(asWrittenBy(odd, SubtitleFormat::MicroDvd, kPal).milliseconds() == 4000);
}

TEST_CASE("a conversion that loses nothing reports nothing", "[format][degradation]") {
    std::vector<Subtitle> subtitles = {at(1000, 3000, "<i>penché</i>")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::SubRip), SubtitleFormat::SubViewer2, kPal);

    CHECK_FALSE(loss.isAny());
    CHECK(loss == ConversionLoss{});
    CHECK(subtitles.front().mainText == "<i>penché</i>");
}

TEST_CASE("a document going nowhere is not converted at all", "[format][degradation]") {
    // The format has not changed, so nothing is a conversion and nothing is a
    // loss — the raw text is not even decoded.
    std::vector<Subtitle> subtitles = {at(1000, 3000, "<v Marie>intact</v>")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::WebVtt), SubtitleFormat::WebVtt, kPal);

    CHECK_FALSE(loss.isAny());
    CHECK(subtitles.front().mainText == "<v Marie>intact</v>");
}

TEST_CASE("the ends a format does not carry are a post of their own", "[format][degradation]") {
    std::vector<Subtitle> subtitles = {at(1000, 3000, "Une réplique.")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::SubRip), SubtitleFormat::Lrc, kPal);

    CHECK(loss.ends);
}

TEST_CASE("the line breaks LRC joins are counted, subtitle by subtitle", "[format][degradation]") {
    std::vector<Subtitle> subtitles = {at(1000, 3000, "sur deux\nlignes"),
                                       at(4000, 6000, "sur une")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::SubRip), SubtitleFormat::Lrc, kPal);

    CHECK(loss.joined == 1);
}

TEST_CASE("the tags an arriving format cannot write are counted", "[format][degradation]") {
    std::vector<Subtitle> subtitles = {at(1000, 3000, R"(<font color="#ff0000">rouge</font>)")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::SubRip), SubtitleFormat::WebVtt, kPal);

    CHECK(loss.tags == 1);
    CHECK(subtitles.front().mainText == "rouge");
}

TEST_CASE("a header does not cross a format boundary, and that is said", "[format][degradation]") {
    std::vector<Subtitle> subtitles = {at(1000, 3000, "Une réplique.")};

    const ConversionLoss loss = convertFor(
        subtitles, from(SubtitleFormat::WebVtt, "WEBVTT - Dialogue"), SubtitleFormat::SubRip, kPal);

    CHECK(loss.header);
}

TEST_CASE("a branch of extras is not a loss until it holds something", "[format][degradation]") {
    // **The mistake this guards against**: every SubRip subtitle carries a
    // `SubRipExtras`, and almost none carries coordinates. Counting the branch
    // would tell everyone converting an ordinary `.srt` that they lost data.
    std::vector<Subtitle> bare = {at(1000, 3000, "Une réplique.")};
    bare.front().extras = subedit::core::SubRipExtras{};

    CHECK(convertFor(bare, from(SubtitleFormat::SubRip), SubtitleFormat::Lrc, kPal).fields == 0);

    std::vector<Subtitle> placed = {at(1000, 3000, "Une réplique.")};
    placed.front().extras =
        subedit::core::SubRipExtras{.coordinates = subedit::core::Rectangle{.x1 = 40}};

    CHECK(convertFor(placed, from(SubtitleFormat::SubRip), SubtitleFormat::Lrc, kPal).fields == 1);
}

TEST_CASE("the largest a position moved is the one reported", "[format][degradation]") {
    std::vector<Subtitle> subtitles = {at(1001, 3000, "Une."), at(4000, 6007, "Deux.")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::SubRip), SubtitleFormat::SubViewer2, kPal);

    // A hundredth is the grain: 1 001 comes back to 1 000, 6 007 to 6 010.
    CHECK(loss.precision == 3);
}

TEST_CASE("an end that is not written cannot have moved", "[format][degradation]") {
    // **Only the positions the file will hold are measured.** An LRC writes no
    // end, so rounding one would report a shift nobody can see.
    std::vector<Subtitle> subtitles = {at(1000, 3007, "Une réplique.")};

    const ConversionLoss loss =
        convertFor(subtitles, from(SubtitleFormat::SubRip), SubtitleFormat::Lrc, kPal);

    CHECK(loss.precision == 0);
}
