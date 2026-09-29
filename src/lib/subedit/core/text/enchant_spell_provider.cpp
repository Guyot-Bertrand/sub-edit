#include <subedit/core/text/enchant_spell_provider.hpp>

#include <cstddef>
#include <enchant.h>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

/// An `EnchantBroker`, freed when the last dictionary and the provider let go
/// of it — Enchant frees a dictionary through its broker, so the broker has to
/// outlast every dictionary.
struct EnchantBrokerHandle {
    EnchantBroker* handle = enchant_broker_init();

    EnchantBrokerHandle() = default;
    EnchantBrokerHandle(const EnchantBrokerHandle&) = delete;
    EnchantBrokerHandle& operator=(const EnchantBrokerHandle&) = delete;
    EnchantBrokerHandle(EnchantBrokerHandle&&) = delete;
    EnchantBrokerHandle& operator=(EnchantBrokerHandle&&) = delete;

    ~EnchantBrokerHandle() {
        if (handle != nullptr)
            enchant_broker_free(handle);
    }
};

namespace {

class EnchantDictionary final : public SpellDictionary {

public:
    EnchantDictionary(std::shared_ptr<EnchantBrokerHandle> broker, EnchantDict* handle)
        : m_broker(std::move(broker)), m_handle(handle) {}

    ~EnchantDictionary() override { enchant_broker_free_dict(m_broker->handle, m_handle); }

    EnchantDictionary(const EnchantDictionary&) = delete;
    EnchantDictionary& operator=(const EnchantDictionary&) = delete;
    EnchantDictionary(EnchantDictionary&&) = delete;
    EnchantDictionary& operator=(EnchantDictionary&&) = delete;

    [[nodiscard]] bool isCorrect(std::string_view word) const override {
        // Zero is a correct word, positive a wrong one, negative an error —
        // and a word Enchant could not judge is not reported as a mistake.
        return enchant_dict_check(m_handle, word.data(), static_cast<ssize_t>(word.size())) <= 0;
    }

    [[nodiscard]] std::vector<std::string> suggestions(std::string_view word) const override {
        std::size_t count = 0;
        char** found =
            enchant_dict_suggest(m_handle, word.data(), static_cast<ssize_t>(word.size()), &count);
        std::vector<std::string> suggestions;
        if (found == nullptr)
            return suggestions;
        suggestions.reserve(count);
        for (std::size_t index = 0; index < count; ++index)
            suggestions.emplace_back(found[index]);
        enchant_dict_free_string_list(m_handle, found);
        return suggestions;
    }

    void addToPersonal(std::string_view word) override {
        enchant_dict_add(m_handle, word.data(), static_cast<ssize_t>(word.size()));
    }

private:
    std::shared_ptr<EnchantBrokerHandle> m_broker;
    EnchantDict* m_handle;
};

void collectLanguage(const char* tag,
                     const char* /*providerName*/,
                     const char* /*providerDescription*/,
                     const char* /*providerFile*/,
                     void* into) {
    static_cast<std::vector<std::string>*>(into)->emplace_back(tag);
}

} // namespace

EnchantSpellProvider::EnchantSpellProvider() : m_broker(std::make_shared<EnchantBrokerHandle>()) {}

EnchantSpellProvider::~EnchantSpellProvider() = default;

std::vector<std::string> EnchantSpellProvider::languages() const {
    std::vector<std::string> tags;
    enchant_broker_list_dicts(m_broker->handle, &collectLanguage, &tags);
    return tags;
}

std::unique_ptr<SpellDictionary> EnchantSpellProvider::open(std::string_view language) const {
    const std::string tag{language};
    EnchantDict* handle = enchant_broker_request_dict(m_broker->handle, tag.c_str());
    if (handle == nullptr)
        return nullptr;
    return std::make_unique<EnchantDictionary>(m_broker, handle);
}

std::unique_ptr<SpellDictionary>
EnchantSpellProvider::openWordList(const std::filesystem::path& file) const {
    EnchantDict* handle = enchant_broker_request_pwl_dict(m_broker->handle, file.c_str());
    if (handle == nullptr)
        return nullptr;
    return std::make_unique<EnchantDictionary>(m_broker, handle);
}

} // namespace subedit::core
