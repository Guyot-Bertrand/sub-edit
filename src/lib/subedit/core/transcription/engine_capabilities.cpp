#include <subedit/core/transcription/engine_capabilities.hpp>

#include <algorithm>

namespace subedit::core {

bool EngineCapabilities::supportsLanguage(std::string_view language) const {
    return languages.empty() || std::ranges::find(languages, language) != languages.end();
}

} // namespace subedit::core
