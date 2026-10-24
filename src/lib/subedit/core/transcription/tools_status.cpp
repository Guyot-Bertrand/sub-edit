#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/transcription/tools_status.hpp>
#include <subedit/core/transcription/whisper_cpp_engine.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <cstddef>
#include <fstream>
#include <ios>
#include <string>

namespace subedit::core {

namespace {

constexpr std::string_view kExtractorName = "ffmpeg";

/// What the first bytes of `model` say. **Only the first bytes are read**: a model weighs hundreds
/// of megabytes, and the window asks every time it opens.
[[nodiscard]] ModelState modelStateOf(const std::filesystem::path& model) {
    if (model.empty())
        return ModelState::NotSet;
    std::ifstream file{model, std::ios::binary};
    if (!file)
        return ModelState::NotFound;
    std::string header(kModelHeaderSize, '\0');
    file.read(header.data(), static_cast<std::streamsize>(header.size()));
    header.resize(static_cast<std::size_t>(file.gcount()));
    const std::optional<bool> multilingual = readModelIsMultilingual(header);
    if (!multilingual)
        return ModelState::NotAModel;
    return *multilingual ? ModelState::Multilingual : ModelState::EnglishOnly;
}

} // namespace

TranscriptionTools inspectTranscriptionTools(const FileSystem& files,
                                             std::string_view searchPath,
                                             const TranscriptionSettings& settings) {
    TranscriptionTools tools;
    if (settings.engine == kWhisperCppName)
        tools.engine = findWhisperCli(
            files,
            searchPath,
            settings.executable.empty() ? std::nullopt : std::optional{settings.executable});
    tools.extractor = findExecutable(files, kExtractorName, searchPath).has_value();
    tools.model = modelStateOf(settings.model);
    return tools;
}

} // namespace subedit::core
