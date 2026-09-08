// Reading Sub Station Alpha and Advanced SSA.
//
// The two most structured of the nine, and they come together because they
// differ by one column and a version string — which is why `ass.py` inherits
// from `ssa.py` in Gaupol.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_station_alpha_reader.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <expected>
#include <string>
#include <string_view>
#include <variant>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::SubStationAlphaExtras;
using subedit::core::SubStationAlphaFile;
using subedit::core::SubStationAlphaReader;
using subedit::core::SubtitleFormat;

[[nodiscard]] ReadResult read(std::string_view content,
                              SubtitleFormat format = SubtitleFormat::SubStationAlpha) {
    const std::expected<ReadResult, ReadError> result = SubStationAlphaReader{format}.read(content);
    if (!result.has_value()) {
        FAIL("the reading was meant to succeed");
        return {};
    }
    return *result;
}

[[nodiscard]] bool hasDiagnostic(const ReadResult& result, DiagnosticKind kind) {
    return std::ranges::any_of(result.diagnostics,
                               [kind](const auto& one) { return one.kind == kind; });
}

[[nodiscard]] SubStationAlphaExtras extrasOf(const ReadResult& result, std::size_t index) {
    const auto* extras = std::get_if<SubStationAlphaExtras>(&result.subtitles[index].extras);
    if (extras == nullptr) {
        FAIL("the subtitle was meant to carry Sub Station Alpha extras");
        return {};
    }
    return *extras;
}

constexpr std::string_view kSsaFile =
    "[Script Info]\n"
    "ScriptType: v4.00\n"
    "\n"
    "[Events]\n"
    "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
    "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,Le vent se lève.\n";

} // namespace

TEST_CASE("a Sub Station Alpha file gives its subtitles and its columns", "[format][ssa]") {
    const ReadResult result = read(kSsaFile);

    CHECK(result.format == SubtitleFormat::SubStationAlpha);
    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[0].end.milliseconds() == 3000);
    CHECK(result.subtitles[0].mainText == "Le vent se lève.");
    CHECK(result.diagnostics.empty());

    // **The order is read, never assumed** — it varies from one producer to the
    // next, and writing has to put back the one the file had.
    const auto* file = std::get_if<SubStationAlphaFile>(&result.extras);
    REQUIRE(file != nullptr);
    REQUIRE(file->eventFields.size() == 10);
    CHECK(file->eventFields.front() == "Marked");
    CHECK(file->eventFields.back() == "Text");
}

TEST_CASE("the header stops at the events, and comes back without its blank lines",
          "[format][ssa]") {
    const ReadResult result = read(kSsaFile);

    CHECK(result.header == "[Script Info]\nScriptType: v4.00");
}

TEST_CASE("what an event line carries besides its text is kept", "[format][ssa]") {
    const ReadResult result =
        read("[Script Info]\nScriptType: v4.00\n"
             "[Events]\n"
             "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
             "Dialogue: Marked=1,0:00:01.00,0:00:03.00,Titre,Marie,0030,0040,0010,fade,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    const SubStationAlphaExtras extras = extrasOf(result, 0);
    CHECK(extras.marked == 1);
    CHECK(extras.style == "Titre");
    CHECK(extras.name == "Marie");
    CHECK(extras.marginLeft == 30);
    CHECK(extras.marginRight == 40);
    CHECK(extras.marginVertical == 10);
    CHECK(extras.effect == "fade");
}

TEST_CASE("Advanced SSA opens on a layer where the other opens on a mark", "[format][ssa]") {
    // The one column the two formats disagree on, and they are held apart in
    // the model: `Marked=1` is a bookmark, `Layer: 1` is what a subtitle is
    // drawn over.
    const ReadResult result =
        read("[Script Info]\nScriptType: v4.00+\n"
             "[Events]\n"
             "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
             "Dialogue: 2,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,Bonjour.\n",
             SubtitleFormat::AdvancedSubStationAlpha);

    REQUIRE(result.subtitles.size() == 1);
    const SubStationAlphaExtras extras = extrasOf(result, 0);
    CHECK(extras.layer == 2);
    CHECK(extras.marked == 0);
}

TEST_CASE("the text keeps the commas it holds", "[format][ssa]") {
    // **The last column is not split**, and this is the rule that makes the
    // format readable at all: the `Format:` line says how many columns precede
    // the text, and everything after them is the text.
    const ReadResult result =
        read("[Script Info]\nScriptType: v4.00\n"
             "[Events]\n"
             "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
             "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,"
             "Oui, bien sûr, comme toujours.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Oui, bien sûr, comme toujours.");
}

TEST_CASE("both break markers become a real line break", "[format][ssa]") {
    const ReadResult result =
        read("[Script Info]\nScriptType: v4.00\n"
             "[Events]\n"
             "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
             "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,"
             "Une ligne\\Nune autre\\nune troisième.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Une ligne\nune autre\nune troisième.");
}

TEST_CASE("the columns may come in any order", "[format][ssa]") {
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Style, Start, MarginV, End, Text\n"
                                   "Dialogue: Titre,0:00:01.00,0010,0:00:03.00,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[0].end.milliseconds() == 3000);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
    CHECK(extrasOf(result, 0).style == "Titre");
    CHECK(extrasOf(result, 0).marginVertical == 10);
}

TEST_CASE("it is the last column that keeps the commas, whichever it is", "[format][ssa]") {
    // **The rule is about the position, not about the name**, and Gaupol's is
    // the same: the split stops one comma short of the column count and hands
    // the rest over whole. Every file in the wild declares `Text` last, which
    // is what makes the rule look like it is about the text.
    //
    // A file that declared something else last would put the commas there. It
    // would come back byte for byte all the same, which is what matters.
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Start, End, Text, Style\n"
                                   "Dialogue: 0:00:01.00,0:00:03.00,Bonjour, Marie.,Titre\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour");
    CHECK(extrasOf(result, 0).style == " Marie.,Titre");
}

TEST_CASE("a column nothing here can fill is declared and reported", "[format][ssa]") {
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, Karaoke, Text\n"
                                   "Dialogue: Marked=0,0:00:01.00,0:00:03.00,x,Bonjour.\n");

    CHECK(hasDiagnostic(result, DiagnosticKind::UnknownEventField));
    const auto* file = std::get_if<SubStationAlphaFile>(&result.extras);
    REQUIRE(file != nullptr);
    // Left out of what is written back: a file has to declare the columns it
    // fills, and this one could not be filled.
    CHECK(std::ranges::find(file->eventFields, "Karaoke") == file->eventFields.end());
}

TEST_CASE("an event without both its positions is reported, not placed at the origin",
          "[format][ssa]") {
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, Text\n"
                                   "Dialogue: Marked=0,pas une heure,0:00:03.00,Bonjour.\n"
                                   "Dialogue: Marked=0,0:00:04.00,0:00:06.00,Une vraie.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Une vraie.");
    CHECK(hasDiagnostic(result, DiagnosticKind::MalformedTimestamp));
}

TEST_CASE("an events section that declares no columns falls back on its format's",
          "[format][ssa]") {
    // Gaupol raises here — `indices` is never bound. A truncated file looks
    // exactly like this.
    const ReadResult result =
        read("[Script Info]\nScriptType: v4.00\n"
             "[Events]\n"
             "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
}

TEST_CASE("a line inside the events that is neither is reported", "[format][ssa]") {
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, Text\n"
                                   "Comment: une note du traducteur\n"
                                   "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::IgnoredLine));
}

TEST_CASE("a file without a single dialogue line is refused", "[format][ssa]") {
    const std::expected<ReadResult, ReadError> refused =
        SubStationAlphaReader{SubtitleFormat::SubStationAlpha}.read(
            "[Script Info]\nScriptType: v4.00\n[Events]\nFormat: Marked, Start, End, Text\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a margin that is not a number is reported rather than read as zero", "[format][ssa]") {
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, MarginL, Text\n"
                                   "Dialogue: Marked=0,0:00:01.00,0:00:03.00,large,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::UnknownEventField));
}

TEST_CASE("an events section with nothing in it is refused", "[format][ssa]") {
    // No columns declared and no dialogue: the reading falls back on the
    // columns its format uses, finds nothing to fill them with, and says so.
    const std::expected<ReadResult, ReadError> refused =
        SubStationAlphaReader{SubtitleFormat::SubStationAlpha}.read(
            "[Script Info]\nScriptType: v4.00\n[Events]\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("the blank line before the events is dropped, as the writing puts it back",
          "[format][ssa]") {
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n\n\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, Text\n"
                                   "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Bonjour.\n");

    CHECK(result.header == "[Script Info]\nScriptType: v4.00");
}

TEST_CASE("an empty column where a number belongs is reported", "[format][ssa]") {
    // Empty is not zero: a file that wrote nothing there said nothing, and
    // answering zero would be inventing a margin the author did not give.
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, MarginL, Text\n"
                                   "Dialogue: Marked=0,0:00:01.00,0:00:03.00,,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::UnknownEventField));
}

TEST_CASE("a number too long to hold is reported rather than wrapped", "[format][ssa]") {
    const ReadResult result =
        read("[Script Info]\nScriptType: v4.00\n"
             "[Events]\n"
             "Format: Marked, Start, End, MarginL, Text\n"
             "Dialogue: Marked=0,0:00:01.00,0:00:03.00,12345678901,Bonjour.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(hasDiagnostic(result, DiagnosticKind::UnknownEventField));
}

TEST_CASE("an event line shorter than its columns keeps what it has", "[format][ssa]") {
    // The split stops when the commas run out. What was written is read; what
    // was not is left at the value a subtitle from nowhere would have.
    const ReadResult result = read("[Script Info]\nScriptType: v4.00\n"
                                   "[Events]\n"
                                   "Format: Marked, Start, End, Style, Text\n"
                                   "Dialogue: Marked=0,0:00:01.00,0:00:03.00\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText.empty());
}
