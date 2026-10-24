#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/default_engines.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/transcription/transcribe_media.hpp>
#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/transcription_runner.hpp>

#include <QCoreApplication>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>

#include <atomic>
#include <cstdlib>
#include <utility>

namespace subedit::gui {

namespace {

/// How often the box is brought up to date and the cancellation looked at.
constexpr int kPollMilliseconds = 100;

constexpr int kPercent = 100;

using Result = std::expected<TranscriptionOutcome, TranscriptionFailure>;

[[nodiscard]] TranscriptionFailure failed(std::string message) {
    return TranscriptionFailure{.message = std::move(message)};
}

/// The words for what a transcription that came to nothing said.
[[nodiscard]] TranscriptionFailure failureOf(const core::MediaTranscriptionError& error) {
    if (const auto* audio = std::get_if<core::AudioPreparationError>(&error.cause)) {
        if (audio->kind == core::AudioPreparationErrorKind::Cancelled)
            return TranscriptionFailure{.cancelled = true};
        std::string text = core::audioPreparationErrorText(*audio);
        if (audio->kind == core::AudioPreparationErrorKind::NoSuchTrack)
            text += "\n" + core::audioTracksText(error.tracks);
        return failed(std::move(text));
    }
    const auto& engine = std::get<core::TranscriptionError>(error.cause);
    if (engine.kind == core::TranscriptionErrorKind::Cancelled)
        return TranscriptionFailure{.cancelled = true};
    return failed(core::transcriptionErrorText(engine));
}

/// The whole work, on whichever thread calls it. Touches nothing of Qt.
[[nodiscard]] Result work(const TranscriptionJob& job,
                          const std::string& searchPath,
                          std::atomic<int>& percent,
                          const std::atomic<bool>& cancelled) {
    core::RealFileSystem files;
    const core::TranscriptionSettings& settings = job.settings;
    const core::EngineRegistry engines = core::makeDefaultEngines(
        files,
        searchPath,
        settings.executable.empty() ? std::nullopt : std::optional{settings.executable});
    const core::TranscriptionEngine* const engine = engines.find(settings.engine);
    if (engine == nullptr)
        return std::unexpected(failed(core::noSuchTranscriptionEngine(settings.engine)));

    core::MediaTranscriptionRequest request{.input = job.video, .audioTrack = job.track};
    if (!job.language.empty())
        request.language = job.language;
    if (!job.model.empty())
        request.model = job.model;

    const core::TranscriptionObserver observer{
        .progress =
            [&percent](double fraction) { percent = static_cast<int>(fraction * kPercent); },
        .cancelled = [&cancelled] { return cancelled.load(); }};
    auto transcribed = core::transcribeMedia(files, *engine, searchPath, request, observer);
    if (!transcribed)
        return std::unexpected(failureOf(transcribed.error()));

    return TranscriptionOutcome{
        .subtitles = core::assembleSubtitles(transcribed->segments, core::assemblyOf(settings)),
        .language = transcribed->language.value_or(std::string{})};
}

} // namespace

std::string searchPathOfProcess() {
    const char* const path = std::getenv("PATH");
    return path != nullptr ? path : "";
}

BackgroundTranscriptionRunner::BackgroundTranscriptionRunner(std::string searchPath)
    : m_searchPath(std::move(searchPath)) {}

Result BackgroundTranscriptionRunner::run(const TranscriptionJob& job, QWidget* parent) {
    std::atomic<int> percent{0};
    std::atomic<bool> cancelled{false};

    QProgressDialog box{
        translate("Transcribing the audio…"), translate("Cancel"), 0, kPercent, parent};
    box.setWindowTitle(translate("Transcribe audio"));
    box.setWindowModality(Qt::WindowModal);
    box.setMinimumDuration(0);
    box.setAutoClose(false);
    box.setAutoReset(false);
    box.setValue(0);

    QFutureWatcher<Result> watcher;
    QEventLoop loop;
    QObject::connect(&watcher, &QFutureWatcher<Result>::finished, &loop, &QEventLoop::quit);
    watcher.setFuture(QtConcurrent::run([this, &job, &percent, &cancelled] {
        return work(job, m_searchPath, percent, cancelled);
    }));

    QTimer poll;
    QObject::connect(&poll, &QTimer::timeout, &loop, [&] {
        box.setValue(percent.load());
        if (box.wasCanceled() || (m_cancelProbe && m_cancelProbe()))
            cancelled = true;
    });
    poll.start(kPollMilliseconds);

    loop.exec();
    return watcher.result();
}

} // namespace subedit::gui
