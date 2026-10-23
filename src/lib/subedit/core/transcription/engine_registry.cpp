#include <subedit/core/transcription/engine_registry.hpp>

#include <algorithm>
#include <utility>

namespace subedit::core {

bool EngineRegistry::add(std::unique_ptr<TranscriptionEngine> engine) {
    if (!engine || find(engine->name()) != nullptr)
        return false;
    m_engines.push_back(std::move(engine));
    return true;
}

const TranscriptionEngine* EngineRegistry::find(std::string_view name) const {
    const auto found = std::ranges::find_if(
        m_engines, [name](const auto& engine) { return engine->name() == name; });
    return found == m_engines.end() ? nullptr : found->get();
}

std::vector<std::string> EngineRegistry::names() const {
    std::vector<std::string> names;
    names.reserve(m_engines.size());
    for (const auto& engine : m_engines)
        names.emplace_back(engine->name());
    return names;
}

} // namespace subedit::core
