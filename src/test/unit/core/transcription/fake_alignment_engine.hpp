#pragma once

// An alignment engine that aligns nothing, and answers what it was told to.
//
// As the fake transcription engine does for the first port: deterministic, instantaneous, there on
// every machine, and **honest about its own capabilities**, so that it passes the contract suite
// and shows what a conforming engine looks like. It reads nothing of the audio file but its
// existence.

#include <subedit/core/transcription/alignment_engine.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace subedit::test {

struct FakeAlignmentScript {
    std::string name = "fake-alignment";
    core::EngineCapabilities capabilities{.needsModel = false};

    /// What each line gets, in order. Lines beyond the script get nothing (not found).
    std::vector<core::AlignedLine> answers{};

    /// Where the last request received is left, when a test wants to read it.
    std::shared_ptr<core::AlignmentRequest> seen{};
};

class FakeAlignmentEngine final : public core::AlignmentEngine {

public:
    explicit FakeAlignmentEngine(FakeAlignmentScript script) : m_script(std::move(script)) {}

    [[nodiscard]] std::string_view name() const override { return m_script.name; }

    [[nodiscard]] std::expected<core::EngineCapabilities, core::TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& /*model*/) const override {
        return m_script.capabilities;
    }

    [[nodiscard]] std::expected<std::vector<core::AlignedLine>, core::TranscriptionError>
    align(const core::AlignmentRequest& request,
          const core::TranscriptionObserver& observer) const override {
        // Refused **before any work**, as the contract asks: no progress is reported.
        if (m_script.capabilities.needsModel && !request.model)
            return std::unexpected(
                core::TranscriptionError{.kind = core::TranscriptionErrorKind::ModelMissing});
        if (!std::filesystem::exists(request.audio))
            return std::unexpected(
                core::TranscriptionError{.kind = core::TranscriptionErrorKind::AudioUnreadable,
                                         .detail = request.audio.string()});
        if (m_script.seen)
            *m_script.seen = request;

        constexpr int kSteps = 4;
        for (int step = 1; step <= kSteps; ++step) {
            if (observer.isCancelled())
                return std::unexpected(
                    core::TranscriptionError{.kind = core::TranscriptionErrorKind::Cancelled});
            observer.advance({.fraction = static_cast<double>(step) / kSteps});
        }

        std::vector<core::AlignedLine> answer(request.lines.size());
        for (std::size_t line = 0; line < answer.size() && line < m_script.answers.size(); ++line)
            answer[line] = m_script.answers[line];
        return answer;
    }

private:
    FakeAlignmentScript m_script;
};

} // namespace subedit::test
