#pragma once

#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

class QWidget;

namespace subedit::gui {

/// What a transcription is asked to do for the window: the film, the track of it, and the
/// choices of the dialog. The engine, the executable and the limits are the settings.
struct TranscriptionJob {
    std::filesystem::path video;

    /// The audio track, counted from 0 among the film's.
    int track = 0;

    /// An ISO 639-1 code, or empty to detect it.
    std::string language{};

    std::filesystem::path model{};

    core::TranscriptionSettings settings{};
};

/// What it came to: the subtitles, and the language they are in.
struct TranscriptionOutcome {
    std::vector<core::Subtitle> subtitles;
    std::string language{};
};

/// Why it did not: in the user's words, or that they stopped it.
struct TranscriptionFailure {
    std::string message{};
    bool cancelled = false;
};

/// Runs a transcription for the window. **A seam**, for the reason `Prompts` is one: the
/// real thing takes minutes and a model of several hundred megabytes, which a test has
/// neither of.
class TranscriptionRunner {

public:
    virtual ~TranscriptionRunner() = default;

    /// Transcribes `job`, and returns when it is over. `parent` is what the progress box
    /// sits over.
    [[nodiscard]] virtual std::expected<TranscriptionOutcome, TranscriptionFailure>
    run(const TranscriptionJob& job, QWidget* parent) = 0;

protected:
    TranscriptionRunner() = default;
    TranscriptionRunner(const TranscriptionRunner&) = default;
    TranscriptionRunner(TranscriptionRunner&&) = default;
    TranscriptionRunner& operator=(const TranscriptionRunner&) = default;
    TranscriptionRunner& operator=(TranscriptionRunner&&) = default;
};

/// The real runner: the work on a background thread, a box that shows how far it has got
/// and lets the person stop it, and the window left alone meanwhile.
///
/// The engine, `ffmpeg` and `ffprobe` are looked for on `searchPath`, the `PATH` of the
/// process unless a test gives its own.
class BackgroundTranscriptionRunner final : public TranscriptionRunner {

public:
    explicit BackgroundTranscriptionRunner(std::string searchPath);

    /// A test's way of pressing Cancel: asked every tenth of a second, and a true stops the
    /// work as the button does.
    void setCancelProbe(std::function<bool()> probe) { m_cancelProbe = std::move(probe); }

    [[nodiscard]] std::expected<TranscriptionOutcome, TranscriptionFailure>
    run(const TranscriptionJob& job, QWidget* parent) override;

private:
    std::string m_searchPath;
    std::function<bool()> m_cancelProbe{};
};

/// The `PATH` of the process, empty when it has none.
[[nodiscard]] std::string searchPathOfProcess();

} // namespace subedit::gui
