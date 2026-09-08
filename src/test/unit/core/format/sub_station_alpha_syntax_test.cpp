// The grammar the two Sub Station Alpha formats share.

#include <subedit/core/format/sub_station_alpha_syntax.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::defaultEventFields;
using subedit::core::isKnownEventField;
using subedit::core::splitEventLine;
using subedit::core::SubtitleFormat;
using subedit::core::textFromEvent;
using subedit::core::textToEvent;

} // namespace

TEST_CASE("the two formats declare the same columns but the first", "[format][ssa]") {
    const std::vector<std::string> ssa = defaultEventFields(SubtitleFormat::SubStationAlpha);
    const std::vector<std::string> ass =
        defaultEventFields(SubtitleFormat::AdvancedSubStationAlpha);

    REQUIRE(ssa.size() == 10);
    REQUIRE(ass.size() == 10);
    CHECK(ssa.front() == "Marked");
    CHECK(ass.front() == "Layer");
    CHECK(std::vector<std::string>(ssa.begin() + 1, ssa.end()) ==
          std::vector<std::string>(ass.begin() + 1, ass.end()));
}

TEST_CASE("a column this library cannot fill is not a known one", "[format][ssa]") {
    CHECK(isKnownEventField("Start"));
    CHECK(isKnownEventField("MarginV"));
    CHECK_FALSE(isKnownEventField("Karaoke"));
    CHECK_FALSE(isKnownEventField(""));
}

TEST_CASE("splitting stops one comma short of the column count", "[format][ssa]") {
    const std::vector<std::string_view> three = splitEventLine("a,b,c,d,e", 3);

    REQUIRE(three.size() == 3);
    CHECK(three[0] == "a");
    CHECK(three[1] == "b");
    CHECK(three[2] == "c,d,e");
}

TEST_CASE("splitting a line with no column asks for nothing", "[format][ssa]") {
    // A guard rather than a case that happens: a `Format:` line with no name in
    // it would otherwise hand back one column holding the whole line.
    CHECK(splitEventLine("a,b,c", 0).empty());
}

TEST_CASE("splitting a line shorter than its columns stops where it runs out", "[format][ssa]") {
    // Two commas short of five columns gives two, not five padded with nothing:
    // what the line holds is what is read, and the caller fills the rest with
    // the values a subtitle from nowhere would have.
    const std::vector<std::string_view> found = splitEventLine("a,b", 5);

    REQUIRE(found.size() == 2);
    CHECK(found[0] == "a");
    CHECK(found[1] == "b");
}

TEST_CASE("the two break markers read the same, and one is written", "[format][ssa]") {
    // Sub Station Alpha means `\\n` as a soft break and `\\N` as a hard one.
    // Nothing here tells them apart, and Gaupol does not either.
    CHECK(textFromEvent("une ligne\\Nune autre\\nune troisième") ==
          "une ligne\nune autre\nune troisième");
    CHECK(textToEvent("une ligne\nune autre") == "une ligne\\Nune autre");

    // A backslash that opens nothing is text, and comes back as text.
    CHECK(textFromEvent("un chemin\\vers") == "un chemin\\vers");
}
