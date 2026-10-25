#pragma once

#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/tools_status.hpp>
#include <subedit/core/transcription/transcript.hpp>
#include <subedit/core/transcription/transcription_progress.hpp>

#include <QString>

#include <chrono>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
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

    /// The part of the film to transcribe; all of it when not given.
    std::optional<core::Timestamp> start{};
    std::optional<core::Timestamp> end{};
};

/// What it came to: **the pieces the engine cut, as it cut them**, and the language they are in.
/// The post-processing is not done here: the window does it, with the limits it holds, so that the
/// preview can show both and the person can judge the settings.
struct TranscriptionOutcome {
    std::vector<core::TranscribedSegment> segments;
    std::string language{};
};

/// Why it did not: in the user's words, or that they stopped it.
struct TranscriptionFailure {
    std::string message{};
    bool cancelled = false;

    /// What the engine had obtained when it stopped, for the preview to offer: empty when nothing.
    std::vector<core::TranscribedSegment> partial{};
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

    /// Which of what a transcription needs is there, for the settings given. Asked each time the
    /// window shows it: the person installs a tool, comes back, and expects the window to know.
    [[nodiscard]] virtual core::TranscriptionTools
    tools(const core::TranscriptionSettings& settings) = 0;

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

    [[nodiscard]] core::TranscriptionTools
    tools(const core::TranscriptionSettings& settings) override;

private:
    std::string m_searchPath;
    std::function<bool()> m_cancelProbe{};
};

/// What the progress box says of where the work stands: the step, the position in the film,
/// the time left, the time spent and the last text obtained, each when it is known.
[[nodiscard]] QString
transcriptionBoxLabel(const core::TranscriptionProgress& progress,
                      std::optional<std::chrono::seconds> left,
                      std::optional<std::chrono::seconds> elapsed = std::nullopt);

/// The `PATH` of the process, empty when it has none.
[[nodiscard]] std::string searchPathOfProcess();

} // namespace subedit::gui
