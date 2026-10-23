#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/transcription/transcription_engine.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace subedit::core {

/// The name the setting `transcription.engine` uses for this engine.
inline constexpr std::string_view kWhisperCppName = "whisper.cpp";

/// The executable of whisper.cpp: `configured` when the user named one, else
/// `whisper-cli` on `searchPath`. Nothing when it is not there — the engine then
/// reports itself missing, and the feature disables itself (the roadmap's rule
/// for every external tool).
[[nodiscard]] std::optional<std::filesystem::path>
findWhisperCli(const FileSystem& files,
               std::string_view searchPath,
               const std::optional<std::filesystem::path>& configured);

/// The first adapter of the transcription port: it runs `whisper-cli` as a
/// subprocess (ADR 0046, D6 of the phase 22 spec).
///
/// It works in a temporary directory, since the tool writes files and not a
/// value; reads the JSON of `-oj -ojf`; follows the progress the tool prints on
/// standard error; and maps its exit codes (2, 3 and 11 for a missing input, a
/// missing model and a file that is not audio) to the port's errors.
///
/// **What it can do depends on the model**, and it reads that from the model
/// itself: a multilingual one detects the language and transcribes any; an
/// English-only one does neither. Timed words are rebuilt from the tokens.
class WhisperCppEngine final : public TranscriptionEngine {

public:
    /// `executable` is the program to run, as `findWhisperCli` found it.
    explicit WhisperCppEngine(std::filesystem::path executable);

    [[nodiscard]] std::string_view name() const override { return kWhisperCppName; }

    [[nodiscard]] std::expected<EngineCapabilities, TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& model) const override;

protected:
    [[nodiscard]] std::expected<Transcription, TranscriptionError>
    run(const TranscriptionRequest& request,
        const EngineCapabilities& capabilities,
        const TranscriptionObserver& observer) const override;

private:
    std::filesystem::path m_executable;
};

} // namespace subedit::core
