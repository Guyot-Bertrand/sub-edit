#include <subedit/core/config/correction_settings.hpp>

#include <algorithm>

namespace subedit::core {

bool patternEnabled(const CorrectionPattern& pattern, const CorrectionSettings& settings) {
    for (const PatternActivation& activation : settings.patternActivations) {
        if (activation.kind == pattern.kind() && activation.code == pattern.code &&
            activation.name == pattern.name)
            return activation.enabled;
    }
    return pattern.enabled;
}

void foldActivations(std::vector<PatternActivation>& activations,
                     std::span<const CorrectionPattern* const> shown,
                     std::span<const PatternActivation> overrides) {
    std::erase_if(activations, [shown](const PatternActivation& activation) {
        return std::ranges::any_of(shown, [&activation](const CorrectionPattern* record) {
            return record->kind() == activation.kind && record->code == activation.code &&
                   record->name == activation.name;
        });
    });
    activations.insert(activations.end(), overrides.begin(), overrides.end());
}

} // namespace subedit::core
