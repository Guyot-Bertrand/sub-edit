// Joining and splitting words by the spell-checker — issue #508, decisions
// D6 and D8. Over a dictionary written in the test: no dictionary of the
// machine is read.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/join_split_words.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <string>
#include <vector>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::joinWords;
using subedit::core::openSpellChecker;
using subedit::core::SpellChecker;
using subedit::core::splitWords;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;

SpellChecker checkerOver(const std::vector<std::string>& words,
                         const std::map<std::string, std::vector<std::string>>& suggestions = {}) {
    WordList list;
    list.words.insert(words.begin(), words.end());
    for (const auto& [word, offered] : suggestions)
        list.suggestions.emplace(word, offered);
    WordListSpellProvider provider;
    provider.add("fr", std::move(list));
    const InMemoryFileSystem files;
    return openSpellChecker(provider, "fr", files, "/none.repl").value();
}

std::string joined(const SpellChecker& checker, const std::string& text) {
    return joinWords(checker, std::vector<std::string>{text}).front();
}

std::string split(const SpellChecker& checker, const std::string& text) {
    return splitWords(checker, std::vector<std::string>{text}).front();
}

} // namespace

TEST_CASE("a word is joined forwards when only that direction spells",
          "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"bonjour"});

    CHECK(joined(checker, "bon jour") == "bonjour");
}

TEST_CASE("a word is joined backwards when only that direction spells",
          "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"bon", "bonjour"});

    CHECK(joined(checker, "bon jour") == "bonjour");
}

TEST_CASE("nothing is joined when both directions spell", "[text][spell][join-split]") {
    // `bb` is misspelt; `aabb` and `bbcc` are both words.
    const SpellChecker checker = checkerOver({"aa", "cc", "aabb", "bbcc"});

    CHECK(joined(checker, "aa bb cc") == "aa bb cc");
}

TEST_CASE("nothing is joined when neither direction spells", "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({});

    CHECK(joined(checker, "xx yy") == "xx yy");
}

TEST_CASE("a word next to punctuation has no word to join", "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"bonjour"});

    CHECK(joined(checker, "bon, jour") == "bon, jour");
}

TEST_CASE("runs of spaces are collapsed for the join, and only if something was joined",
          "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"bonjour", "aa", "bb"});

    CHECK(joined(checker, "bon   jour") == "bonjour");
    CHECK(joined(checker, "aa   bb") == "aa   bb"); // nothing joined: as it was
}

TEST_CASE("a join is kept when a later misspelt word is left alone", "[text][spell][join-split]") {
    // Gaupol compares with the text as it stood at the last misspelt word,
    // and loses this join; ours does not.
    const SpellChecker checker = checkerOver({"bonjour"});

    CHECK(joined(checker, "bon jour zzzz") == "bonjour zzzz");
}

TEST_CASE("several joins in one text", "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"bonjour", "merci", "beaucoup"});

    CHECK(joined(checker, "bon jour et mer ci beau coup") == "bonjour et merci beaucoup");
}

TEST_CASE("one text in, one text out, in order", "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"bonjour"});

    const std::vector<std::string> done =
        joinWords(checker, std::vector<std::string>{"bon jour", "rien", ""});

    CHECK(done == std::vector<std::string>{"bonjour", "rien", ""});
}

TEST_CASE("a word is split when exactly one suggestion is it with a space",
          "[text][spell][join-split]") {
    const SpellChecker checker =
        checkerOver({}, {{"bonjourtous", {"bonjour tous", "bonjourtout"}}});

    CHECK(split(checker, "dis bonjourtous ici") == "dis bonjour tous ici");
}

TEST_CASE("a word is not split when two suggestions are it with spaces",
          "[text][spell][join-split]") {
    const SpellChecker checker =
        checkerOver({}, {{"bonjourtous", {"bonjour tous", "bon jourtous"}}});

    CHECK(split(checker, "bonjourtous") == "bonjourtous");
}

TEST_CASE("a word is not split when no suggestion is it with a space",
          "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({}, {{"bonjourtous", {"bonjourtout", "bonjours"}}});

    CHECK(split(checker, "bonjourtous") == "bonjourtous");
    CHECK(split(checker, "inconnu") == "inconnu"); // no suggestion at all
}

TEST_CASE("a replacement made before counts as a suggestion", "[text][spell][join-split]") {
    SpellChecker checker = checkerOver({});
    checker.addReplacement("bonjourtous", "bonjour tous");

    CHECK(split(checker, "bonjourtous") == "bonjour tous");
}

TEST_CASE("a word with a capital and lower case after it is left alone",
          "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({},
                                             {{"Bonjourtous", {"Bonjour tous"}},
                                              {"McDonald", {"Mc Donald"}},
                                              {"BONJOURTOUS", {"BONJOUR TOUS"}}});

    CHECK(split(checker, "Bonjourtous") == "Bonjourtous");  // a name, perhaps
    CHECK(split(checker, "McDonald") == "Mc Donald");       // not title case: split
    CHECK(split(checker, "BONJOURTOUS") == "BONJOUR TOUS"); // upper case is not title case
}

TEST_CASE("an apostrophe is not title case", "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({}, {{"Don'tgo", {"Don't go"}}});

    CHECK(split(checker, "Don'tgo") == "Don't go");
}

TEST_CASE("runs of spaces are collapsed for the split, and only if something was split",
          "[text][spell][join-split]") {
    const SpellChecker checker = checkerOver({"aa", "bb"}, {{"bonjourtous", {"bonjour tous"}}});

    CHECK(split(checker, "bonjourtous   ici") == "bonjour tous ici");
    CHECK(split(checker, "aa   bb") == "aa   bb");
}
