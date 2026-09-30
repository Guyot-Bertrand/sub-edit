#include <subedit/core/text/spell_check_navigator.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/uchar.h>

#include <utility>

namespace subedit::core {

namespace {

[[nodiscard]] bool isSpace(std::string_view character) {
    return !character.empty() && u_isspace(static_cast<UChar32>(codePointAt(character, 0)));
}

} // namespace

SpellCheckNavigator::SpellCheckNavigator(SpellChecker checker) : m_checker(std::move(checker)) {}

void SpellCheckNavigator::reset(std::string text) {
    m_text = std::move(text);
    m_pos = 0;
    m_word.clear();
}

std::string_view SpellCheckNavigator::leadingCharacter() const {
    if (m_pos == 0)
        return {};
    const std::size_t start = previousCodePoint(m_text, m_pos);
    return std::string_view{m_text}.substr(start, m_pos - start);
}

std::string_view SpellCheckNavigator::trailingCharacter() const {
    const std::size_t end = endPos();
    if (end >= m_text.size())
        return {};
    return std::string_view{m_text}.substr(end, nextCodePoint(m_text, end) - end);
}

std::optional<SpellWord> SpellCheckNavigator::next() {
    // Gaupol recurses after a silent replacement; a loop restarts the same way.
    for (;;) {
        const std::size_t initial = m_pos;
        bool replaced = false;
        for (const SpellWord& found :
             tokenizeForSpelling(std::string_view{m_text}.substr(initial))) {
            m_pos = initial + found.offset;
            m_word = found.word;
            if (m_checker.check(m_word, leadingCharacter(), trailingCharacter()))
                continue;
            if (const auto it = m_replacements.find(m_word); it != m_replacements.end()) {
                replace(std::string{it->second});
                replaced = true;
                break;
            }
            return SpellWord{.offset = m_pos, .word = m_word};
        }
        if (!replaced)
            return std::nullopt;
    }
}

void SpellCheckNavigator::ignore() {
    m_pos = endPos();
}

void SpellCheckNavigator::ignoreAll() {
    m_checker.addToSession(m_word);
    m_pos = endPos();
}

void SpellCheckNavigator::add() {
    m_checker.addToPersonal(m_word);
}

void SpellCheckNavigator::replace(std::string_view replacement) {
    m_checker.addReplacement(m_word, replacement);
    m_text.replace(m_pos, m_word.size(), replacement);
    m_pos += replacement.size();
    // As in Gaupol, the current word stays the old one: `endPos` is stale
    // until the next `next`, which nothing reads before.
}

void SpellCheckNavigator::replaceAll(std::string_view replacement) {
    m_replacements[m_word] = std::string{replacement};
    replace(replacement);
}

void SpellCheckNavigator::joinWithNext() {
    while (isSpace(trailingCharacter())) {
        const std::size_t at = endPos();
        m_text.erase(at, nextCodePoint(m_text, at) - at);
    }
}

void SpellCheckNavigator::joinWithPrevious() {
    while (isSpace(leadingCharacter())) {
        const std::size_t start = previousCodePoint(m_text, m_pos);
        m_text.erase(start, m_pos - start);
        m_pos = start;
    }
    // Rewind to the start of the new compound word.
    while (m_pos > 0 && u_isalnum(static_cast<UChar32>(codePointAt(leadingCharacter(), 0))))
        m_pos = previousCodePoint(m_text, m_pos);
}

bool SpellCheckNavigator::spaceBefore() const {
    return isSpace(leadingCharacter());
}

bool SpellCheckNavigator::spaceAfter() const {
    return isSpace(trailingCharacter());
}

std::vector<std::string> SpellCheckNavigator::suggest() const {
    return m_checker.suggest(m_word);
}

} // namespace subedit::core
