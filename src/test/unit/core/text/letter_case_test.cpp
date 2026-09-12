// The four cases, held to the corpora written for them.
//
// One file per case, because the harness runs one transformation at a time and
// a file says which. The rule they share — what precedes the first letter is
// left alone — is written once, in `casse-titre.cas`.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/letter_case.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <text_cases.hpp>

namespace {

using subedit::core::kLetterCases;
using subedit::core::LetterCase;
using subedit::core::recased;
using subedit::core::SubtitleFormat;
using subedit::test::checkTextCases;
using subedit::test::textCasesOf;

void runs(const std::string& corpus, LetterCase wanted) {
    INFO("corpus : " << corpus);
    checkTextCases(textCasesOf(corpus), [wanted](const std::string& text) {
        return std::optional<std::string>{recased(text, wanted, SubtitleFormat::SubRip)};
    });
}

} // namespace

TEST_CASE("the four corpora of case pass", "[text][case]") {
    runs("textes/casse-titre.cas", LetterCase::Title);
    runs("textes/casse-phrase.cas", LetterCase::Sentence);
    runs("textes/casse-capitales.cas", LetterCase::Upper);
    runs("textes/casse-minuscules.cas", LetterCase::Lower);
}

TEST_CASE("the four are four, and none of them is another", "[text][case]") {
    // A text that reads differently under each: a switch arm wired to the wrong
    // mapping would collapse two of them into one, and no single corpus above
    // would notice.
    std::set<std::string> seen;
    for (const LetterCase wanted : kLetterCases)
        seen.insert(recased("bONJOUR mARIE", wanted, SubtitleFormat::SubRip));

    CHECK(seen.size() == 4);
}

TEST_CASE("a text with nothing to case comes back as the bytes it was", "[text][case]") {
    // The reassembly is not the identity — the parser moves tags — so an
    // operation that changes nothing must not go through it.
    constexpr const char* kOdd = "<i >123</I > !";

    for (const LetterCase wanted : kLetterCases) {
        INFO("casse : " << static_cast<int>(wanted));
        CHECK(recased(kOdd, wanted, SubtitleFormat::SubRip) == kOdd);
    }
}

TEST_CASE("a format writing no tag cases everything it holds", "[text][case]") {
    // An LRC carries no tag, so `<i>` in it is three characters of text — and
    // they take the case like the rest.
    CHECK(recased("<i>bonjour", LetterCase::Upper, SubtitleFormat::Lrc) == "<I>BONJOUR");
}

TEST_CASE("a letter whose capital is two letters keeps its small form", "[text][case]") {
    // The German `ß` capitalises to `SS`. Writing two letters where there was
    // one would be defensible and surprising; Gaupol leaves it, and so do we.
    CHECK(recased("ßonjour", LetterCase::Sentence, SubtitleFormat::SubRip) == "ßonjour");
    // Asked for capitals outright, it does grow — that is a different question.
    CHECK(recased("ßonjour", LetterCase::Upper, SubtitleFormat::SubRip) == "SSONJOUR");
}
