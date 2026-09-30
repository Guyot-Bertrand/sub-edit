// The unknown words of a text, as the cell editor underlines them — issue #525,
// `GUI-SPELL-04`, over a dictionary written in the test.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/spell_check_navigator.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_ranges.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::misspelledSpellRanges;
using subedit::core::openSpellChecker;
using subedit::core::SpellChecker;
using subedit::core::SpellCheckNavigator;
using subedit::core::SpellRange;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;

SpellChecker checkerOver(const std::string& language, const std::vector<std::string>& words) {
    WordList list;
    list.words.insert(words.begin(), words.end());
    WordListSpellProvider provider;
    provider.add(language, std::move(list));
    const InMemoryFileSystem files;
    return openSpellChecker(provider, language, files, "/none.repl").value();
}

} // namespace

TEST_CASE("GUI-SPELL-04: the unknown words are given as byte ranges, accents counted in bytes",
          "[text][spell][ranges][GUI-SPELL-04]") {
    const SpellChecker checker = checkerOver("fr", {"un", "été", "beau"});

    // "été" is five bytes: the range of "bau" starts after them.
    const std::string text = "un été bau, xyzzy";

    CHECK(misspelledSpellRanges(checker, text) ==
          std::vector<SpellRange>{{.offset = 9, .length = 3}, {.offset = 14, .length = 5}});
    CHECK(misspelledSpellRanges(checker, "un beau été").empty());
    CHECK(misspelledSpellRanges(checker, "").empty());
}

TEST_CASE("GUI-SPELL-04: the English heuristics and the apostrophes are the checker's",
          "[text][spell][ranges][GUI-SPELL-04]") {
    const SpellChecker checker = checkerOver("en", {"going", "we", "dog"});

    // `goin'` is `going`, `we'd` and `dog's` are checked without the suffix,
    // `1st` is an ordinal; `cat's` is not, and `goin` alone is not.
    const std::string text = "goin' we'd dog's 1st cat's goin";

    CHECK(misspelledSpellRanges(checker, text) ==
          std::vector<SpellRange>{{.offset = 21, .length = 5}, {.offset = 27, .length = 4}});
}

TEST_CASE("GUI-SPELL-04: the underline stops where the walk of the spell check stops",
          "[text][spell][ranges][GUI-SPELL-04]") {
    const std::string text = "goin' cat's un xyzzy bonn";
    const std::vector<std::string> known{"going", "un", "bon", "dog"};

    const SpellChecker checker = checkerOver("en", known);
    SpellCheckNavigator navigator{checkerOver("en", known)};
    navigator.reset(text);

    std::vector<SpellRange> walked;
    while (const auto stop = navigator.next()) {
        walked.push_back({.offset = stop->offset, .length = stop->word.size()});
        navigator.ignore();
    }

    CHECK(walked == misspelledSpellRanges(checker, text));
    CHECK_FALSE(walked.empty());
}
