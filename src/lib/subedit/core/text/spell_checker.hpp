#pragma once

// Checking and suggesting words, as Gaupol's `SpellChecker` does
// (`aeidon/spell.py`) — decision D6 of the spec of phase 12, issue #507.
//
// **What is here is what Gaupol adds on top of a dictionary**, taken over
// line for line: the English heuristics, the two OCR suggestions, the
// replacement list, the words ignored for the session. The dictionary itself
// is behind `SpellDictionary`.

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/text/spell_dictionary.hpp>
#include <subedit/core/text/spell_replacements.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

class SpellChecker {

public:
    SpellChecker(std::unique_ptr<SpellDictionary> dictionary,
                 std::string language,
                 std::vector<SpellReplacement> replacements = {});

    [[nodiscard]] const std::string& language() const { return m_language; }

    /// Whether `word` is correct, given the characters around it.
    ///
    /// **For English only** (a code starting with `en`), the special cases
    /// Gaupol makes for the informal spoken language subtitles are full of:
    /// `goin` before an apostrophe is also tried as `going`; a word ending in
    /// `'d`, `'ll`, `'re`, `'s` or `'ve` is checked without it; and an ordinal
    /// numeral — `1st`, `22nd`, `13th` — is correct. Anything else goes to the
    /// dictionary, after the words ignored for the session.
    [[nodiscard]] bool check(std::string_view word,
                             std::string_view leadingContext = {},
                             std::string_view trailingContext = {}) const;

    /// Whether all of `words` are correct, or any of them — no heuristic.
    [[nodiscard]] bool checkAll(std::span<const std::string> words) const;
    [[nodiscard]] bool checkAny(std::span<const std::string> words) const;

    /// Suggestions for `word`, without duplicates, in this order: what the
    /// user replaced it with before; `I` read as `l`, the commonest OCR
    /// error, if that is correct; a number stuck to its unit, `5km`, spaced
    /// out, if both halves are correct; then the dictionary's own.
    [[nodiscard]] std::vector<std::string> suggest(std::string_view word) const;

    /// Adds `word` to the user's personal word list.
    void addToPersonal(std::string_view word);

    /// Ignores `word` until this checker goes.
    void addToSession(std::string_view word);

    /// Remembers that `word` was replaced with `replacement`, for
    /// `saveReplacements` and for `suggest`.
    void addReplacement(std::string_view word, std::string_view replacement);

    [[nodiscard]] std::span<const SpellReplacement> replacements() const { return m_replacements; }

private:
    [[nodiscard]] bool isCorrectWord(std::string_view word) const;

    std::unique_ptr<SpellDictionary> m_dictionary;
    std::string m_language;
    std::vector<SpellReplacement> m_replacements;
    std::set<std::string, std::less<>> m_sessionWords;
};

/// There is no dictionary for `language`: what the window shows, greyed, in
/// `noDictionaryFor`'s words.
struct NoDictionary {
    std::string language;

    friend bool operator==(const NoDictionary&, const NoDictionary&) = default;
};

/// A checker for `language`, with the replacements its file holds — an absent
/// or unreadable file holds none, as in Gaupol — or `NoDictionary`.
[[nodiscard]] std::expected<SpellChecker, NoDictionary>
openSpellChecker(const SpellProvider& provider,
                 std::string_view language,
                 const FileSystem& files,
                 const std::filesystem::path& replacementFile);

/// Writes `checker`'s replacements to `replacementFile`, its directory made if
/// need be. **Nothing is written when there are none**, like Gaupol.
[[nodiscard]] std::expected<void, FileError> saveSpellReplacements(
    const SpellChecker& checker, FileSystem& files, const std::filesystem::path& replacementFile);

} // namespace subedit::core
