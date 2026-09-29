// The spell-checker over a dictionary written in the test — issue #507,
// decision D6. No test here depends on the dictionaries of the machine.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

using subedit::core::availableSpellLanguages;
using subedit::core::InMemoryFileSystem;
using subedit::core::isValidSpellLanguage;
using subedit::core::NoDictionary;
using subedit::core::noDictionaryFor;
using subedit::core::openSpellChecker;
using subedit::core::SpellChecker;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;

/// A checker for `language` over `words`, with `suggestions` for what it does
/// not know.
SpellChecker checkerOver(const std::string& language,
                         const std::vector<std::string>& words,
                         const std::map<std::string, std::vector<std::string>>& suggestions = {}) {
    WordList list;
    list.words.insert(words.begin(), words.end());
    for (const auto& [word, offered] : suggestions)
        list.suggestions.emplace(word, offered);
    WordListSpellProvider provider;
    provider.add(language, std::move(list));
    const InMemoryFileSystem files;
    return openSpellChecker(provider, language, files, "/none.repl").value();
}

} // namespace

TEST_CASE("a word is correct if the dictionary has it", "[text][spell]") {
    const SpellChecker checker = checkerOver("fr", {"bonjour"});

    CHECK(checker.check("bonjour"));
    CHECK_FALSE(checker.check("bonjuor"));
}

TEST_CASE("ignoring a word for the session makes it correct, and only in that checker",
          "[text][spell]") {
    SpellChecker checker = checkerOver("fr", {"bonjour"});
    const SpellChecker other = checkerOver("fr", {"bonjour"});

    checker.addToSession("Zorglub");

    CHECK(checker.check("Zorglub"));
    CHECK_FALSE(other.check("Zorglub"));
}

TEST_CASE("adding a word to the personal list reaches the dictionary", "[text][spell]") {
    WordList list;
    const std::shared_ptr<std::vector<std::string>> personal = list.personal;
    WordListSpellProvider provider;
    provider.add("fr", std::move(list));
    const InMemoryFileSystem files;
    SpellChecker checker = openSpellChecker(provider, "fr", files, "/none.repl").value();

    checker.addToPersonal("Zorglub");

    CHECK(*personal == std::vector<std::string>{"Zorglub"});
    CHECK(checker.check("Zorglub"));
}

TEST_CASE("English: a word cut before its g is also tried with it", "[text][spell]") {
    const SpellChecker checker = checkerOver("en_US", {"going"});

    CHECK(checker.check("goin", "", "'"));
    CHECK_FALSE(checker.check("goin", "", " "));
    CHECK_FALSE(checker.check("goin"));
}

TEST_CASE("English: each contraction suffix is stripped to check the main word", "[text][spell]") {
    const SpellChecker checker = checkerOver("en_GB", {"we", "dog"});

    for (const char* word : {"we'd", "we'll", "we're", "we've", "dog's"})
        CHECK(checker.check(word));
    CHECK_FALSE(checker.check("cat's")); // the main word has to be correct too
}

TEST_CASE("English: ordinal numerals are correct", "[text][spell]") {
    const SpellChecker checker = checkerOver("en", {});

    for (const char* word : {"1st",
                             "2nd",
                             "3rd",
                             "4th",
                             "0th",
                             "9th",
                             "11th",
                             "12th",
                             "13th",
                             "21st",
                             "22nd",
                             "23rd",
                             "101st",
                             "111th",
                             "112th",
                             "100th"})
        CHECK(checker.check(word));
    for (const char* word :
         {"11st", "12nd", "13rd", "21th", "22th", "1th", "5st", "th", "a1st", "ab5th"})
        CHECK_FALSE(checker.check(word));
}

TEST_CASE("the English heuristics are for English alone", "[text][spell]") {
    const SpellChecker checker = checkerOver("fr", {"going", "we"});

    CHECK_FALSE(checker.check("goin", "", "'"));
    CHECK_FALSE(checker.check("we'll"));
    CHECK_FALSE(checker.check("1st"));
}

TEST_CASE("a suggestion is a replacement made before, then the OCR ones, then the dictionary's",
          "[text][spell]") {
    SpellChecker checker = checkerOver("en", {"look", "5", "km"}, {{"Iook", {"took", "book"}}});
    checker.addReplacement("Iook", "gaze");

    CHECK(checker.suggest("Iook") == std::vector<std::string>{"gaze", "look", "took", "book"});
}

TEST_CASE("an I read for an l is suggested only if the result is correct", "[text][spell]") {
    const SpellChecker checker = checkerOver("fr", {"lundi"});

    CHECK(checker.suggest("Iundi") == std::vector<std::string>{"lundi"});
    CHECK(checker.suggest("Izzz").empty());
}

TEST_CASE("a number stuck to its unit is suggested spaced out, if both halves are correct",
          "[text][spell]") {
    const SpellChecker checker = checkerOver("en", {"5", "km"});

    CHECK(checker.suggest("5km") == std::vector<std::string>{"5 km"});
    CHECK(checker.suggest("5zz").empty());
    CHECK(checker.suggest("km5").empty());
    CHECK(checker.suggest("5a5").empty()); // a digit after the unit is no unit
    CHECK(checker.suggest("55").empty());
}

TEST_CASE("suggestions carry no duplicate", "[text][spell]") {
    SpellChecker checker = checkerOver("fr", {}, {{"mot", {"mots", "mot", "mots"}}});
    checker.addReplacement("mot", "mots");
    checker.addReplacement("mot", "mots");

    CHECK(checker.suggest("mot") == std::vector<std::string>{"mots", "mot"});
}

TEST_CASE("all and any of a few words", "[text][spell]") {
    const SpellChecker checker = checkerOver("fr", {"un", "deux"});

    CHECK(checker.checkAll(std::vector<std::string>{"un", "deux"}));
    CHECK_FALSE(checker.checkAll(std::vector<std::string>{"un", "trois"}));
    CHECK(checker.checkAny(std::vector<std::string>{"trois", "deux"}));
    CHECK_FALSE(checker.checkAny(std::vector<std::string>{"trois", "quatre"}));
}

TEST_CASE("a language with no dictionary says so", "[text][spell]") {
    WordListSpellProvider provider;
    provider.add("en", WordList{});
    const InMemoryFileSystem files;

    const auto opened = openSpellChecker(provider, "fr", files, "/none.repl");

    REQUIRE_FALSE(opened.has_value());
    CHECK(opened.error() == NoDictionary{"fr"});
    CHECK(noDictionaryFor(opened.error().language) == "no dictionary for fr");
}

TEST_CASE("the languages offered are the valid locale codes, sorted", "[text][spell]") {
    WordListSpellProvider provider;
    for (const char* code :
         {"fr_FR", "en-variant_0", "en", "en_w_accents", "sr@Latn", "de_DE", "EN"})
        provider.add(code, WordList{});

    CHECK(availableSpellLanguages(provider) ==
          std::vector<std::string>{"de_DE", "en", "fr_FR", "sr@Latn"});
}

TEST_CASE("locale codes: language, country and script", "[text][spell]") {
    for (const char* code : {"fr", "en_US", "sr@Latn", "sr_RS@Latn"})
        CHECK(isValidSpellLanguage(code));
    for (const char* code : {"", "f", "FR", "en_us", "en_USA", "en-US", "sr@latn", "sr@Lat", "fr "})
        CHECK_FALSE(isValidSpellLanguage(code));
}
