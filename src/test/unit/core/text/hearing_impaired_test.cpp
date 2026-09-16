// The rule of phase 4, confronted with the corpus that decided it.
//
// One line of test for the whole corpus, which is what `mentions.cas` was
// written for: the cases were settled before the transformation existed, and
// the day it exists nothing has to be transcribed into C++. A case added the
// day a question is settled becomes a failing test on its own.
//
// The corpus is the specification, and it is written in SubRip. What is
// written here besides is how the other vocabularies judge what shows.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/hearing_impaired.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

#include "text_cases.hpp"

namespace {

using subedit::core::SubtitleFormat;
using subedit::core::withoutHearingImpaired;

} // namespace

TEST_CASE("removing hearing impaired mentions follows the corpus of cases", "[text]") {
    subedit::test::checkTextCases(subedit::test::textCasesOf("textes/mentions.cas"),
                                  [](const std::string& text) {
                                      return withoutHearingImpaired(text, SubtitleFormat::SubRip);
                                  });
}

TEST_CASE("a subtitle left with only the tags of its format goes", "[text]") {
    // Issue #403: the analyser knew `<…>` alone, whatever the format, so an
    // Advanced SSA subtitle that held nothing but a mention kept its two braces
    // and stayed, empty, where a SubRip one went.
    CHECK_FALSE(
        withoutHearingImpaired(R"({\i1}[SOUPIR]{\i0})", SubtitleFormat::AdvancedSubStationAlpha)
            .has_value());
    CHECK_FALSE(withoutHearingImpaired("{Y:i}[SOUPIR]", SubtitleFormat::MicroDvd).has_value());
    CHECK_FALSE(withoutHearingImpaired("/[SOUPIR]", SubtitleFormat::Mpl2).has_value());
}

TEST_CASE("what is a tag in one format is text in another", "[text]") {
    // A TMPlayer line writes no tag, so `<i>` there is three characters a
    // viewer reads — and the seam between them and what follows the mention is
    // the one space any two things that show get.
    CHECK(
        withoutHearingImpaired("<i>[SOUPIR]</i>", SubtitleFormat::TMPlayer).value_or("<absent>") ==
        "<i> </i>");
    // And an MPL2 marker is one at the head of a line only: after a mention, a
    // slash is a slash.
    CHECK(withoutHearingImpaired("Oui [SOUPIR]/", SubtitleFormat::Mpl2).value_or("<absent>") ==
          "Oui /");
}

TEST_CASE("brackets inside a tag are not a mention", "[text]") {
    // The scan looked for `(` letter by letter, tags included: the position of
    // an Advanced SSA subtitle, `\pos(320,50)`, was taken for a mention and
    // cut out of a subtitle that held none.
    CHECK(withoutHearingImpaired(R"({\an8\pos(320,50)}Bonjour)",
                                 SubtitleFormat::AdvancedSubStationAlpha)
              .value_or("<absent>") == R"({\an8\pos(320,50)}Bonjour)");
    CHECK(withoutHearingImpaired(R"({\pos(1,2)}[SOUPIR] Bonjour)",
                                 SubtitleFormat::AdvancedSubStationAlpha)
              .value_or("<absent>") == R"({\pos(1,2)}Bonjour)");
}
