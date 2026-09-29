#pragma once

// The system's dictionaries, through Enchant 2 — decision D6 of the spec of
// phase 12, issue #507.
//
// **Enchant is not in this header**: `enchant.h` is included by the one
// implementation file, and what depends on `subedit_core` inherits the link
// and nothing else.

#include <subedit/core/text/spell_dictionary.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// Owns the `EnchantBroker`; defined where `enchant.h` is.
struct EnchantBrokerHandle;

class EnchantSpellProvider final : public SpellProvider {

public:
    EnchantSpellProvider();
    ~EnchantSpellProvider() override;

    EnchantSpellProvider(const EnchantSpellProvider&) = delete;
    EnchantSpellProvider& operator=(const EnchantSpellProvider&) = delete;
    EnchantSpellProvider(EnchantSpellProvider&&) = delete;
    EnchantSpellProvider& operator=(EnchantSpellProvider&&) = delete;

    [[nodiscard]] std::vector<std::string> languages() const override;

    /// **A dictionary must not outlive the provider it came from**: it shares
    /// the broker, but the handle is kept alive only by what holds it.
    [[nodiscard]] std::unique_ptr<SpellDictionary> open(std::string_view language) const override;

    /// A dictionary made of one file of words, one per line, with no language
    /// or provider behind it — Enchant's own personal-word-list machinery. A
    /// word added goes into that file.
    ///
    /// **What the tests use to reach the real wrapper** without depending on
    /// the dictionaries of the machine that runs them.
    [[nodiscard]] std::unique_ptr<SpellDictionary>
    openWordList(const std::filesystem::path& file) const;

private:
    std::shared_ptr<EnchantBrokerHandle> m_broker;
};

} // namespace subedit::core
