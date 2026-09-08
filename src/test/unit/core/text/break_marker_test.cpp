// The marker each format writes where a subtitle holds a line break.
//
// Four of the nine cannot hold a real break — their subtitles are one file line
// long — and each names a stand-in. The stand-ins differ; the idea does not,
// which is why this is written once.

#include <subedit/core/text/break_marker.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using subedit::core::textFromMarker;
using subedit::core::textToMarker;
namespace marker = subedit::core::break_marker;

} // namespace

TEST_CASE("a marker becomes a break, and a break becomes a marker", "[text][breaks]") {
    CHECK(textFromMarker("une ligne[br]une autre", marker::kBracketed) == "une ligne\nune autre");
    CHECK(textToMarker("une ligne\nune autre", marker::kBracketed) == "une ligne[br]une autre");

    CHECK(textFromMarker("une ligne|une autre", marker::kPipe) == "une ligne\nune autre");
    CHECK(textToMarker("une ligne\nune autre", marker::kPipe) == "une ligne|une autre");
}

TEST_CASE("a text with no break is handed back as it was", "[text][breaks]") {
    CHECK(textFromMarker("une seule ligne", marker::kPipe) == "une seule ligne");
    CHECK(textToMarker("une seule ligne", marker::kPipe) == "une seule ligne");
    CHECK(textFromMarker("", marker::kPipe).empty());
}

TEST_CASE("two markers in a row make two breaks, and an empty line between", "[text][breaks]") {
    CHECK(textFromMarker("une||autre", marker::kPipe) == "une\n\nautre");
    CHECK(textToMarker("une\n\nautre", marker::kPipe) == "une||autre");
}

TEST_CASE("a marker at either end leaves an empty line there", "[text][breaks]") {
    // What a file writes at the edge is what the text gets: nothing is trimmed,
    // because trimming would lose a line the author put there.
    CHECK(textFromMarker("|une", marker::kPipe) == "\nune");
    CHECK(textFromMarker("une|", marker::kPipe) == "une\n");
}

TEST_CASE("the round trip through a marker is exact", "[text][breaks]") {
    // The property both readers and both writers rest on. It holds because the
    // text of a subtitle never holds the marker itself — and where it would,
    // the file could not have written it either.
    for (const std::string text : {"une ligne", "une\ndeux", "une\n\ntrois", "\nau début"}) {
        CHECK(textFromMarker(textToMarker(text, marker::kPipe), marker::kPipe) == text);
        CHECK(textFromMarker(textToMarker(text, marker::kBracketed), marker::kBracketed) == text);
    }
}
