#pragma once

#include <subedit/core/transcription/engine_capabilities.hpp>
#include <subedit/core/transcription/transcript.hpp>
#include <subedit/core/transcription/transcription_error.hpp>
#include <subedit/core/transcription/transcription_progress.hpp>

#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// What a transcription is asked to do.
struct TranscriptionRequest {
    /// The normalised audio file of phase 22, D5 — never a video.
    std::filesystem::path audio;

    /// The ISO 639-1 code to transcribe, or nothing to let the engine detect it.
    std::optional<std::string> language{};

    /// The model, for an engine that needs one.
    std::optional<std::filesystem::path> model{};

    /// Whether the engine is to avoid writing sounds as notes (`(phone ringing)`, `[MUSIC
    /// PLAYING]`). Honoured by an engine that declares `EngineCapabilities::suppressNotes`, ignored
    /// by one that does not.
    bool suppressNotes = true;

    /// The part of the audio to transcribe, in the time of the film; the whole of it
    /// when nothing is given. The segments come back in that same time, and none
    /// begins before `start` or after `end`.
    std::optional<Timestamp> start{};
    std::optional<Timestamp> end{};
};

/// How a run is watched and stopped.
///
/// Both are optional, and both are called from the thread that runs the
/// transcription. A progress of 0 to 1 is reported as the engine can tell,
/// coarsely for an engine that works by windows; **cancellation is polled**,
/// at the engine's own pace, and a cancelled run returns
/// `TranscriptionErrorKind::Cancelled`.
struct TranscriptionObserver {
    std::function<void(double fraction)> progress{};

    /// Called each time something is learnt: the step changes, a piece of speech is
    /// obtained, the engine reports a window done.
    std::function<void(const TranscriptionProgress&)> advanced{};
    std::function<bool()> cancelled{};

    /// Called each time the work is waited on, many times a second whether or not anything
    /// was learnt: a surface that wants to say something at its own pace — a line every ten
    /// seconds — has no other clock to hang it on.
    std::function<void()> waiting{};

    void wait() const {
        if (waiting)
            waiting();
    }

    void report(double fraction) const {
        if (progress)
            progress(fraction);
    }

    /// Reports `state`, and its fraction too when it has one.
    void advance(const TranscriptionProgress& state) const {
        if (advanced)
            advanced(state);
        if (state.fraction)
            report(*state.fraction);
    }

    [[nodiscard]] bool isCancelled() const { return cancelled && cancelled(); }
};

/// The port: audio in, segments out.
///
/// One of the points where this project knows the variation is real — phase 22
/// is built so that another engine can replace whisper.cpp by writing an
/// adapter and passing the contract suite. It knows nothing of whisper, of a
/// command line, of JSON or of Qt (ADR 0046).
///
/// **`transcribe` is not virtual.** It asks the engine what it can do with the
/// model, refuses a request the answer rules out *before any work starts*, and
/// only then calls `run`. An adapter therefore cannot forget the refusal, and a
/// transcription in the wrong language never comes back looking like a success.
class TranscriptionEngine {

public:
    TranscriptionEngine(const TranscriptionEngine&) = delete;
    TranscriptionEngine(TranscriptionEngine&&) = delete;
    TranscriptionEngine& operator=(const TranscriptionEngine&) = delete;
    TranscriptionEngine& operator=(TranscriptionEngine&&) = delete;
    virtual ~TranscriptionEngine() = default;

    /// The name the setting uses to choose it (`whisper.cpp`).
    [[nodiscard]] virtual std::string_view name() const = 0;

    /// What the engine can do with `model`, or why it cannot say — typically a
    /// program that is not there or a model that is not readable.
    [[nodiscard]] virtual std::expected<EngineCapabilities, TranscriptionError>
    capabilitiesFor(const std::optional<std::filesystem::path>& model) const = 0;

    [[nodiscard]] std::expected<Transcription, TranscriptionError>
    transcribe(const TranscriptionRequest& request,
               const TranscriptionObserver& observer = {}) const;

protected:
    TranscriptionEngine() = default;

    /// Does the work, for a request `transcribe` has already checked against
    /// `capabilities`.
    [[nodiscard]] virtual std::expected<Transcription, TranscriptionError>
    run(const TranscriptionRequest& request,
        const EngineCapabilities& capabilities,
        const TranscriptionObserver& observer) const = 0;
};

/// Whether `request` can be honoured by an engine that declares `capabilities`.
///
/// Exposed on its own because the surfaces ask the same question before they
/// start anything: the window greys what the pair cannot do, the command line
/// refuses with the same reason.
[[nodiscard]] std::expected<void, TranscriptionError>
checkRequest(const EngineCapabilities& capabilities, const TranscriptionRequest& request);

} // namespace subedit::core
