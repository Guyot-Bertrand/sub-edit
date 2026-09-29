#include <subedit/core/text/join_split_words.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_tokenizer.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/uchar.h>

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// Gaupol's `SpellCheckNavigator`: a position in a text, moved by the words
/// a checker finds misspelt, that the caller then joins, replaces or passes.
/// Positions are bytes of the UTF-8 text.
class Navigator {

public:
    Navigator(const SpellChecker& checker, std::string text)
        : m_checker(&checker), m_text(std::move(text)) {}

    [[nodiscard]] const std::string& text() const { return m_text; }

    [[nodiscard]] std::size_t position() const { return m_pos; }

    [[nodiscard]] const std::string& word() const { return m_word; }

    [[nodiscard]] std::size_t end() const { return m_pos + m_word.size(); }

    /// The next misspelt word from here, or false. Every word looked at moves
    /// the position to it, correct or not — as Gaupol's iteration does.
    [[nodiscard]] bool next() {
        const std::size_t from = m_pos;
        for (SpellWord& one : tokenizeForSpelling(std::string_view{m_text}.substr(from))) {
            m_pos = from + one.offset;
            m_word = std::move(one.word);
            if (!m_checker->check(m_word, leadingContext(), trailingContext()))
                return true;
        }
        return false;
    }

    /// The character before the word, or nothing at the start of the text.
    [[nodiscard]] std::string_view leadingContext() const {
        if (m_pos == 0)
            return {};
        const std::size_t before = previousCodePoint(m_text, m_pos);
        return std::string_view{m_text}.substr(before, m_pos - before);
    }

    /// The character after the word, or nothing at the end of the text.
    [[nodiscard]] std::string_view trailingContext() const {
        const std::size_t at = end();
        if (at >= m_text.size())
            return {};
        return std::string_view{m_text}.substr(at, nextCodePoint(m_text, at) - at);
    }

    void ignore() { m_pos = end(); }

    void replace(std::string_view replacement) {
        m_text.replace(m_pos, m_word.size(), replacement);
        m_pos += replacement.size();
    }

    void joinWithNext() {
        while (isSpaceAt(end()))
            deleteAt(end());
    }

    void joinWithPrevious() {
        while (m_pos > 0 && isSpaceAt(previousCodePoint(m_text, m_pos))) {
            m_pos = previousCodePoint(m_text, m_pos);
            deleteAt(m_pos);
        }
        // Rewind to the start of the new compound word.
        while (m_pos > 0 && u_isalnum(static_cast<UChar32>(
                                codePointAt(m_text, previousCodePoint(m_text, m_pos)))))
            m_pos = previousCodePoint(m_text, m_pos);
    }

private:
    [[nodiscard]] bool isSpaceAt(std::size_t at) const {
        return at < m_text.size() && u_isUWhiteSpace(static_cast<UChar32>(codePointAt(m_text, at)));
    }

    void deleteAt(std::size_t at) { m_text.erase(at, nextCodePoint(m_text, at) - at); }

    const SpellChecker* m_checker;
    std::string m_text;
    std::size_t m_pos = 0;
    std::string m_word;
};

/// `re.sub(r" +", " ", text)`.
[[nodiscard]] std::string withSpacesCollapsed(std::string_view text) {
    std::string collapsed;
    collapsed.reserve(text.size());
    for (const char c : text) {
        if (c == ' ' && !collapsed.empty() && collapsed.back() == ' ')
            continue;
        collapsed.push_back(c);
    }
    return collapsed;
}

/// `re.split(r"\W+", text)[-1]`: the word characters ending `text`.
[[nodiscard]] std::string_view wordEnding(std::string_view text) {
    std::size_t start = text.size();
    while (start > 0) {
        const std::size_t before = previousCodePoint(text, start);
        if (!isSpellWordCharacter(codePointAt(text, before)))
            break;
        start = before;
    }
    return text.substr(start);
}

/// `re.split(r"\W+", text)[0]`: the word characters starting `text`.
[[nodiscard]] std::string_view wordStart(std::string_view text) {
    std::size_t end = 0;
    while (end < text.size() && isSpellWordCharacter(codePointAt(text, end)))
        end = nextCodePoint(text, end);
    return text.substr(0, end);
}

/// Python's `str.istitle()`, by ICU's case properties: capitals only after
/// what is not a letter with case, lower case only after one, and at least one
/// cased letter — so `Hello` is, `HELLO` and `McDonald` and `Don't` are not.
[[nodiscard]] bool isTitleCase(std::string_view word) {
    bool cased = false;
    bool previousCased = false;
    for (std::size_t at = 0; at < word.size(); at = nextCodePoint(word, at)) {
        const auto c = static_cast<UChar32>(codePointAt(word, at));
        if (u_isupper(c) || u_istitle(c)) {
            if (previousCased)
                return false;
            previousCased = true;
            cased = true;
        } else if (u_islower(c)) {
            if (!previousCased)
                return false;
            previousCased = true;
            cased = true;
        } else {
            previousCased = false;
        }
    }
    return cased;
}

/// Where one misspelt word of `text` is joined, if one direction is right.
void joinAt(const SpellChecker& checker, Navigator& navigator) {
    const std::string snapshot = navigator.text();
    const std::size_t start = navigator.position();
    const std::size_t end = navigator.end();
    const std::string_view text{snapshot};

    bool okWithPrevious = false;
    if (navigator.leadingContext() == " ") {
        const std::string candidate =
            std::string{wordEnding(text.substr(0, start - 1))} + navigator.word();
        okWithPrevious = checker.check(candidate);
    }
    bool okWithNext = false;
    if (navigator.trailingContext() == " ") {
        const std::string candidate =
            navigator.word() + std::string{wordStart(text.substr(end + 1))};
        okWithNext = checker.check(candidate);
    }

    // Join backwards or forwards if only one direction, not both, gives a
    // correctly spelt word.
    if (okWithPrevious == okWithNext)
        navigator.ignore();
    else if (okWithPrevious)
        navigator.joinWithPrevious();
    else
        navigator.joinWithNext();
}

/// Where one misspelt word of `text` is split, if one suggestion says how.
void splitAt(const SpellChecker& checker, Navigator& navigator) {
    // Capitalised words are usually names, which dictionaries lack.
    if (isTitleCase(navigator.word())) {
        navigator.ignore();
        return;
    }
    std::vector<std::string> same;
    for (std::string& suggestion : checker.suggest(navigator.word())) {
        std::string stripped;
        for (const char c : suggestion) {
            if (c != ' ')
                stripped.push_back(c);
        }
        if (stripped == navigator.word())
            same.push_back(std::move(suggestion));
    }
    // Only if there is exactly one suggestion with all the letters of the
    // unsplit word.
    if (same.size() == 1)
        navigator.replace(same.front());
    else
        navigator.ignore();
}

template<typename Step>
[[nodiscard]] std::vector<std::string>
correctedBy(const SpellChecker& checker, std::span<const std::string> texts, Step step) {
    std::vector<std::string> done;
    done.reserve(texts.size());
    for (const std::string& original : texts) {
        const std::string collapsed = withSpacesCollapsed(original);
        Navigator navigator{checker, collapsed};
        while (navigator.next())
            step(checker, navigator);
        done.push_back(navigator.text() == collapsed ? original : navigator.text());
    }
    return done;
}

} // namespace

std::vector<std::string> joinWords(const SpellChecker& checker,
                                   std::span<const std::string> texts) {
    return correctedBy(checker, texts, joinAt);
}

std::vector<std::string> splitWords(const SpellChecker& checker,
                                    std::span<const std::string> texts) {
    return correctedBy(checker, texts, splitAt);
}

} // namespace subedit::core
