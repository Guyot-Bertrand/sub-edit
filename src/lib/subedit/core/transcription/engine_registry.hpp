#pragma once

#include <subedit/core/transcription/transcription_engine.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The engines a build knows, found by the name the setting gives.
///
/// An object and not a global: what the program knows is decided where it
/// starts, and a test builds the registry it needs. Choosing among the engines
/// is the setting's job (`transcription.engine`, #777); this only holds them.
class EngineRegistry {

public:
    /// Adds `engine`. Refused, and false, when one of that name is already there.
    [[nodiscard]] bool add(std::unique_ptr<TranscriptionEngine> engine);

    /// The engine called `name`, or nothing. The registry keeps ownership.
    [[nodiscard]] const TranscriptionEngine* find(std::string_view name) const;

    /// The names, in the order the engines were added.
    [[nodiscard]] std::vector<std::string> names() const;

private:
    std::vector<std::unique_ptr<TranscriptionEngine>> m_engines;
};

} // namespace subedit::core
