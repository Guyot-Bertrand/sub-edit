// The Enchant provider — issue #507, decision D6.
//
// **Nothing here depends on which dictionaries the machine has**, and Enchant's
// configuration is moved to a directory of the test's own before it is first
// used: the personal word list of whoever runs the test is never touched
// (`check-config-home.sh` watches `~/.config/enchant` for it).

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/enchant_spell_provider.hpp>
#include <subedit/core/text/spell_checker.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

using subedit::core::availableSpellLanguages;
using subedit::core::EnchantSpellProvider;
using subedit::core::InMemoryFileSystem;
using subedit::core::isValidSpellLanguage;
using subedit::core::openSpellChecker;

/// Points Enchant's configuration at a fresh directory for the length of a
/// test, then back.
class EnchantSandbox {

public:
    [[nodiscard]] const std::filesystem::path& directory() const { return m_directory; }

    EnchantSandbox()
        : m_directory(std::filesystem::temp_directory_path() /
                      ("subedit-enchant-test-" + std::to_string(::getpid()))) {
        std::filesystem::create_directories(m_directory);
        const char* previous = std::getenv("ENCHANT_CONFIG_DIR");
        if (previous != nullptr)
            m_previous = previous;
        ::setenv("ENCHANT_CONFIG_DIR", m_directory.c_str(), 1);
    }

    ~EnchantSandbox() {
        if (m_previous.has_value())
            ::setenv("ENCHANT_CONFIG_DIR", m_previous->c_str(), 1);
        else
            ::unsetenv("ENCHANT_CONFIG_DIR");
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    EnchantSandbox(const EnchantSandbox&) = delete;
    EnchantSandbox& operator=(const EnchantSandbox&) = delete;
    EnchantSandbox(EnchantSandbox&&) = delete;
    EnchantSandbox& operator=(EnchantSandbox&&) = delete;

private:
    std::filesystem::path m_directory;
    std::optional<std::string> m_previous;
};

} // namespace

TEST_CASE("Enchant has no dictionary for a language nobody has", "[text][spell][enchant]") {
    const EnchantSandbox sandbox;
    const EnchantSpellProvider provider;
    const InMemoryFileSystem files;

    const auto opened = openSpellChecker(provider, "zz_ZZ", files, "/none.repl");

    REQUIRE_FALSE(opened.has_value());
    CHECK(opened.error().language == "zz_ZZ");
}

TEST_CASE("Enchant lists only valid locale codes once filtered, whatever the machine has",
          "[text][spell][enchant]") {
    const EnchantSandbox sandbox;
    const EnchantSpellProvider provider;

    for (const std::string& code : availableSpellLanguages(provider))
        CHECK(isValidSpellLanguage(code));
}

TEST_CASE("a word list is a dictionary: check, suggest, and add to the personal list",
          "[text][spell][enchant]") {
    const EnchantSandbox sandbox;
    const EnchantSpellProvider provider;
    const std::filesystem::path file = sandbox.directory() / "words.dic";
    { std::ofstream{file} << "alpha\nbeta\n"; }

    const auto dictionary = provider.openWordList(file);

    REQUIRE(dictionary != nullptr);
    CHECK(dictionary->isCorrect("alpha"));
    CHECK_FALSE(dictionary->isCorrect("gamma"));
    const std::vector<std::string> offered = dictionary->suggestions("alpah");
    CHECK(std::ranges::find(offered, "alpha") != offered.end());
    CHECK(dictionary->suggestions("qwxzvkjhgfdsqwxzvkjh").empty()); // nothing near it

    dictionary->addToPersonal("gamma");

    CHECK(dictionary->isCorrect("gamma"));
    std::ifstream written{file};
    const std::string content{std::istreambuf_iterator<char>{written}, {}};
    CHECK(content.find("gamma") != std::string::npos);
}

TEST_CASE("the same word list serves a checker, heuristics and all", "[text][spell][enchant]") {
    const EnchantSandbox sandbox;
    const EnchantSpellProvider provider;
    const std::filesystem::path file = sandbox.directory() / "words.dic";
    { std::ofstream{file} << "going\n"; }
    const subedit::core::SpellChecker checker{provider.openWordList(file), "en"};

    CHECK(checker.check("goin", "", "'"));
    CHECK_FALSE(checker.check("goin"));
}

TEST_CASE("a word list that cannot be opened gives no dictionary", "[text][spell][enchant]") {
    const EnchantSandbox sandbox;
    const EnchantSpellProvider provider;

    CHECK(provider.openWordList(sandbox.directory() / "no-such-directory" / "words.dic") ==
          nullptr);
}

TEST_CASE("every language Enchant offers opens, whichever dictionaries the machine has",
          "[text][spell][enchant]") {
    // **Vacuous where the machine has no dictionary, and that is the point**: a
    // test that needed one would depend on the machine (D6). Where there are
    // some, each is opened and asked a question, which is the road a real
    // language takes.
    const EnchantSandbox sandbox;
    const EnchantSpellProvider provider;

    for (const std::string& code : availableSpellLanguages(provider)) {
        const auto dictionary = provider.open(code);
        INFO(code);
        REQUIRE(dictionary != nullptr);
        static_cast<void>(dictionary->isCorrect("a"));
    }
}
