#include <subedit/core/text/spell_ranges.hpp>
#include <subedit/core/text/spell_tokenizer.hpp>
#include <subedit/core/text/utf8.hpp>

namespace subedit::core {

std::vector<SpellRange> misspelledSpellRanges(const SpellChecker& checker, std::string_view text) {
    std::vector<SpellRange> ranges;
    for (const SpellWord& found : tokenizeForSpelling(text)) {
        const std::size_t end = found.offset + found.word.size();
        const std::string_view leading =
            found.offset == 0 ? std::string_view{}
                              : text.substr(previousCodePoint(text, found.offset),
                                            found.offset - previousCodePoint(text, found.offset));
        const std::string_view trailing = end >= text.size()
                                              ? std::string_view{}
                                              : text.substr(end, nextCodePoint(text, end) - end);
        if (!checker.check(found.word, leading, trailing))
            ranges.push_back({.offset = found.offset, .length = found.word.size()});
    }
    return ranges;
}

} // namespace subedit::core
