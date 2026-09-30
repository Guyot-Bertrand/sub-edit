#pragma once

// Where the unknown words of a text are — issue #525, `GUI-SPELL-04`: what the
// cell editor underlines while one types.

#include <subedit/core/text/spell_checker.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace subedit::core {

/// A word the checker refuses, as a run of bytes of the UTF-8 text.
struct SpellRange {
    std::size_t offset;
    std::size_t length;

    friend bool operator==(const SpellRange&, const SpellRange&) = default;
};

/// The words of `text` that `checker` refuses, in order.
///
/// **The same rule as `SpellCheckNavigator::next`**: the words are the
/// tokenizer's, and each is asked with the character before and the one after
/// it, so that what is underlined is what `Check Spelling…` stops on.
[[nodiscard]] std::vector<SpellRange> misspelledSpellRanges(const SpellChecker& checker,
                                                            std::string_view text);

} // namespace subedit::core
