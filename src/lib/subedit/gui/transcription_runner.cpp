#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/transcription/annotation_notes.hpp>
#include <subedit/core/transcription/default_engines.hpp>
#include <subedit/core/transcription/remaining_time.hpp>
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
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <utility>

namespace subedit::gui {

namespace {

/// How often the box is brought up to date and the cancellation looked at.
constexpr int kPollMilliseconds = 100;

constexpr int kPercent = 100;

/// Wide enough for the title of the window and the longest line of the label.
constexpr int kBoxWidth = 520;

using Result = std::expected<TranscriptionOutcome, TranscriptionFailure>;

/// The longest a text is shown in the box, which would otherwise widen to hold it.
constexpr qsizetype kLastTextWidth = 80;

/// Where the worker leaves how far it is, and where the box picks it up: the worker writes at
/// the pace of the engine, the box reads every tenth of a second.
class SharedProgress {
public:
    void set(const core::TranscriptionProgress& progress) {
        const std::scoped_lock lock{m_mutex};
        m_progress = progress;
    }

    [[nodiscard]] core::TranscriptionProgress get() const {
        const std::scoped_lock lock{m_mutex};
        return m_progress;
    }

private:
    mutable std::mutex m_mutex;
    core::TranscriptionProgress m_progress{.stage = core::TranscriptionStage::Extracting};
};

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
                          SharedProgress& progress,
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
    request.start = job.start;
    request.end = job.end;

    const core::TranscriptionObserver observer{
        .advanced = [&progress](const core::TranscriptionProgress& state) { progress.set(state); },
        .cancelled = [&cancelled] { return cancelled.load(); }};
    auto transcribed = core::transcribeMedia(files, *engine, searchPath, request, observer);
    if (!transcribed)
        return std::unexpected(failureOf(transcribed.error()));

    const core::NotesShare share = core::notesShare(transcribed->segments);
    return TranscriptionOutcome{
        .subtitles = core::assembleSubtitles(transcribed->segments, core::assemblyOf(settings)),
        .language = transcribed->language.value_or(std::string{}),
        .notes = share.notes,
        .pieces = share.pieces};
}

} // namespace

QString transcriptionBoxLabel(const core::TranscriptionProgress& progress,
                              std::optional<std::chrono::seconds> left,
                              std::optional<std::chrono::seconds> elapsed) {
    const QString spent =
        elapsed
            ? QStringLiteral("\n%1").arg(QString::fromStdString(core::elapsedTimeText(*elapsed)))
            : QString{};
    if (progress.stage == core::TranscriptionStage::Extracting)
        return translate("Extracting the audio…") + spent;

    // The time left and the time spent, then where the engine is in the film, **just above the last
    // text**: the position and the text it stands for read together.
    QString label = translate("Transcribing the audio…");
    if (left)
        label += QStringLiteral("\n%1").arg(QString::fromStdString(core::remainingTimeText(*left)));
    label += spent;
    if (progress.position && progress.total)
        label += QStringLiteral("\n%1").arg(QString::fromStdString(core::substitute(
            core::translate("%1 of %2"),
            {core::clockText(*progress.position), core::clockText(*progress.total)})));
    if (!progress.lastText.empty())
        label += QStringLiteral("\n%1").arg(
            substitute(translate("Last text: %1"),
                       QString::fromStdString(progress.lastText).left(kLastTextWidth)));
    return label;
}

std::string searchPathOfProcess() {
    const char* const path = std::getenv("PATH");
    return path != nullptr ? path : "";
}

BackgroundTranscriptionRunner::BackgroundTranscriptionRunner(std::string searchPath)
    : m_searchPath(std::move(searchPath)) {}

core::TranscriptionTools
BackgroundTranscriptionRunner::tools(const core::TranscriptionSettings& settings) {
    const core::RealFileSystem files;
    return core::inspectTranscriptionTools(files, m_searchPath, settings);
}

Result BackgroundTranscriptionRunner::run(const TranscriptionJob& job, QWidget* parent) {
    SharedProgress progress;
    core::RemainingTime remaining;
    const auto started = core::RemainingTime::Clock::now();
    remaining.note(0.0, started);
    std::atomic<bool> cancelled{false};

    QProgressDialog box{
        translate("Transcribing the audio…"), translate("Cancel"), 0, kPercent, parent};
    box.setWindowTitle(translate("Transcribe audio"));
    box.setWindowModality(Qt::WindowModal);
    box.setMinimumDuration(0);
    box.setAutoClose(false);
    box.setAutoReset(false);
    // Busy until something says how far it is: an extraction has no figure to give.
    box.setRange(0, 0);
    // A box with no value yet is shown after a delay Qt sets by itself, a few seconds: asked for at
    // once, since the work starts at once and a window that does not answer looks like a freeze.
    box.setMinimumWidth(kBoxWidth);
    box.show();

    QFutureWatcher<Result> watcher;
    QEventLoop loop;
    QObject::connect(&watcher, &QFutureWatcher<Result>::finished, &loop, &QEventLoop::quit);
    watcher.setFuture(QtConcurrent::run([this, &job, &progress, &cancelled] {
        return work(job, m_searchPath, progress, cancelled);
    }));

    QTimer poll;
    QObject::connect(&poll, &QTimer::timeout, &loop, [&] {
        const core::TranscriptionProgress state = progress.get();
        const auto now = core::RemainingTime::Clock::now();
        if (state.fraction)
            remaining.note(*state.fraction, now);
        if (state.fraction && state.stage == core::TranscriptionStage::Transcribing) {
            box.setRange(0, kPercent);
            box.setValue(static_cast<int>(*state.fraction * kPercent));
        }
        box.setLabelText(
            transcriptionBoxLabel(state,
                                  remaining.left(now),
                                  std::chrono::duration_cast<std::chrono::seconds>(now - started)));
        if (box.wasCanceled() || (m_cancelProbe && m_cancelProbe()))
            cancelled = true;
    });
    poll.start(kPollMilliseconds);

    loop.exec();
    return watcher.result();
}

} // namespace subedit::gui
