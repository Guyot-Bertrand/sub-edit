#pragma once

// A transcription engine that transcribes nothing, and answers what it was told to.
//
// The tests of everything that sits around an engine — the post-processing, the
// registry, the surfaces — need an engine whose answer they know in advance, and
// that exists on every machine. This is it: deterministic, instantaneous, and
// honest about its own capabilities, so that it also passes the contract suite
// and shows what a conforming engine looks like.
//
// It reads nothing of the audio file but its existence.

#include <subedit/core/transcription/transcription_engine.hpp>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace subedit::test {

struct FakeEngineScript {
    std::string name = "fake";
    core::EngineCapabilities capabilities{};

    /// What every successful run answers.
    std::vector<core::TranscribedSegment> segments{};

    /// The language reported when none is asked for.
    std::string detectedLanguage = "en";
};

class FakeTranscriptionEngine final : public core::TranscriptionEngine {

public:
    explicit FakeTranscriptionEngine(FakeEngineScript script) : m_script(std::move(script)) {}

    [[nodiscard]] std::string_view name() const override { return m_script.name; }

    [[nodiscard]] std::expected<core::EngineCapabilities, core::TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& /*model*/) const override {
        return m_script.capabilities;
    }

protected:
    [[nodiscard]] std::expected<core::Transcription, core::TranscriptionError>
    run(const core::TranscriptionRequest& request,
        const core::EngineCapabilities& /*capabilities*/,
        const core::TranscriptionObserver& observer) const override {
        if (!std::filesystem::exists(request.audio))
            return std::unexpected(
                core::TranscriptionError{.kind = core::TranscriptionErrorKind::AudioUnreadable,
                                         .detail = request.audio.string()});

        constexpr int kSteps = 4;
        for (int step = 1; step <= kSteps; ++step) {
            if (observer.isCancelled())
                return std::unexpected(
                    core::TranscriptionError{.kind = core::TranscriptionErrorKind::Cancelled});
            observer.report(static_cast<double>(step) / kSteps);
        }

        return core::Transcription{.segments = m_script.segments,
                                   .language =
                                       request.language.value_or(m_script.detectedLanguage)};
    }

private:
    FakeEngineScript m_script;
};

} // namespace subedit::test
