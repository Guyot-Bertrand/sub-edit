#include <subedit/core/config/correction_settings.hpp>

namespace subedit::core {

bool patternEnabled(const CorrectionPattern& pattern, const CorrectionSettings& settings) {
    for (const PatternActivation& activation : settings.patternActivations) {
        if (activation.kind == pattern.kind() && activation.code == pattern.code &&
            activation.name == pattern.name)
            return activation.enabled;
    }
    return pattern.enabled;
}

} // namespace subedit::core
