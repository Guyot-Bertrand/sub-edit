#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/transcription/engine_registry.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace subedit::core {

/// The engines this build knows: whisper.cpp, found where `configured` says or on
/// `searchPath`. **It is registered even when it is not there**, and then reports itself
/// missing when asked what it can do — the feature disables itself with a clear message,
/// as the roadmap's rule for every external tool has it, instead of the engine
/// vanishing from the choices.
[[nodiscard]] EngineRegistry
makeDefaultEngines(const FileSystem& files,
                   std::string_view searchPath,
                   const std::optional<std::filesystem::path>& configuredExecutable);

} // namespace subedit::core
