// The rule of phase 4, confronted with the corpus that decided it.
//
// One line of test for the whole corpus, which is what `mentions.cas` was
// written for: the cases were settled before the transformation existed, and
// the day it exists nothing has to be transcribed into C++. A case added the
// day a question is settled becomes a failing test on its own.
//
// The corpus is the specification. What is written here is only the wiring.

#include <subedit/core/text/hearing_impaired.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>

#include "text_cases.hpp"

TEST_CASE("removing hearing impaired mentions follows the corpus of cases", "[text]") {
    subedit::test::checkTextCases(
        subedit::test::textCasesOf("textes/mentions.cas"),
        [](const std::string& text) { return subedit::core::withoutHearingImpaired(text); });
}
