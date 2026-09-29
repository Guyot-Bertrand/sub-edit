#include <subedit/core/text/word_list_spell_provider.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

class WordListDictionary final : public SpellDictionary {

public:
    explicit WordListDictionary(WordList list) : m_list(std::move(list)) {}

    [[nodiscard]] bool isCorrect(std::string_view word) const override {
        return m_list.words.contains(word);
    }

    [[nodiscard]] std::vector<std::string> suggestions(std::string_view word) const override {
        const auto found = m_list.suggestions.find(word);
        return found == m_list.suggestions.end() ? std::vector<std::string>{} : found->second;
    }

    void addToPersonal(std::string_view word) override {
        m_list.words.emplace(word);
        m_list.personal->emplace_back(word);
    }

private:
    WordList m_list;
};

} // namespace

void WordListSpellProvider::add(std::string language, WordList list) {
    m_lists.insert_or_assign(std::move(language), std::move(list));
}

std::vector<std::string> WordListSpellProvider::languages() const {
    std::vector<std::string> codes;
    codes.reserve(m_lists.size());
    for (const auto& [code, list] : m_lists)
        codes.push_back(code);
    return codes;
}

std::unique_ptr<SpellDictionary> WordListSpellProvider::open(std::string_view language) const {
    const auto found = m_lists.find(language);
    if (found == m_lists.end())
        return nullptr;
    return std::make_unique<WordListDictionary>(found->second);
}

} // namespace subedit::core
