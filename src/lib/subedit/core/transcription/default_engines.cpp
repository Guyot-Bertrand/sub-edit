#include <subedit/core/transcription/default_engines.hpp>
#include <subedit/core/transcription/whisper_cpp_engine.hpp>

#include <memory>

namespace subedit::core {

EngineRegistry
makeDefaultEngines(const FileSystem& files,
                   std::string_view searchPath,
                   const std::optional<std::filesystem::path>& configuredExecutable) {
    EngineRegistry engines;
    const std::optional<std::filesystem::path> found =
        findWhisperCli(files, searchPath, configuredExecutable);
    // The name alone when it was not found: nothing is at that path, which is what the
    // engine reports.
    (void)engines.add(std::make_unique<WhisperCppEngine>(found.value_or("whisper-cli")));
    return engines;
}

} // namespace subedit::core
