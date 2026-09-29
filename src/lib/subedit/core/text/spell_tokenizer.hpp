#pragma once

// Splitting a text into the words the spell-checker looks at — Gaupol's
// `SpellCheckTokenizer`, verbatim (`aeidon/spell.py`), decision D6 of the spec
// of phase 12, issue #507.
//
// **It admits it is imperfect**, and stays Gaupol's until a case takes it
// out: joining and splitting words depend on exactly where it cuts.

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

struct SpellWord {
    /// Where the word starts, in bytes of the UTF-8 text.
    std::size_t offset;
    std::string word;

    friend bool operator==(const SpellWord&, const SpellWord&) = default;
};

/// The words of `text`, in order: a run that starts with a letter or a digit
/// and goes on to the next character that is neither a word character
/// (letter, digit, underscore) nor an apostrophe, minus what non-word
/// characters trail it — so `don't` is one word and `rock'n'` is `rock'n`.
/// A word of one character, or of digits alone, is not one.
[[nodiscard]] std::vector<SpellWord> tokenizeForSpelling(std::string_view text);

} // namespace subedit::core
