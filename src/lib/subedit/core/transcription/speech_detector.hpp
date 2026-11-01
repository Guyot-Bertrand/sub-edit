#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/transcription_engine.hpp>
#include <subedit/core/transcription/transcription_error.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

namespace subedit::core {

/// A stretch of audio in which somebody speaks, in the time of the audio.
struct SpeechSpan {
    Timestamp start;
    Timestamp end;

    friend bool operator==(const SpeechSpan&, const SpeechSpan&) = default;
};

/// The program of whisper.cpp that finds where there is speech, and nothing else.
inline constexpr std::string_view kVadToolName = "whisper-vad-speech-segments";

/// Where the program is on `searchPath`; nothing when it is not there.
[[nodiscard]] std::optional<std::filesystem::path> findVadTool(const FileSystem& files,
                                                               std::string_view searchPath);

/// What `whisper-vad-speech-segments` prints on its standard output: one
/// `Speech segment N: start = A, end = B` line per stretch, in **hundredths of a second**. Any
/// other line is skipped, and so is a stretch that ends before it begins.
[[nodiscard]] std::vector<SpeechSpan> parseSpeechSegments(std::string_view output);

/// The third port of phase 23: audio in, the stretches where somebody speaks out.
///
/// It knows nothing of what is said, and so nothing of the language: that is what lets a subtitle
/// file in one language be put right on the sound of a film in another.
class SpeechDetector {

public:
    SpeechDetector(const SpeechDetector&) = delete;
    SpeechDetector(SpeechDetector&&) = delete;
    SpeechDetector& operator=(const SpeechDetector&) = delete;
    SpeechDetector& operator=(SpeechDetector&&) = delete;
    virtual ~SpeechDetector() = default;

    /// The stretches of `audio`, in order. `model` is the voice-activity model the detector runs.
    [[nodiscard]] virtual std::expected<std::vector<SpeechSpan>, TranscriptionError>
    detect(const std::filesystem::path& audio,
           const std::optional<std::filesystem::path>& model,
           const TranscriptionObserver& observer) const = 0;

protected:
    SpeechDetector() = default;
};

/// Runs `whisper-vad-speech-segments` on the audio.
class WhisperVadDetector final : public SpeechDetector {

public:
    /// `executable` is the program to run, as `findVadTool` found it.
    explicit WhisperVadDetector(std::filesystem::path executable);

    [[nodiscard]] std::expected<std::vector<SpeechSpan>, TranscriptionError>
    detect(const std::filesystem::path& audio,
           const std::optional<std::filesystem::path>& model,
           const TranscriptionObserver& observer) const override;

private:
    std::filesystem::path m_executable;
};

} // namespace subedit::core
