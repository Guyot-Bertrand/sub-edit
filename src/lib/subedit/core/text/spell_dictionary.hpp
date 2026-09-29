#pragma once

// What the spell-checker asks of a dictionary — decision D6 of the spec of
// phase 12, issue #507.
//
// **Two interfaces, and `SpellChecker` is not one of them.** D6 speaks of "an
// interface of the core, `SpellChecker`, and a double for the tests"; what is
// abstract turned out to be the dictionary alone. What Gaupol puts on top of
// one — the English heuristics, the two OCR suggestions, the replacement list
// — is the same for every dictionary and is what the tests are after, so it
// is one concrete class (`spell_checker.hpp`) over these two.

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The dictionary of one language.
class SpellDictionary {

public:
    virtual ~SpellDictionary() = default;

    /// Whether `word` is spelt correctly. Nothing more: no heuristic, no
    /// session list — those are `SpellChecker`'s.
    [[nodiscard]] virtual bool isCorrect(std::string_view word) const = 0;

    /// The dictionary's own suggestions for `word`, best first.
    [[nodiscard]] virtual std::vector<std::string> suggestions(std::string_view word) const = 0;

    /// Adds `word` to the user's personal word list. **With Enchant this is
    /// shared with every other program that uses it** — Gaupol's list is this
    /// one, which is the reason D6 chose it.
    virtual void addToPersonal(std::string_view word) = 0;

protected:
    SpellDictionary() = default;
    SpellDictionary(const SpellDictionary&) = default;
    SpellDictionary(SpellDictionary&&) = default;
    SpellDictionary& operator=(const SpellDictionary&) = default;
    SpellDictionary& operator=(SpellDictionary&&) = default;
};

/// Where dictionaries come from: the system's, through Enchant, or a test's.
class SpellProvider {

public:
    virtual ~SpellProvider() = default;

    /// Every language code the provider knows, unfiltered — Enchant lists odd
    /// entries that are not locale codes, `availableSpellLanguages` sorts
    /// those out.
    [[nodiscard]] virtual std::vector<std::string> languages() const = 0;

    /// The dictionary of `language`, or null if there is none.
    [[nodiscard]] virtual std::unique_ptr<SpellDictionary>
    open(std::string_view language) const = 0;

protected:
    SpellProvider() = default;
    SpellProvider(const SpellProvider&) = default;
    SpellProvider(SpellProvider&&) = default;
    SpellProvider& operator=(const SpellProvider&) = default;
    SpellProvider& operator=(SpellProvider&&) = default;
};

/// Whether `code` is a locale code — `aeidon.locales.is_valid`, verbatim:
/// two lower-case letters, then optionally `_` and two upper-case, then
/// optionally `@` and a four-letter script (`fr`, `en_US`, `sr@Latn`).
[[nodiscard]] bool isValidSpellLanguage(std::string_view code);

/// The languages `provider` offers that are valid locale codes, sorted —
/// `SpellChecker.list_languages`.
[[nodiscard]] std::vector<std::string> availableSpellLanguages(const SpellProvider& provider);

} // namespace subedit::core
