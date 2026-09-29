// Gaupol's spell-check tokenizer, ported — issue #507, decision D6.

#include <subedit/core/text/spell_tokenizer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::SpellWord;
using subedit::core::tokenizeForSpelling;

std::vector<std::string> wordsOf(const std::string& text) {
    std::vector<std::string> words;
    for (const SpellWord& one : tokenizeForSpelling(text))
        words.push_back(one.word);
    return words;
}

} // namespace

TEST_CASE("words are runs of letters and digits, at their byte offsets", "[text][spell]") {
    const std::vector<SpellWord> words = tokenizeForSpelling("Hello, big world!");

    REQUIRE(words.size() == 3);
    CHECK(words[0] == SpellWord{0, "Hello"});
    CHECK(words[1] == SpellWord{7, "big"});
    CHECK(words[2] == SpellWord{11, "world"});
}

TEST_CASE("an apostrophe stays inside a word, and trailing ones do not", "[text][spell]") {
    CHECK(wordsOf("don't stop") == std::vector<std::string>{"don't", "stop"});
    // `rock'n'` is cut at the space, then loses its trailing apostrophe.
    CHECK(wordsOf("rock'n' roll") == std::vector<std::string>{"rock'n", "roll"});
    // A comma ends the word even when an apostrophe follows it.
    CHECK(wordsOf("a,'bc") == std::vector<std::string>{"bc"});
}

TEST_CASE("a word of one character, or of digits alone, is not one", "[text][spell]") {
    CHECK(wordsOf("I a 42 x2 12th").size() == 2);
    CHECK(wordsOf("I a 42 x2 12th") == std::vector<std::string>{"x2", "12th"});
}

TEST_CASE("an underscore is a word character", "[text][spell]") {
    CHECK(wordsOf("snake_case here") == std::vector<std::string>{"snake_case", "here"});
}

TEST_CASE("offsets count bytes, and accented letters are letters", "[text][spell]") {
    const std::vector<SpellWord> words = tokenizeForSpelling("été là");

    REQUIRE(words.size() == 2);
    CHECK(words[0] == SpellWord{0, "été"});
    CHECK(words[1] == SpellWord{6, "là"}); // "été" is five bytes, then a space
}

TEST_CASE("a text with no word gives none", "[text][spell]") {
    CHECK(tokenizeForSpelling("").empty());
    CHECK(tokenizeForSpelling("... -- !!").empty());
}
