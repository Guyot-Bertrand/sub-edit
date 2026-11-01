#pragma once

// A speech detector that finds what it is told to, and records what it was asked.

#include <subedit/core/transcription/speech_detector.hpp>

#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

namespace subedit::test {

class FakeSpeechDetector final : public core::SpeechDetector {

public:
    explicit FakeSpeechDetector(std::vector<core::SpeechSpan> spans) : m_spans(std::move(spans)) {}

    /// A detector that fails with `error`.
    explicit FakeSpeechDetector(core::TranscriptionError error) : m_failure(std::move(error)) {}

    [[nodiscard]] std::expected<std::vector<core::SpeechSpan>, core::TranscriptionError>
    detect(const std::filesystem::path& audio,
           const std::optional<std::filesystem::path>& model,
           const core::TranscriptionObserver& /*observer*/) const override {
        m_audio = audio;
        m_model = model;
        if (m_failure)
            return std::unexpected(*m_failure);
        return m_spans;
    }

    [[nodiscard]] const std::filesystem::path& audio() const { return m_audio; }

    [[nodiscard]] const std::optional<std::filesystem::path>& model() const { return m_model; }

private:
    std::vector<core::SpeechSpan> m_spans;
    std::optional<core::TranscriptionError> m_failure;
    mutable std::filesystem::path m_audio;
    mutable std::optional<std::filesystem::path> m_model;
};

} // namespace subedit::test
