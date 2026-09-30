// The per-language replacement list — issue #507, decision D6.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_replacements.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::kMaxSpellReplacements;
using subedit::core::openSpellChecker;
using subedit::core::parseSpellReplacements;
using subedit::core::renderSpellReplacements;
using subedit::core::saveSpellReplacements;
using subedit::core::SpellReplacement;
using subedit::core::spellReplacementFile;
using subedit::core::WordList;
using subedit::core::WordListSpellProvider;

} // namespace

TEST_CASE("the replacement file lives in the given configuration directory", "[text][spell]") {
    CHECK(spellReplacementFile("/config/subedit", "fr_FR") ==
          std::filesystem::path{"/config/subedit/spell-check/fr_FR.repl"});
}

TEST_CASE("replacements are read line by line, split at the first bar", "[text][spell]") {
    const std::vector<SpellReplacement> read =
        parseSpellReplacements("teh|the\r\nwierd|weird|weirder\n\n  \nteh|the\nno bar here\n");

    REQUIRE(read.size() == 2); // the duplicate, the blank lines and the bar-less line are gone
    CHECK(read[0] == SpellReplacement{"teh", "the"});
    CHECK(read[1] == SpellReplacement{"wierd", "weird|weirder"});
}

TEST_CASE("replacements survive a round trip", "[text][spell]") {
    InMemoryFileSystem files;
    WordListSpellProvider provider;
    provider.add("en", WordList{});
    const std::filesystem::path file = spellReplacementFile("/config", "en");

    auto first = openSpellChecker(provider, "en", files, file);
    REQUIRE(first.has_value());
    first->addReplacement("teh", "the");
    first->addReplacement("wierd", "weird");
    REQUIRE(saveSpellReplacements(*first, files, file).has_value());

    auto second = openSpellChecker(provider, "en", files, file);
    REQUIRE(second.has_value());
    CHECK(std::vector<SpellReplacement>{second->replacements().begin(),
                                        second->replacements().end()} ==
          std::vector<SpellReplacement>{{"teh", "the"}, {"wierd", "weird"}});
}

TEST_CASE("no replacements writes no file", "[text][spell]") {
    InMemoryFileSystem files;
    WordListSpellProvider provider;
    provider.add("en", WordList{});
    const std::filesystem::path file = spellReplacementFile("/config", "en");

    auto checker = openSpellChecker(provider, "en", files, file);
    REQUIRE(checker.has_value());
    REQUIRE(saveSpellReplacements(*checker, files, file).has_value());

    CHECK_FALSE(files.exists(file));
}

TEST_CASE("writing keeps the last of each duplicate and the last ten thousand", "[text][spell]") {
    // Keeping the last: the first `a|1` goes, the second stays, in its place.
    const std::vector<SpellReplacement> duplicated{{.word = "a", .replacement = "1"},
                                                   {.word = "b", .replacement = "2"},
                                                   {.word = "a", .replacement = "1"}};
    CHECK(renderSpellReplacements(duplicated) == "b|2\na|1\n");

    std::vector<SpellReplacement> many;
    many.reserve(kMaxSpellReplacements + 5);
    for (std::size_t index = 0; index < kMaxSpellReplacements + 5; ++index)
        many.push_back({.word = "w" + std::to_string(index), .replacement = "r"});
    const std::vector<SpellReplacement> kept =
        parseSpellReplacements(renderSpellReplacements(many));

    REQUIRE(kept.size() == kMaxSpellReplacements);
    CHECK(kept.front().word == "w5");
    CHECK(kept.back().word == "w" + std::to_string(kMaxSpellReplacements + 4));
}

TEST_CASE("an unreadable replacement file leaves no replacement", "[text][spell]") {
    const InMemoryFileSystem files; // no file at all
    WordListSpellProvider provider;
    provider.add("en", WordList{});

    auto checker = openSpellChecker(provider, "en", files, "/config/spell-check/en.repl");

    REQUIRE(checker.has_value());
    CHECK(checker->replacements().empty());
}

TEST_CASE("an empty configuration directory gives no file, which reads and writes nothing",
          "[text][spell]") {
    // Issue #530: a relative `spell-check/en.repl` would be written wherever
    // the working directory happens to be.
    CHECK(spellReplacementFile({}, "en").empty());

    InMemoryFileSystem files;
    WordListSpellProvider provider;
    provider.add("en", WordList{});
    auto checker = openSpellChecker(provider, "en", files, spellReplacementFile({}, "en"));
    REQUIRE(checker.has_value());
    CHECK(checker->replacements().empty());

    checker->addReplacement("teh", "the");
    CHECK(saveSpellReplacements(*checker, files, spellReplacementFile({}, "en")).has_value());
    CHECK(files.readFile("spell-check/en.repl").has_value() == false);
}
