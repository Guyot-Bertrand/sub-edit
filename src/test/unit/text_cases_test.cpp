#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "text_cases.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::test::checkReplacementCases;
using subedit::test::checkTextCases;
using subedit::test::ReplacementCase;
using subedit::test::replacementCasesOf;
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

/// The transformation the five-field harness corpus is written against: it
/// replaces every occurrence in the raw string, knowing nothing of tags.
///
/// It lives here for the reason the one above does — the real one is not
/// written, and a harness is proved by running it on something whose answers
/// are known. Its tag-blindness is the point: it must not be mistaken for the
/// rule that `recherche.cas` decides.
std::string replacedEverywhere(const std::string& text,
                               const std::string& pattern,
                               const std::string& replacement) {
    std::string out;
    std::size_t at = 0;
    while (true) {
        const std::size_t found = text.find(pattern, at);
        if (found == std::string::npos) {
            out += text.substr(at);
            return out;
        }
        out += text.substr(at, found - at);
        out += replacement;
        at = found + pattern.size();
    }
}

const std::vector<TextCase>& harness() {
    static const std::vector<TextCase> cases = textCasesOf("textes/harnais.cas");
    return cases;
}

const std::vector<ReplacementCase>& replacementHarness() {
    static const std::vector<ReplacementCase> cases =
        replacementCasesOf("textes/harnais-remplacement.cas");
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
    // mentions, written before the transformation existed — that is the point
    // of the format. The transformation now runs them, in
    // core/text/hearing_impaired_test.cpp; what is checked here is something
    // else, and still worth its own case: that every case is **well formed**.
    // A corpus that loads badly would run fewer cases than it holds and still
    // report green.
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

    refused("sans-separateur.cas", "séparés par");
    refused("texte-nu.cas", "entre guillemets");
    refused("guillemet-ouvert.cas", "guillemet fermant");
    refused("echappement-inconnu.cas", "échappement inconnu");
    refused("oblique-finale.cas", "n'échappe rien");
    refused("residu.cas", "traîne après");
    refused("sans-nom.cas", "sans nom");
    refused("egal-en-entree.cas", "de l'attendu");
}

TEST_CASE("the five-field harness runs against the transformation it describes",
          "[test][textcases]") {
    checkReplacementCases(replacementHarness(), replacedEverywhere);
}

TEST_CASE("a replacement case carries its four texts", "[test][textcases]") {
    REQUIRE(replacementHarness().size() == 11);

    const ReplacementCase& first = replacementHarness().front();
    CHECK(first.name == "un motif absent laisse tout en place");
    CHECK(first.input == "Bonjour Marie");
    CHECK(first.pattern == "Sophie");
    CHECK(first.replacement == "Claire");
    // `=` still means « the text is left alone », which is what a pattern that
    // matches nothing gives.
    CHECK(first.expected == first.input);
}

TEST_CASE("an empty replacement is a replacement", "[test][textcases]") {
    // Not the same thing as no case at all: taking a word out is what a user
    // does with an empty « replace with » field, and the corpus has to be able
    // to say it.
    for (const ReplacementCase& one : replacementHarness()) {
        if (one.name == "un remplacement vide retire") {
            CHECK(one.replacement.empty());
            CHECK(one.expected == "Bonjour");
            return;
        }
    }
    FAIL("cas absent du corpus");
}

TEST_CASE("a replacement corpus the harness cannot read stops the run", "[test][textcases]") {
    const auto refused = [](const std::string& file, const std::string& said) {
        INFO("corpus : " << file);
        CHECK_THROWS_WITH(replacementCasesOf("textes/refus/" + file),
                          ContainsSubstring(said) && ContainsSubstring(":2 :"));
    };

    refused("remplacement-quatre-champs.cas", "séparés par");
    refused("remplacement-motif-vide.cas", "ne dit pas ce qu'il cherche");
    refused("remplacement-supprime.cas", "il n'en retire aucun");
}

TEST_CASE("the corpus of the phase reads, long before there is a parser to run it",
          "[test][textcases]") {
    // recherche.cas and its brace sibling hold what has been decided about
    // searching in marked-up text, written before the tag-aware parser of
    // ADR 0009 exists — that is the point of the format, and it is what
    // mentions.cas did for phase 4. What is checked here is that every case is
    // **well formed**: a corpus that loads badly would run fewer cases than it
    // holds and still report green.
    for (const std::string file : {"textes/recherche.cas", "textes/recherche-accolades.cas"}) {
        INFO("corpus : " << file);
        const std::vector<ReplacementCase> decided = replacementCasesOf(file);

        CHECK(decided.size() > 5);
        for (const ReplacementCase& one : decided) {
            INFO("cas ligne " << one.line);
            CHECK_FALSE(one.name.empty());
            CHECK_FALSE(one.pattern.empty());
        }
    }
}

TEST_CASE("the braces of the corpus are braces, and its backslashes backslashes",
          "[test][textcases]") {
    // The one thing a corpus written in a vocabulary of backslashes can get
    // wrong without anybody seeing it: `\\i1` read as two characters instead of
    // one would make every case of that file test a text no format writes.
    for (const ReplacementCase& one : replacementCasesOf("textes/recherche-accolades.cas")) {
        if (one.name == "une fermante coupe le mot") {
            CHECK(one.input == R"({\i1}Bon{\i0}jour)");
            CHECK(one.expected == R"({\i1}Salut{\i0})");
            return;
        }
    }
    FAIL("cas absent du corpus");
}
