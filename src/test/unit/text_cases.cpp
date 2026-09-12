#include "text_cases.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace subedit::test {

namespace {

/// Where the harness stops reading a case and complains instead.
/// The two spellings that are not a text: « unchanged », and « the subtitle
/// goes ». Carried as bytes no case file can hold, until the reader knows the
/// input and can settle them.
const std::string kUnchangedMark{"\x01"};
const std::string kRemovedMark{"\x02"};
const std::string kRemoved{"supprimé"};

[[noreturn]] void refuse(const std::string& file, int line, const std::string& why) {
    throw std::runtime_error(file + ":" + std::to_string(line) + " : " + why);
}

/// Reads a quoted text, or `=` meaning "the input, unchanged".
///
/// Returns the text and the offset just past what it consumed. The escapes are
/// the three a one-line format needs: the quote that would end it, the newline
/// it cannot hold, and the backslash that introduces both.
std::pair<std::string, std::size_t>
readText(const std::string& field, std::size_t at, const std::string& file, int line) {
    while (at < field.size() && field[at] == ' ') {
        ++at;
    }
    if (at < field.size() && field[at] == '=') {
        return {std::string{"\x01"}, at + 1};
    }
    if (field.compare(at, kRemoved.size(), kRemoved) == 0) {
        return {std::string{"\x02"}, at + kRemoved.size()};
    }
    if (at >= field.size() || field[at] != '"') {
        refuse(file,
               line,
               "un texte s'écrit entre guillemets, ou « = » pour « inchangé », ou "
               "« supprimé »");
    }

    std::string text;
    for (++at; at < field.size(); ++at) {
        if (field[at] == '"') {
            return {text, at + 1};
        }
        if (field[at] != '\\') {
            text += field[at];
            continue;
        }
        if (++at >= field.size()) {
            refuse(file, line, "une barre oblique inverse en fin de ligne n'échappe rien");
        }
        switch (field[at]) {
        case 'n':
            text += '\n';
            break;
        case 't':
            text += '\t';
            break;
        case '"':
        case '\\':
            text += field[at];
            break;
        default:
            refuse(file, line, std::string{"échappement inconnu : \\"} + field[at]);
        }
    }
    refuse(file, line, "guillemet fermant absent");
}

/// The separator between two fields, skipped along with the spaces around it.
std::size_t
skipSeparator(const std::string& field, std::size_t at, const std::string& file, int line) {
    while (at < field.size() && field[at] == ' ') {
        ++at;
    }
    if (at >= field.size() || field[at] != '|') {
        refuse(file, line, "des champs séparés par « | » sont attendus");
    }
    return at + 1;
}

/// Reads the name that opens a case, and says where the first `|` was.
///
/// Shared by the two readers: a case of three fields and one of five open the
/// same way, and the refusal for a case without a name is written once.
std::pair<std::string, std::size_t>
readName(const std::string& text, const std::string& file, int line) {
    const std::size_t first = text.find_first_not_of(" \t");
    const std::size_t bar = text.find('|');
    if (bar == std::string::npos) {
        refuse(file, line, "des champs séparés par « | » sont attendus");
    }

    std::string name = text.substr(first, bar - first);
    while (!name.empty() && name.back() == ' ') {
        name.pop_back();
    }
    if (name.empty()) {
        refuse(file, line, "un cas sans nom ne dit pas ce qu'il éprouve");
    }
    return {std::move(name), bar + 1};
}

/// Tells whether `text` is a line the readers walk past — blank, or a comment.
[[nodiscard]] bool isAside(const std::string& text) {
    const std::size_t first = text.find_first_not_of(" \t");
    return first == std::string::npos || text[first] == '#';
}

/// Refuses whatever follows the last field of a case.
void refuseTrailing(const std::string& text,
                    std::size_t at,
                    const std::string& file,
                    int line,
                    const std::string& which) {
    while (at < text.size() && text[at] == ' ') {
        ++at;
    }
    if (at != text.size()) {
        refuse(file, line, "du texte traîne après le " + which + " champ");
    }
}

} // namespace

std::vector<TextCase> textCasesOf(const std::string& relative) {
    const std::filesystem::path path = std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative;
    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error("corpus de cas introuvable : " + path.string());
    }

    std::vector<TextCase> cases;
    std::string text;
    int line = 0;
    while (std::getline(file, text)) {
        ++line;
        if (isAside(text)) {
            continue;
        }

        TextCase one;
        one.line = line;
        auto [name, afterName] = readName(text, path.string(), line);
        one.name = std::move(name);

        auto [input, afterInput] = readText(text, afterName, path.string(), line);
        if (input == kUnchangedMark || input == kRemovedMark) {
            refuse(path.string(), line, "« = » et « supprimé » ne se disent que de l'attendu");
        }
        const std::size_t second = skipSeparator(text, afterInput, path.string(), line);
        auto [expected, afterExpected] = readText(text, second, path.string(), line);

        refuseTrailing(text, afterExpected, path.string(), line, "troisième");

        one.input = input;
        if (expected == kRemovedMark) {
            one.expected = std::nullopt;
        } else {
            one.expected = expected == kUnchangedMark ? input : expected;
        }
        cases.push_back(std::move(one));
    }
    return cases;
}

std::vector<ReplacementCase> replacementCasesOf(const std::string& relative) {
    const std::filesystem::path path = std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative;
    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error("corpus de cas introuvable : " + path.string());
    }

    std::vector<ReplacementCase> cases;
    std::string text;
    int line = 0;
    while (std::getline(file, text)) {
        ++line;
        if (isAside(text)) {
            continue;
        }

        ReplacementCase one;
        one.line = line;
        auto [name, afterName] = readName(text, path.string(), line);
        one.name = std::move(name);

        std::size_t at = afterName;
        std::array<std::string, 3> given;
        for (std::string& field : given) {
            auto [read, after] = readText(text, at, path.string(), line);
            if (read == kUnchangedMark || read == kRemovedMark) {
                refuse(path.string(), line, "« = » et « supprimé » ne se disent que de l\'attendu");
            }
            field = std::move(read);
            at = skipSeparator(text, after, path.string(), line);
        }

        auto [expected, afterExpected] = readText(text, at, path.string(), line);
        if (expected == kRemovedMark) {
            refuse(path.string(),
                   line,
                   "« supprimé » ne se dit pas d\'un remplacement : il réécrit un sous-titre, "
                   "il n\'en retire aucun");
        }
        refuseTrailing(text, afterExpected, path.string(), line, "cinquième");

        one.input = std::move(given[0]);
        one.pattern = std::move(given[1]);
        one.replacement = std::move(given[2]);
        if (one.pattern.empty()) {
            refuse(path.string(), line, "un motif vide ne dit pas ce qu\'il cherche");
        }
        one.expected = expected == kUnchangedMark ? one.input : expected;
        cases.push_back(std::move(one));
    }
    return cases;
}

void checkTextCases(
    const std::vector<TextCase>& cases,
    const std::function<std::optional<std::string>(const std::string&)>& transform) {
    for (const TextCase& one : cases) {
        // INFO rather than a message on the assertion: Catch2 prints it only
        // for the cases that fail, so a corpus of forty stays readable.
        INFO("cas ligne " << one.line << " : " << one.name);
        CHECK(transform(one.input) == one.expected);
    }
}

void checkReplacementCases(
    const std::vector<ReplacementCase>& cases,
    const std::function<std::string(const std::string&, const std::string&, const std::string&)>&
        replace) {
    for (const ReplacementCase& one : cases) {
        INFO("cas ligne " << one.line << " : " << one.name);
        CHECK(replace(one.input, one.pattern, one.replacement) == one.expected);
    }
}

} // namespace subedit::test
