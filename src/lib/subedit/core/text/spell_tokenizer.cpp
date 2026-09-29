#include <subedit/core/text/spell_tokenizer.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/uchar.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

[[nodiscard]] bool isDigitsOnly(std::string_view word) {
    for (std::size_t at = 0; at < word.size(); at = nextCodePoint(word, at)) {
        if (!u_isdigit(static_cast<UChar32>(codePointAt(word, at))))
            return false;
    }
    return true;
}

[[nodiscard]] std::size_t codePointCount(std::string_view word) {
    std::size_t count = 0;
    for (std::size_t at = 0; at < word.size(); at = nextCodePoint(word, at))
        ++count;
    return count;
}

/// `re.split(r"(?!')\W+", text)[0]`: everything up to the first character that
/// is not a word character and not an apostrophe.
[[nodiscard]] std::size_t firstPieceEnd(std::string_view text, std::size_t from) {
    std::size_t end = from;
    while (end < text.size()) {
        const char32_t c = codePointAt(text, end);
        if (!isSpellWordCharacter(c) && c != U'\'')
            break;
        end = nextCodePoint(text, end);
    }
    return end;
}

/// `re.sub(r"\W+$", "", word)` on `text[from, end)`: the end once trailing
/// non-word characters are dropped.
[[nodiscard]] std::size_t
withoutTrailingNonWord(std::string_view text, std::size_t from, std::size_t end) {
    while (end > from) {
        const std::size_t before = previousCodePoint(text, end);
        if (isSpellWordCharacter(codePointAt(text, before)))
            break;
        end = before;
    }
    return end;
}

} // namespace

bool isSpellWordCharacter(char32_t c) {
    return u_isalnum(static_cast<UChar32>(c)) || c == U'_';
}

std::vector<SpellWord> tokenizeForSpelling(std::string_view text) {
    std::vector<SpellWord> words;
    std::size_t at = 0;
    while (at < text.size()) {
        if (!u_isalnum(static_cast<UChar32>(codePointAt(text, at)))) {
            at = nextCodePoint(text, at);
            continue;
        }
        const std::size_t end = withoutTrailingNonWord(text, at, firstPieceEnd(text, at));
        const std::string_view word = text.substr(at, end - at);
        if (codePointCount(word) > 1 && !isDigitsOnly(word))
            words.push_back({.offset = at, .word = std::string{word}});
        at = end;
    }
    return words;
}

} // namespace subedit::core
