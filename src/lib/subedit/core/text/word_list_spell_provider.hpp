#pragma once

// A dictionary written in the test — decision D6 of the spec of phase 12,
// issue #507: a list of words and their suggestions, so that no test depends
// on the dictionaries installed on the machine that runs it: a test that did
// would pass on one machine and fail on another.

#include <subedit/core/text/spell_dictionary.hpp>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The words of one language and what is suggested for a misspelt one.
struct WordList {
    std::set<std::string, std::less<>> words;
    std::map<std::string, std::vector<std::string>, std::less<>> suggestions;
    /// Every word added through `addToPersonal`, in order — what a test reads
    /// to see that the personal list was reached.
    std::shared_ptr<std::vector<std::string>> personal =
        std::make_shared<std::vector<std::string>>();
};

/// A provider of `WordList`s by language code. A language with no list has no
/// dictionary, which is what `open` answers with a null.
class WordListSpellProvider final : public SpellProvider {

public:
    WordListSpellProvider() = default;

    /// Gives `language` a dictionary made of `list`.
    void add(std::string language, WordList list);

    [[nodiscard]] std::vector<std::string> languages() const override;

    [[nodiscard]] std::unique_ptr<SpellDictionary> open(std::string_view language) const override;

private:
    std::map<std::string, WordList, std::less<>> m_lists;
};

} // namespace subedit::core
