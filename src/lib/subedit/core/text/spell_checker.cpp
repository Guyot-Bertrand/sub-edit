#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/uchar.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

[[nodiscard]] bool endsWith(std::string_view text, std::string_view suffix) {
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

[[nodiscard]] bool isDecimalDigit(char32_t c) {
    return u_isdigit(static_cast<UChar32>(c));
}

/// `\d*` — every character of `text` a digit; nothing is.
[[nodiscard]] bool isAllDigits(std::string_view text) {
    for (std::size_t at = 0; at < text.size(); at = nextCodePoint(text, at)) {
        if (!isDecimalDigit(codePointAt(text, at)))
            return false;
    }
    return true;
}

/// Gaupol's six ordinal patterns and its seventh, `^\d*?[0,4-9]th$`.
///
/// **The comma in `[0,4-9]` is Gaupol's and is kept**: a class of `0`, `,` and
/// `4` to `9`, where `0` and `4-9` were surely meant. The tokenizer never
/// hands over a comma, so it only ever matters to a direct call.
[[nodiscard]] bool isOrdinalNumeral(std::string_view word) {
    for (const std::string_view suffix : std::array<std::string_view, 3>{"1st", "2nd", "3rd"}) {
        // `^\d*?(?<!1)1st$`: the digits before it do not end with a 1.
        if (endsWith(word, suffix)) {
            const std::string_view digits = word.substr(0, word.size() - suffix.size());
            if (isAllDigits(digits) && !endsWith(digits, "1"))
                return true;
        }
    }
    for (const std::string_view suffix : std::array<std::string_view, 3>{"1th", "2th", "3th"}) {
        // `^\d*?(?<=1)1th$`: the digits before it end with a 1.
        if (endsWith(word, suffix)) {
            const std::string_view digits = word.substr(0, word.size() - suffix.size());
            if (isAllDigits(digits) && endsWith(digits, "1"))
                return true;
        }
    }
    if (endsWith(word, "th") && word.size() >= 3) {
        const std::string_view before = word.substr(0, word.size() - 2);
        const char last = before.back();
        const bool inClass = last == '0' || last == ',' || (last >= '4' && last <= '9');
        if (inClass && isAllDigits(before.substr(0, before.size() - 1)))
            return true;
    }
    return false;
}

/// `re.match(r"^\d+\D+$", word)`, and `re.sub(r"^(\d+)(\D+)$", r"\1 \2")`: the
/// word with a space where its digits stop, or nothing if it is not digits
/// and then non-digits.
[[nodiscard]] std::string numberThenUnitSpaced(std::string_view word) {
    std::size_t at = 0;
    while (at < word.size() && isDecimalDigit(codePointAt(word, at)))
        at = nextCodePoint(word, at);
    if (at == 0 || at == word.size())
        return {};
    for (std::size_t rest = at; rest < word.size(); rest = nextCodePoint(word, rest)) {
        if (isDecimalDigit(codePointAt(word, rest)))
            return {};
    }
    return std::string{word.substr(0, at)} + ' ' + std::string{word.substr(at)};
}

/// Python's `str.split()`: on runs of white space, none left over.
[[nodiscard]] std::vector<std::string> splitOnWhitespace(std::string_view text) {
    std::vector<std::string> pieces;
    std::size_t start = std::string_view::npos;
    for (std::size_t at = 0; at < text.size(); at = nextCodePoint(text, at)) {
        const bool space = u_isUWhiteSpace(static_cast<UChar32>(codePointAt(text, at)));
        if (space && start != std::string_view::npos) {
            pieces.emplace_back(text.substr(start, at - start));
            start = std::string_view::npos;
        } else if (!space && start == std::string_view::npos) {
            start = at;
        }
    }
    if (start != std::string_view::npos)
        pieces.emplace_back(text.substr(start));
    return pieces;
}

void addUnique(std::vector<std::string>& into, std::string candidate) {
    if (std::ranges::find(into, candidate) == into.end())
        into.push_back(std::move(candidate));
}

} // namespace

bool isValidSpellLanguage(std::string_view code) {
    // `^[a-z]{2}(_[A-Z]{2})?(@[A-Z][a-z]{3})?$`
    const auto lower = [](char c) { return c >= 'a' && c <= 'z'; };
    const auto upper = [](char c) { return c >= 'A' && c <= 'Z'; };
    if (code.size() < 2 || !lower(code[0]) || !lower(code[1]))
        return false;
    std::size_t at = 2;
    if (at < code.size() && code[at] == '_') {
        if (at + 3 > code.size() || !upper(code[at + 1]) || !upper(code[at + 2]))
            return false;
        at += 3;
    }
    if (at < code.size() && code[at] == '@') {
        constexpr std::size_t kScriptLength = 5; // `@` and four letters
        if (at + kScriptLength > code.size() || !upper(code[at + 1]) || !lower(code[at + 2]) ||
            !lower(code[at + 3]) || !lower(code[at + 4]))
            return false;
        at += kScriptLength;
    }
    return at == code.size();
}

std::vector<std::string> availableSpellLanguages(const SpellProvider& provider) {
    std::vector<std::string> codes = provider.languages();
    std::erase_if(codes, [](const std::string& code) { return !isValidSpellLanguage(code); });
    std::ranges::sort(codes);
    return codes;
}

SpellChecker::SpellChecker(std::unique_ptr<SpellDictionary> dictionary,
                           std::string language,
                           std::vector<SpellReplacement> replacements)
    : m_dictionary(std::move(dictionary)),
      m_language(std::move(language)),
      m_replacements(std::move(replacements)) {}

bool SpellChecker::isCorrectWord(std::string_view word) const {
    if (m_sessionWords.contains(word))
        return true;
    return m_dictionary->isCorrect(word);
}

bool SpellChecker::check(std::string_view word,
                         std::string_view /*leadingContext*/,
                         std::string_view trailingContext) const {
    if (m_language.starts_with("en")) {
        if (endsWith(word, "in") && trailingContext.starts_with('\'')) {
            // Also check the word with its formal "ing" ending.
            const std::array<std::string, 2> both{std::string{word}, std::string{word} + "g"};
            return checkAny(both);
        }
        for (const std::string_view suffix :
             std::array<std::string_view, 5>{"'d", "'ll", "'re", "'s", "'ve"}) {
            if (endsWith(word, suffix)) {
                // Also check just the main word.
                const std::array<std::string, 2> both{
                    std::string{word}, std::string{word.substr(0, word.size() - suffix.size())}};
                return checkAny(both);
            }
        }
        if (isOrdinalNumeral(word))
            return true;
    }
    return isCorrectWord(word);
}

bool SpellChecker::checkAll(std::span<const std::string> words) const {
    return std::ranges::all_of(words,
                               [this](const std::string& one) { return isCorrectWord(one); });
}

bool SpellChecker::checkAny(std::span<const std::string> words) const {
    return std::ranges::any_of(words,
                               [this](const std::string& one) { return isCorrectWord(one); });
}

std::vector<std::string> SpellChecker::suggest(std::string_view word) const {
    std::vector<std::string> custom;
    for (const SpellReplacement& one : m_replacements) {
        if (one.word == word)
            custom.push_back(one.replacement);
    }
    if (word.contains('I')) {
        // The most common OCR error: a lower-case l read as a capital I.
        std::string replacement{word};
        std::ranges::replace(replacement, 'I', 'l');
        if (check(replacement))
            custom.push_back(std::move(replacement));
    }
    if (const std::string spaced = numberThenUnitSpaced(word); !spaced.empty()) {
        // A common OCR omission: the space between a number and its unit.
        if (checkAll(splitOnWhitespace(spaced)))
            custom.push_back(spaced);
    }

    std::vector<std::string> suggestions;
    for (std::string& one : custom)
        addUnique(suggestions, std::move(one));
    for (std::string& one : m_dictionary->suggestions(word))
        addUnique(suggestions, std::move(one));
    return suggestions;
}

void SpellChecker::addToPersonal(std::string_view word) {
    m_dictionary->addToPersonal(word);
}

void SpellChecker::addToSession(std::string_view word) {
    m_sessionWords.emplace(word);
}

void SpellChecker::addReplacement(std::string_view word, std::string_view replacement) {
    m_replacements.push_back({.word = std::string{word}, .replacement = std::string{replacement}});
}

std::expected<SpellChecker, NoDictionary>
openSpellChecker(const SpellProvider& provider,
                 std::string_view language,
                 const FileSystem& files,
                 const std::filesystem::path& replacementFile) {
    std::unique_ptr<SpellDictionary> dictionary = provider.open(language);
    if (dictionary == nullptr)
        return std::unexpected{NoDictionary{.language = std::string{language}}};

    std::vector<SpellReplacement> replacements;
    if (const auto text = files.readFile(replacementFile); text.has_value())
        replacements = parseSpellReplacements(*text);
    return SpellChecker{std::move(dictionary), std::string{language}, std::move(replacements)};
}

std::expected<void, FileError> saveSpellReplacements(const SpellChecker& checker,
                                                     FileSystem& files,
                                                     const std::filesystem::path& replacementFile) {
    const std::string text = renderSpellReplacements(checker.replacements());
    if (text.empty())
        return {};
    if (auto made = files.createDirectories(replacementFile.parent_path()); !made.has_value())
        return std::unexpected{made.error()};
    return writeAtomically(files, replacementFile, text);
}

} // namespace subedit::core
