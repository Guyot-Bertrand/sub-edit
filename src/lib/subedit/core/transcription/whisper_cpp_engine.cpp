#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/io/temporary_directory.hpp>
#include <subedit/core/process/start_process.hpp>
#include <subedit/core/transcription/whisper_cpp_engine.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kExecutableName = "whisper-cli";

/// What `whisper-cli` exits with, checked on 1.9.5-dev.
constexpr int kMissingInput = 2;
constexpr int kMissingModel = 3;
constexpr int kNotAudio = 11;

[[nodiscard]] std::string readAll(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

[[nodiscard]] TranscriptionError error(TranscriptionErrorKind kind, std::string detail = {}) {
    return TranscriptionError{.kind = kind, .detail = std::move(detail)};
}

/// The error the tool's exit code stands for.
[[nodiscard]] TranscriptionError errorOfExit(int code, std::string lastError) {
    switch (code) {
    case kMissingModel:
        return error(TranscriptionErrorKind::ModelMissing, std::move(lastError));
    case kMissingInput:
    case kNotAudio:
        return error(TranscriptionErrorKind::AudioUnreadable, std::move(lastError));
    default:
        return error(TranscriptionErrorKind::Failed, std::move(lastError));
    }
}

} // namespace

std::optional<std::filesystem::path>
findWhisperCli(const FileSystem& files,
               std::string_view searchPath,
               const std::optional<std::filesystem::path>& configured) {
    if (configured && !configured->empty())
        return files.exists(*configured) ? configured : std::nullopt;
    return findExecutable(files, kExecutableName, searchPath);
}

WhisperCppEngine::WhisperCppEngine(std::filesystem::path executable)
    : m_executable(std::move(executable)) {}

std::expected<EngineCapabilities, TranscriptionError>
WhisperCppEngine::capabilitiesFor(const std::optional<std::filesystem::path>& model) const {
    if (!std::filesystem::exists(m_executable))
        return std::unexpected(error(TranscriptionErrorKind::EngineMissing, m_executable.string()));
    if (!model || !std::filesystem::exists(*model))
        return std::unexpected(error(TranscriptionErrorKind::ModelMissing));

    const std::string header = readAll(*model).substr(0, kModelHeaderSize);
    const std::optional<bool> multilingual = readModelIsMultilingual(header);
    if (!multilingual)
        return std::unexpected(error(TranscriptionErrorKind::Failed, model->string()));

    EngineCapabilities capabilities{.timedWords = true, .needsModel = true};
    if (*multilingual)
        capabilities.languageDetection = true;
    else
        capabilities.languages = {"en"};
    return capabilities;
}

std::expected<Transcription, TranscriptionError>
WhisperCppEngine::run(const TranscriptionRequest& request,
                      const EngineCapabilities& /*capabilities*/,
                      const TranscriptionObserver& observer) const {
    if (!std::filesystem::exists(request.audio))
        return std::unexpected(
            error(TranscriptionErrorKind::AudioUnreadable, request.audio.string()));

    const TemporaryDirectory work{"subedit-whisper"};
    if (!work.made())
        return std::unexpected(error(TranscriptionErrorKind::Failed));
    const std::filesystem::path prefix = work.path() / "out";

    const std::vector<std::string> arguments{
        "-m",
        request.model.value_or(std::filesystem::path{}).string(),
        "-f",
        request.audio.string(),
        "-l",
        request.language.value_or("auto"),
        "-pp",
        "-oj",
        "-ojf",
        "-of",
        prefix.string()};

    std::string lastError;
    StreamObserver streamed;
    streamed.cancelled = [&observer] { return observer.isCancelled(); };
    streamed.onLine = [&](const StreamedLine& line) {
        if (line.stream != StreamName::Error)
            return;
        if (const std::optional<double> progress = readWhisperProgress(line.text))
            observer.report(*progress);
        else if (line.text.starts_with("error:"))
            lastError = line.text;
    };

    const std::expected<StreamedRun, LaunchError> ran =
        runStreaming(m_executable, arguments, streamed);
    if (!ran)
        return std::unexpected(error(ran.error().kind == LaunchErrorKind::NotFound
                                         ? TranscriptionErrorKind::EngineMissing
                                         : TranscriptionErrorKind::Failed,
                                     ran.error().detail));
    if (ran->cancelled)
        return std::unexpected(error(TranscriptionErrorKind::Cancelled));
    if (ran->code != 0)
        return std::unexpected(errorOfExit(ran->code, std::move(lastError)));

    std::optional<WhisperOutput> output = readWhisperJson(readAll(work.path() / "out.json"));
    if (!output)
        return std::unexpected(error(TranscriptionErrorKind::Failed));

    observer.report(1.0);
    return std::move(output->transcription);
}

} // namespace subedit::core
