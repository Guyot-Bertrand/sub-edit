#pragma once

// Walking the misspelt words of one text, as Gaupol's `SpellCheckNavigator`
// does (`aeidon/spell.py`) — decision D6 of the spec of phase 12, issue #509.
//
// **A port, gesture for gesture**: the same state (a text, a position, the
// current word, the replace-all memory), the same order of effects. What
// changes is the unit — positions are **bytes of the UTF-8 text**, as the
// tokenizer's are — and every gesture that moves by "a character" moves by a
// whole code point, never by a byte.

#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_tokenizer.hpp>

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

class SpellCheckNavigator {

public:
    explicit SpellCheckNavigator(SpellChecker checker);

    [[nodiscard]] SpellChecker& checker() { return m_checker; }

    [[nodiscard]] const SpellChecker& checker() const { return m_checker; }

    /// Starts over on `text`: position zero, no current word. **The replace-all
    /// memory survives**, as in Gaupol: it spans the texts of a walk.
    void reset(std::string text);

    /// The next misspelt word from the current position, which becomes the
    /// current word. A word `replaceAll` was told about is replaced silently
    /// on the way. Absent when the text holds no more.
    [[nodiscard]] std::optional<SpellWord> next();

    [[nodiscard]] const std::string& text() const { return m_text; }

    /// Where the current word starts, in bytes.
    [[nodiscard]] std::size_t pos() const { return m_pos; }

    /// Where the current word ends, in bytes.
    [[nodiscard]] std::size_t endPos() const { return m_pos + m_word.size(); }

    [[nodiscard]] const std::string& word() const { return m_word; }

    /// Skips the current word.
    void ignore();
    /// Skips the current word and, for the session, every other instance.
    void ignoreAll();
    /// Adds the current word to the personal word list.
    void add();

    /// Replaces the current word, and remembers the pair for the replacement
    /// list; the position moves past the replacement.
    void replace(std::string_view replacement);
    /// `replace`, and the same word is replaced silently wherever `next` meets it.
    void replaceAll(std::string_view replacement);

    /// Removes the white space after the current word.
    void joinWithNext();
    /// Removes the white space before the current word and rewinds to the
    /// start of the compound it makes.
    void joinWithPrevious();

    /// Whether the character just before / after the current word is white
    /// space — the whole code point, not its last byte.
    [[nodiscard]] bool spaceBefore() const;
    [[nodiscard]] bool spaceAfter() const;

    [[nodiscard]] std::vector<std::string> suggest() const;

private:
    [[nodiscard]] std::string_view leadingCharacter() const;
    [[nodiscard]] std::string_view trailingCharacter() const;

    SpellChecker m_checker;
    std::string m_text;
    std::size_t m_pos = 0;
    std::string m_word;
    std::map<std::string, std::string, std::less<>> m_replacements;
};

} // namespace subedit::core
