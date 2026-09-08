#include <subedit/core/analysis/grid_verdict.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <string_view>

using subedit::core::extensionOf;
using subedit::core::FrameRate;
using subedit::core::nameOf;
using subedit::core::StandardFrameRate;

namespace {

/// A rate outside the eight standards, unwrapped where the test can say so.
FrameRate rateOf(std::int64_t numerator, std::int64_t denominator) {
    const std::optional<FrameRate> created = FrameRate::create(numerator, denominator);
    if (!created.has_value()) {
        FAIL("a frame rate with two positive terms must be accepted");
        return FrameRate{StandardFrameRate::Fps25};
    }
    return *created;
}

} // namespace

TEST_CASE("every kind of anomaly has a clause", "[cli][wording]") {
    // Named one by one rather than looped over, for the same reason as below.
    // The clauses follow « subtitle 12 », so each starts with its verb.
    using subedit::core::AnomalyKind;
    CHECK(nameOf(AnomalyKind::EndBeforeStart) == "ends before it starts");
    CHECK(nameOf(AnomalyKind::OverlappingSubtitles) == "starts before the previous one ends");
    CHECK(nameOf(AnomalyKind::OutOfOrder) == "starts before the previous one starts");
}

TEST_CASE("every kind of diagnostic has a phrase", "[cli][wording]") {
    // Named one by one rather than looped over: a new enumerator must fail to
    // compile here, not fall through to an empty line in a report.
    using subedit::core::DiagnosticKind;
    CHECK(nameOf(DiagnosticKind::IgnoredLine) == "a line that fits nowhere");
    CHECK(nameOf(DiagnosticKind::MalformedTimestamp) == "a timing line that could not be read");
    CHECK(nameOf(DiagnosticKind::MissingNumbering) == "a SubRip block without its number");
    CHECK(nameOf(DiagnosticKind::InconsistentNumbering) == "SubRip numbers that do not follow");
    CHECK(nameOf(DiagnosticKind::TextBeforeAnyTimestamp) == "text before the first timing line");
    CHECK(nameOf(DiagnosticKind::UnknownBlock) == "a WebVTT block of an unknown kind");
    CHECK(nameOf(DiagnosticKind::UnknownEventField) ==
          "declares an event column this tool cannot fill");
    CHECK(nameOf(DiagnosticKind::MixedNewlines) == "more than one kind of line ending");
    CHECK(nameOf(DiagnosticKind::GuessedEncoding) == "an encoding nothing declared");
    CHECK(nameOf(DiagnosticKind::MarkOverridesEncoding) ==
          "a byte order mark that contradicts the encoding asked for");
}

TEST_CASE("what was done about an anomaly is named too", "[cli][wording]") {
    using subedit::core::Severity;
    // The whole point of the distinction: one of the two needs a human.
    CHECK(nameOf(Severity::Warning) == "left as it stands");
    CHECK(nameOf(Severity::Recovered) == "settled by the reader");
}

TEST_CASE("a whole rate is named by its number alone", "[cli][wording]") {
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps25}) == "25");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps24}) == "24");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps60}) == "60");
}

TEST_CASE("a rate a decimal can write exactly is named as a decimal", "[cli][wording]") {
    CHECK(nameOf(rateOf(239, 10)) == "23.9");
    CHECK(nameOf(rateOf(25, 2)) == "12.5");
    CHECK(nameOf(rateOf(23976, 1000)) == "23.976");
}

TEST_CASE("an NTSC rate is named by its fraction", "[cli][wording]") {
    // Writing "23.976" here would be the lie the grammar refuses to tell in the
    // other direction: 24000/1001 has no terminating decimal, so naming it by
    // one would report a conversion that did not happen.
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps23976}) == "24000/1001");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps29970}) == "30000/1001");
    CHECK(nameOf(FrameRate{StandardFrameRate::Fps59940}) == "60000/1001");
}

TEST_CASE("a rate below one frame per second is named all the same", "[cli][wording]") {
    CHECK(nameOf(rateOf(1, 8)) == "0.125");
}

TEST_CASE("every kind of command has a name of its own", "[wording]") {
    // Two identical names would make an undo action ambiguous, and an empty
    // name would make it mute. The compiler holds the exhaustiveness of the
    // `switch`; this test holds what it cannot see.
    constexpr std::array kEveryKind = {
        subedit::core::CommandKind::SetText,
        subedit::core::CommandKind::SetStart,
        subedit::core::CommandKind::SetEnd,
        subedit::core::CommandKind::Insert,
        subedit::core::CommandKind::Remove,
        subedit::core::CommandKind::Shift,
        subedit::core::CommandKind::Transform,
        subedit::core::CommandKind::ConvertFrameRate,
        subedit::core::CommandKind::Snap,
        subedit::core::CommandKind::Sort,
        subedit::core::CommandKind::RemoveHearingImpaired,
    };

    std::set<std::string_view> seen;
    for (const subedit::core::CommandKind kind : kEveryKind) {
        CHECK_FALSE(nameOf(kind).empty());
        seen.insert(nameOf(kind));
    }

    CHECK(seen.size() == kEveryKind.size());
}

TEST_CASE("a length is written in signed seconds, to the millisecond", "[wording]") {
    using subedit::core::Duration;
    using subedit::core::secondsOf;

    CHECK(secondsOf(Duration::fromMilliseconds(2999)) == "2.999 s");
    CHECK(secondsOf(Duration::zero()) == "0.000 s");
    CHECK(secondsOf(Duration::fromMilliseconds(-7001)) == "-7.001 s");
    // A length between −1 s and 0 has a whole part of zero, which carries no
    // sign of its own. It was already right in the command line, and moving it
    // here is what keeps it right in both places.
    CHECK(secondsOf(Duration::fromMilliseconds(-500)) == "-0.500 s");
}

// The three formulations issue #174 asks for. What differs between them is the
// operation named — the two numbers are the same two in all three, and writing
// the sentence three times would have been three copies of one rule.
TEST_CASE("each operation says in its own name what it left past the end", "[wording]") {
    using subedit::core::BeyondEnd;
    using subedit::core::CommandKind;
    using subedit::core::Duration;
    using subedit::core::noticeOf;

    const BeyondEnd beyond{.count = 3, .overshoot = Duration::fromMilliseconds(4200)};

    CHECK(noticeOf(CommandKind::Shift, beyond) ==
          "shifting leaves 3 subtitles past the end of the video, by 4.200 s at most");
    CHECK(noticeOf(CommandKind::Transform, beyond) ==
          "transforming leaves 3 subtitles past the end of the video, by 4.200 s at most");
    CHECK(noticeOf(CommandKind::ConvertFrameRate, beyond) ==
          "converting the frame rate leaves 3 subtitles past the end of the video, "
          "by 4.200 s at most");
}

// One is the common case, and « the furthest by » would read wrong for it.
TEST_CASE("a single subtitle past the end is said in the singular", "[wording]") {
    using subedit::core::BeyondEnd;
    using subedit::core::CommandKind;
    using subedit::core::Duration;
    using subedit::core::noticeOf;

    CHECK(noticeOf(CommandKind::Shift,
                   BeyondEnd{.count = 1, .overshoot = Duration::fromMilliseconds(500)}) ==
          "shifting leaves 1 subtitle past the end of the video, by 0.500 s at most");
}

TEST_CASE("every grid verdict has a word of its own", "[wording]") {
    // The same three words the window shows in its status bar. Two surfaces
    // wording one verdict twice is how they start to disagree, which is why
    // this lives in `wording` rather than in the report that first needed it.
    using subedit::core::GridVerdict;

    CHECK(nameOf(GridVerdict::Clean) == "clean");
    CHECK(nameOf(GridVerdict::Partial) == "partial");
    CHECK(nameOf(GridVerdict::Silent) == "none");
}

TEST_CASE("a concentration is written with one decimal", "[wording]") {
    using subedit::core::percentOf;

    CHECK(percentOf(100.0) == "100.0%");
    CHECK(percentOf(99.87) == "99.9%");
    CHECK(percentOf(15.34) == "15.3%");
    // One decimal and not more: the third digit says nothing a reader can act
    // on, and a clean grid reads better as 99.9% than as 99.87342%.
    CHECK(percentOf(0.0) == "0.0%");
}

TEST_CASE("the status line names the film and the rate it declares", "[wording]") {
    using subedit::core::FrameRate;
    using subedit::core::StandardFrameRate;
    using subedit::core::videoStatusOf;

    const std::filesystem::path film{"/films/le-canot.mkv"};

    CHECK(videoStatusOf(std::nullopt) == "No video");
    CHECK(videoStatusOf(film) == "Video: le-canot.mkv");
    // The rate goes with the film rather than beside it: they are one fact,
    // and a third widget would put it at the same rank as the grid deduced
    // from the positions, which is another fact entirely.
    CHECK(videoStatusOf(film, FrameRate{StandardFrameRate::Fps23976}) ==
          "Video: le-canot.mkv, 24000/1001 fps");
}

TEST_CASE("every format has a name, and it is the one the outside world knows",
          "[cli][wording][format]") {
    // Named one by one rather than looped over, for the reason the clauses
    // above give: a tenth format has to fail to compile here.
    //
    // **These strings are read outside this repository.** They are what
    // `inspect` prints, and what `score-format-detection.py` compares its
    // labels to — a name changed here without being changed there turns the
    // score of the phase into nonsense while every test stays green.
    using subedit::core::SubtitleFormat;
    CHECK(nameOf(SubtitleFormat::SubRip) == "SubRip");
    CHECK(nameOf(SubtitleFormat::WebVtt) == "WebVTT");
    CHECK(nameOf(SubtitleFormat::SubViewer2) == "SubViewer 2");
    CHECK(nameOf(SubtitleFormat::SubStationAlpha) == "Sub Station Alpha");
    CHECK(nameOf(SubtitleFormat::AdvancedSubStationAlpha) == "Advanced SSA");
    CHECK(nameOf(SubtitleFormat::MicroDvd) == "MicroDVD");
    CHECK(nameOf(SubtitleFormat::Mpl2) == "MPL2");
    CHECK(nameOf(SubtitleFormat::TMPlayer) == "TMPlayer");
    CHECK(nameOf(SubtitleFormat::Lrc) == "LRC");
}

TEST_CASE("every format has an extension, and two of them are shared", "[cli][wording][format]") {
    using subedit::core::SubtitleFormat;
    CHECK(extensionOf(SubtitleFormat::SubRip) == ".srt");
    CHECK(extensionOf(SubtitleFormat::WebVtt) == ".vtt");
    CHECK(extensionOf(SubtitleFormat::SubViewer2) == ".sub");
    CHECK(extensionOf(SubtitleFormat::SubStationAlpha) == ".ssa");
    CHECK(extensionOf(SubtitleFormat::AdvancedSubStationAlpha) == ".ass");
    CHECK(extensionOf(SubtitleFormat::MicroDvd) == ".sub");
    CHECK(extensionOf(SubtitleFormat::Mpl2) == ".txt");
    CHECK(extensionOf(SubtitleFormat::TMPlayer) == ".txt");
    CHECK(extensionOf(SubtitleFormat::Lrc) == ".lrc");

    // **The sharing is the point of this case.** Going this way there is one
    // answer per format; going back there is none, which is why nothing in the
    // library maps an extension to a format.
    CHECK(extensionOf(SubtitleFormat::MicroDvd) == extensionOf(SubtitleFormat::SubViewer2));
    CHECK(extensionOf(SubtitleFormat::Mpl2) == extensionOf(SubtitleFormat::TMPlayer));
}

TEST_CASE("the two reasons a writing can fail have a sentence", "[cli][wording][format]") {
    using subedit::core::reasonOf;
    using subedit::core::WriteErrorKind;
    CHECK(reasonOf(WriteErrorKind::Unencodable) ==
          "holds a character the chosen encoding cannot write");
    CHECK(reasonOf(WriteErrorKind::NoWriter) == "cannot be written yet");
}

TEST_CASE("every format has a name a command line can take", "[cli][wording][format]") {
    // **Not the extension**, because two extensions name two formats each.
    // These are the values of `--to`, so they are named one by one here for the
    // same reason the format names are: they are a surface, and a surface that
    // changes silently is a surface that breaks someone's script.
    using subedit::core::optionNameOf;
    using subedit::core::SubtitleFormat;
    CHECK(optionNameOf(SubtitleFormat::SubRip) == "srt");
    CHECK(optionNameOf(SubtitleFormat::WebVtt) == "vtt");
    CHECK(optionNameOf(SubtitleFormat::SubViewer2) == "subviewer2");
    CHECK(optionNameOf(SubtitleFormat::SubStationAlpha) == "ssa");
    CHECK(optionNameOf(SubtitleFormat::AdvancedSubStationAlpha) == "ass");
    CHECK(optionNameOf(SubtitleFormat::MicroDvd) == "microdvd");
    CHECK(optionNameOf(SubtitleFormat::Mpl2) == "mpl2");
    CHECK(optionNameOf(SubtitleFormat::TMPlayer) == "tmplayer");
    CHECK(optionNameOf(SubtitleFormat::Lrc) == "lrc");
}

TEST_CASE("a name a command line takes leads back to its format", "[cli][wording][format]") {
    using subedit::core::formatNamed;
    using subedit::core::kSubtitleFormats;
    using subedit::core::optionNameOf;

    // Both ways round, on all nine: a name that led somewhere else, or nowhere,
    // would make `--to` answer with a format nobody asked for.
    for (const subedit::core::SubtitleFormat format : kSubtitleFormats)
        CHECK(formatNamed(optionNameOf(format)) == format);

    // The two extensions that name two formats are deliberately not names.
    CHECK_FALSE(formatNamed("sub").has_value());
    CHECK_FALSE(formatNamed("txt").has_value());
    CHECK_FALSE(formatNamed("").has_value());
}
