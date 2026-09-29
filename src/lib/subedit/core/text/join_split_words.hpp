#pragma once

// Joining and splitting words by what the spell-checker says — Gaupol's
// `spell_check_join_words` and `spell_check_split_words`
// (`aeidon/agents/text.py`), decisions D6 and D8 of the spec of phase 12,
// issue #508.
//
// Functions of texts, like `common_errors.cpp`: one text out for each text in,
// unchanged where nothing applied. **The texts are taken raw**, tags and all,
// as Gaupol takes them: a `font` in a tag is a word to the tokenizer.

#include <subedit/core/text/spell_checker.hpp>

#include <span>
#include <string>
#include <vector>

namespace subedit::core {

/// Joins a misspelt word to the word before or after it when **exactly one**
/// of the two directions gives a correctly spelt word — never when both do or
/// neither does. `Hell o` is `Hello`; `a bout` next to `to` decides nothing.
///
/// Runs of spaces are made one before anything is read. A text that came out
/// the same as that collapsed text — nothing joined — is given back as it
/// was, spaces and all.
///
/// **A change is a change here**, which Gaupol's is not always: it compares
/// the result with the text as it stood when the last misspelt word was
/// reached, so a join followed by a misspelt word it leaves alone is lost.
/// Ours compares with the text it started from.
[[nodiscard]] std::vector<std::string> joinWords(const SpellChecker& checker,
                                                 std::span<const std::string> texts);

/// Splits a misspelt word in two when **exactly one** of the checker's
/// suggestions is the word with a space in it — the same letters, in the same
/// order. `Ihavea` is `I have a` only if that is the one suggestion made of
/// nothing but its letters and spaces. A word with a capital first letter and
/// lower case after it is left alone: it is usually a name, and names are not
/// in dictionaries.
[[nodiscard]] std::vector<std::string> splitWords(const SpellChecker& checker,
                                                  std::span<const std::string> texts);

} // namespace subedit::core
