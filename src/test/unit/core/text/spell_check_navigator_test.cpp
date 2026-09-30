// The spell-check navigator — issue #509, decision D6: Gaupol's
// `SpellCheckNavigator`, over a dictionary written in the test.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/spell_check_navigator.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::openSpellChecker;
using subedit::core::SpellCheckNavigator;
using subedit::core::SpellWord;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;

SpellCheckNavigator
navigatorOver(const std::vector<std::string>& words,
              const std::vector<std::pair<std::string, std::string>>& suggestions = {}) {
    WordList list;
    list.words.insert(words.begin(), words.end());
    for (const auto& [word, offered] : suggestions)
        list.suggestions[word].push_back(offered);
    WordListSpellProvider provider;
    provider.add("fr", std::move(list));
    const InMemoryFileSystem files;
    return SpellCheckNavigator{openSpellChecker(provider, "fr", files, "/none.repl").value()};
}

/// The word / offset of a stop, empty / zero when there is none: what a test
/// reads after it has said, on its own line, that there is one.
std::string wordOf(const std::optional<SpellWord>& stop) {
    return stop.has_value() ? stop->word : std::string{};
}

std::size_t offsetOf(const std::optional<SpellWord>& stop) {
    return stop.has_value() ? stop->offset : 0;
}

} // namespace

TEST_CASE("the navigator finds unknown words in order", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"un", "bon", "jour"});
    nav.reset("un bonn jouur bon");

    CHECK(nav.next() == std::optional<SpellWord>{SpellWord{.offset = 3, .word = "bonn"}});
    CHECK(nav.pos() == 3);
    CHECK(nav.endPos() == 7);
    nav.ignore();
    CHECK(nav.next() == std::optional<SpellWord>{SpellWord{.offset = 8, .word = "jouur"}});
    nav.ignore();
    CHECK_FALSE(nav.next().has_value());
}

TEST_CASE("the navigator counts positions in bytes of accented text", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"le", "été"});
    nav.reset("été le évéé");

    // "été le " is 4 + 1 + 2 + 1 = 8 bytes.
    const auto found = nav.next();
    REQUIRE(found.has_value());
    CHECK(offsetOf(found) == 9);
    CHECK(wordOf(found) == "évéé");
    CHECK(nav.endPos() == nav.text().size());
}

TEST_CASE("replace changes the text and moves past the replacement", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"le", "café", "chaud"});
    nav.reset("le cafe est chaud");

    REQUIRE(nav.next().has_value());
    nav.replace("café");
    CHECK(nav.text() == "le café est chaud");
    CHECK(nav.pos() == 8);
    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "est");
    CHECK(nav.checker().replacements().size() == 1);
}

TEST_CASE("replace all reaches later words and later texts", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok", "bien", "salut"});
    nav.reset("salut sallut bien sallut ok zzzz");

    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "sallut");
    nav.replaceAll("salut");
    const auto stop = nav.next();
    REQUIRE(stop.has_value());
    CHECK(wordOf(stop) == "zzzz");
    CHECK(nav.text() == "salut salut bien salut ok zzzz");

    // The memory survives a reset: the next text is corrected on the way.
    nav.reset("sallut zzzz");
    const auto again = nav.next();
    REQUIRE(again.has_value());
    CHECK(wordOf(again) == "zzzz");
    CHECK(nav.text() == "salut zzzz");
}

TEST_CASE("ignore all silences the word for the rest of the session", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"});
    nav.reset("ok qqq ok qqq yyy");

    REQUIRE(nav.next().has_value());
    nav.ignoreAll();
    const auto stop = nav.next();
    REQUIRE(stop.has_value());
    CHECK(wordOf(stop) == "yyy");
    // Ignoring one instance only would have stopped on the second.
}

TEST_CASE("ignore alone stops on the next instance", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"});
    nav.reset("qqq ok qqq");

    REQUIRE(nav.next().has_value());
    nav.ignore();
    const auto stop = nav.next();
    REQUIRE(stop.has_value());
    CHECK(wordOf(stop) == "qqq");
    CHECK(offsetOf(stop) == 7);
}

TEST_CASE("add puts the word in the personal list", "[text][spell][navigator]") {
    WordList list;
    list.words.insert("ok");
    const auto personal = list.personal;
    WordListSpellProvider provider;
    provider.add("fr", std::move(list));
    const InMemoryFileSystem files;
    SpellCheckNavigator nav{openSpellChecker(provider, "fr", files, "/none.repl").value()};
    nav.reset("ok qqq");

    REQUIRE(nav.next().has_value());
    nav.add();
    REQUIRE(personal->size() == 1);
    CHECK(personal->front() == "qqq");
}

TEST_CASE("join with next removes the white space after the word", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"});
    nav.reset("ok bon  \t jour ok");

    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "bon");
    CHECK(nav.spaceAfter());
    CHECK(nav.spaceBefore());
    nav.joinWithNext();
    CHECK(nav.text() == "ok bonjour ok");
    CHECK_FALSE(nav.spaceAfter());
}

TEST_CASE("join with previous removes the white space before and rewinds",
          "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"});
    nav.reset("ok bon jour ok");

    REQUIRE(nav.next().has_value());
    nav.ignore();
    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "jour");
    CHECK(nav.spaceBefore());
    nav.joinWithPrevious();
    CHECK(nav.text() == "ok bonjour ok");
    // Rewound to the start of the compound, so the next search finds it.
    CHECK(nav.pos() == 3);
    const auto stop = nav.next();
    REQUIRE(stop.has_value());
    CHECK(wordOf(stop) == "bonjour");
}

TEST_CASE("joining steps over whole accented characters", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"});
    nav.reset("ok été fête ok");

    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "été");
    nav.ignore();
    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "fête");
    // U+00A0 is white space and two bytes long.
    CHECK(nav.spaceBefore());
    nav.joinWithPrevious();
    CHECK(nav.text() == "ok étéfête ok");
    CHECK(nav.pos() == 3);

    nav.reset("é été");
    REQUIRE(nav.next().has_value());
    CHECK(nav.word() == "été");
    nav.joinWithPrevious();
    // The lone "é" is not a word, but it is alphanumeric: the compound takes it in.
    CHECK(nav.text() == "éété");
    CHECK(nav.pos() == 0);
}

TEST_CASE("a neighbour that is not white space is not one", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"});
    nav.reset("é,qqq!");

    REQUIRE(nav.next().has_value());
    CHECK_FALSE(nav.spaceBefore());
    CHECK_FALSE(nav.spaceAfter());
}

TEST_CASE("suggest asks the checker about the current word", "[text][spell][navigator]") {
    SpellCheckNavigator nav = navigatorOver({"ok"}, {{"bnjour", "bonjour"}});
    nav.reset("ok bnjour");

    REQUIRE(nav.next().has_value());
    CHECK(nav.suggest() == std::vector<std::string>{"bonjour"});
}
