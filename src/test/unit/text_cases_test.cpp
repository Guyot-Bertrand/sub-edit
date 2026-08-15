#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "text_cases.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::test::checkTextCases;
using subedit::test::TextCase;
using subedit::test::textCasesOf;

namespace {

/// The transformation the harness corpus is written against: it removes what
/// sits between brackets, non-greedily.
///
/// It lives here and not in the library because none exists yet — phase 4 will
/// write the real one. What is under test is the harness, and a harness is
/// proved by running it on something whose answers are known.
std::optional<std::string> withoutBrackets(const std::string& text) {
    std::string kept;
    std::size_t depth = 0;
    for (const char letter : text) {
        if (letter == '[') {
            ++depth;
        } else if (letter == ']' && depth > 0) {
            --depth;
        } else if (depth == 0) {
            kept += letter;
        }
    }
    // An unclosed bracket bites nothing: what was skipped is put back.
    if (depth != 0) {
        return text;
    }
    // Nothing left: the subtitle goes. The corpus has a spelling for that, and
    // exercising it here is what proves the spelling is read.
    return kept.empty() ? std::optional<std::string>{} : kept;
}

const std::vector<TextCase>& harness() {
    static const std::vector<TextCase> cases = textCasesOf("textes/harnais.cas");
    return cases;
}

} // namespace

TEST_CASE("the corpus of the harness runs against the transformation it describes",
          "[test][textcases]") {
    checkTextCases(harness(), withoutBrackets);
}

TEST_CASE("a case carries its name, and the line it was written on", "[test][textcases]") {
    // The name is what a failure names. Without it, a failing case sends the
    // reader to a line of C++ that says nothing about which text broke.
    REQUIRE_FALSE(harness().empty());
    CHECK(harness().front().name == "un texte sans crochet est rendu tel quel");
    CHECK(harness().front().line > 0);
}

TEST_CASE("comments and blank lines are not cases", "[test][textcases]") {
    // The file opens with a dozen lines of comment and holds blank ones
    // throughout; not one of them is a case.
    CHECK(harness().size() == 10);
}

TEST_CASE("an expected of = means the text is left alone", "[test][textcases]") {
    const TextCase& unchanged = harness().front();

    CHECK(unchanged.expected == unchanged.input);
}

TEST_CASE("an expected of supprimé means the subtitle does not survive", "[test][textcases]") {
    // Not the same thing as empty text: a line a mention emptied keeps its
    // place — « Bonjour\n[soupir] » becomes « Bonjour\n » — while a text
    // emptied whole takes its subtitle with it.
    for (const TextCase& one : harness()) {
        if (one.name == "une mention seule ne laisse rien") {
            CHECK_FALSE(one.expected.has_value());
            return;
        }
    }
    FAIL("cas absent du corpus");
}

TEST_CASE("the escapes are read", "[test][textcases]") {
    const auto find = [](const std::string& name) {
        for (const TextCase& one : harness()) {
            if (one.name == name) {
                return one;
            }
        }
        FAIL("cas absent du corpus : " << name);
        return TextCase{};
    };

    CHECK(find("la deuxième ligne seule est touchée").input == "Bonjour\n[soupir]");
    CHECK(find("un guillemet dans le texte se protège").input == "il dit \"non\"");
    CHECK(find("une barre oblique inverse aussi").input == "chemin\\ici");
    CHECK(find("une tabulation s'écrit \\t").input == "avant\taprès");
    // Outside quotes the pipe separates; inside it is a character like any
    // other, so no escape is owed for it.
    CHECK(find("une barre verticale n'a rien de spécial").input == "avant | après");
}

TEST_CASE("spaces outside the quotes are alignment, not data", "[test][textcases]") {
    // The corpus pads its columns. A case whose expected keeps two spaces in
    // the middle proves that only what is inside the quotes counts.
    for (const TextCase& one : harness()) {
        if (one.name == "un espace dans les guillemets est donné") {
            CHECK(one.expected == "Bonjour  fin");
            return;
        }
    }
    FAIL("cas absent du corpus");
}

TEST_CASE("a file that is not there says so", "[test][textcases]") {
    CHECK_THROWS_WITH(textCasesOf("textes/rien-du-tout.cas"),
                      ContainsSubstring("rien-du-tout.cas"));
}

TEST_CASE("the corpus of the phase reads, long before there is anything to run it against",
          "[test][textcases]") {
    // mentions.cas holds what has been decided about removing hearing-impaired
    // mentions, written before the transformation exists — that is the point of
    // the format. Nothing runs those cases yet; what is checked here is that
    // they are well formed, so that a case added the day a question is settled
    // fails now rather than at implementation time.
    const std::vector<TextCase> decided = textCasesOf("textes/mentions.cas");

    CHECK_FALSE(decided.empty());
    for (const TextCase& one : decided) {
        INFO("cas ligne " << one.line);
        CHECK_FALSE(one.name.empty());
    }
}

TEST_CASE("a case the harness cannot read stops the run, naming the line", "[test][textcases]") {
    // A corpus that loads badly is worse than one that fails: it would run
    // fewer cases than it holds and still report green. Each refusal has its
    // own one-line file, under textes/refus/.
    const auto refused = [](const std::string& file, const std::string& said) {
        INFO("corpus : " << file);
        CHECK_THROWS_WITH(textCasesOf("textes/refus/" + file),
                          ContainsSubstring(said) && ContainsSubstring(":2 :"));
    };

    refused("sans-separateur.cas", "trois champs");
    refused("texte-nu.cas", "entre guillemets");
    refused("guillemet-ouvert.cas", "guillemet fermant");
    refused("echappement-inconnu.cas", "échappement inconnu");
    refused("oblique-finale.cas", "n'échappe rien");
    refused("residu.cas", "traîne après");
    refused("sans-nom.cas", "sans nom");
    refused("egal-en-entree.cas", "de l'attendu");
}
