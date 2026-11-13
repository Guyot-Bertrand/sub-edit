#include <subedit/cli/destination.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/syncing.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/transcription/align_media.hpp>
#include <subedit/core/transcription/align_on_speech.hpp>
#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/transcription/alignment_retiming.hpp>
#include <subedit/core/transcription/speech_detector.hpp>
#include <subedit/core/transcription/synchronization.hpp>
#include <subedit/core/transcription/transcript_matching_aligner.hpp>
#include <subedit/core/wording/alignment.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

/// The lines to check, as the report file and the record say them: the number of the subtitle in
/// the file, and why.
struct Flag {
    std::size_t number = 0;
    std::string kind;
    std::string words;
};

/// The identifier of a doubt that is one: a line that is sure is never flagged.
[[nodiscard]] std::string idOf(core::LineDoubt doubt) {
    if (doubt == core::LineDoubt::NotFound)
        return "not-found";
    if (doubt == core::LineDoubt::Partial)
        return "partial";
    return "out-of-line";
}

[[nodiscard]] std::vector<Flag> flagsOf(const core::AlignmentReport& report,
                                        const std::vector<core::RetimedLine>& retimed,
                                        const std::vector<std::size_t>& subtitles) {
    std::vector<Flag> flags;
    for (std::size_t line = 0; line < report.lines.size(); ++line) {
        const std::size_t number = subtitles[line] + 1;
        if (report.lines[line].doubt != core::LineDoubt::None)
            flags.push_back({.number = number,
                             .kind = idOf(report.lines[line].doubt),
                             .words = core::doubtText(report.lines[line].doubt)});
        if (retimed[line].trimmed)
            flags.push_back(
                {.number = number, .kind = "cut-back", .words = core::lineCutBackText()});
        if (retimed[line].overlapping)
            flags.push_back(
                {.number = number, .kind = "overlaps", .words = core::lineOverlapsText()});
    }
    return flags;
}

void writeReport(core::FileSystem& files,
                 const std::string& path,
                 const std::vector<Flag>& flags,
                 const Reporter& reporter) {
    std::string text;
    for (const Flag& flag : flags)
        text += std::to_string(flag.number) + '\t' + flag.words + '\n';
    core::ensureDirectory(files, std::filesystem::path{path}.parent_path());
    if (const auto written = core::writeAtomically(files, path, text); !written)
        reporter.error(path + ": " + std::string{core::reasonOfWriting(written.error().kind)});
}

[[nodiscard]] std::vector<Count> countsOf(const core::AlignmentReport& report, std::size_t moved) {
    return {{"subtitles", static_cast<std::int64_t>(report.lines.size())},
            {"placed", static_cast<std::int64_t>(report.positioned())},
            {"moved", static_cast<std::int64_t>(moved)},
            {"partial", static_cast<std::int64_t>(report.partial)},
            {"out_of_line", static_cast<std::int64_t>(report.outOfLine)},
            {"not_found", static_cast<std::int64_t>(report.notFound)},
            {"median_shift_ms", report.medianShiftMilliseconds},
            {"drift_ms", report.driftMilliseconds}};
}

[[nodiscard]] Json flagsRecord(const std::vector<Flag>& flags) {
    Json list = Json::array();
    for (const Flag& flag : flags) {
        Json one = Json::object();
        one.set("subtitle", flag.number);
        one.set("kind", flag.kind);
        list.push(std::move(one));
    }
    return list;
}

/// What to listen to and how, from the options over the settings; or why it cannot be asked.
[[nodiscard]] std::optional<ExitCode>
prepareListening(core::FileSystem& files,
                 const core::TranscriptionEngine& listener,
                 const std::string& engineName,
                 const SyncOptions& options,
                 const core::TranscriptionSettings& transcription,
                 core::MediaAlignmentRequest& request,
                 const Reporter& reporter) {
    request.media.input = options.video.empty() ? options.audio : options.video;
    request.media.audioTrack = options.audioTrack;
    request.media.keepAudio = options.keepAudio;
    // Notes are not words: the engine is told not to write them, whatever the transcription
    // setting.
    request.media.suppressNotes = true;
    const std::string& languageName =
        options.language.empty() ? transcription.language : options.language;
    const std::filesystem::path& modelPath =
        options.model.empty() ? transcription.model : std::filesystem::path{options.model};
    if (!modelPath.empty())
        request.media.model = modelPath;
    const std::filesystem::path& voiceModel =
        options.vadModel.empty() ? transcription.vadModel : std::filesystem::path{options.vadModel};
    if (!voiceModel.empty())
        request.media.vadModel = voiceModel;
    if (const std::optional<ExitCode> refused =
            readListenerChoices(languageName, options.start, options.end, request.media, reporter))
        return refused;

    const auto capabilities = listener.capabilitiesFor(request.media.model);
    if (!options.keepAudio.empty())
        request.media.keepAudio =
            core::expandAudioName(options.keepAudio, capabilities ? capabilities->sampleRate : 0);
    if (const std::optional<ExitCode> refused =
            checkVoiceModel(options.vadModel,
                            capabilities ? &*capabilities : nullptr,
                            engineName,
                            files,
                            request.media,
                            reporter))
        return refused;
    return std::nullopt;
}

/// Applies the plan to the session, writes the report of the lines to check, and says what it
/// came to: **the one end both methods share.**
[[nodiscard]] OperationResult conclude(core::FileSystem& files,
                                       const SyncOptions& options,
                                       const Reporter& reporter,
                                       core::Session& session,
                                       const core::SynchronizationPlan& plan) {
    const std::size_t moved = plan.changed();
    if (std::unique_ptr<core::Command> command =
            core::synchronizationCommand(session.project(), plan))
        session.apply(std::move(command));

    const std::vector<Flag> flags = flagsOf(plan.report, plan.retimed, plan.subtitles);
    if (!options.report.empty())
        writeReport(files, options.report, flags, reporter);

    OperationResult result{.sentence = core::noticeOfAlignment(plan.report),
                           .counts = countsOf(plan.report, moved)};
    result.fields.emplace_back("flagged", flagsRecord(flags));
    return result;
}

/// Everything one file of the batch needs to be re-timed.
struct SyncRun {
    core::FileSystem& files;
    const core::TranscriptMatchingAligner& aligner;
    std::string_view searchPath;
    core::MediaAlignmentRequest& request;
    const std::function<bool()>& cancelled;
    const core::DoubtLimits& limits;
    const SyncOptions& options;
};

/// The progress of the work on one element, said under its position, and the `media` line before
/// it.
template<typename Work>
[[nodiscard]] auto listening(const Reporter& item,
                             const std::string& media,
                             const std::function<bool()>& cancelled,
                             Work work) {
    item.tell(1, Key::Media, media);
    ProgressNarration narration{item};
    const core::TranscriptionObserver observer{
        .advanced =
            [&narration](const core::TranscriptionProgress& progress) { narration(progress); },
        .cancelled = cancelled,
        .waiting = [&narration] { narration.idle(); }};
    return work(observer);
}

[[nodiscard]] OperationOutcome
syncOneFile(const SyncRun& run, core::Session& session, const Reporter& item) {
    const core::LinesToSynchronize looked = core::subtitlesBeginningIn(
        session.project(), run.request.media.start, run.request.media.end);
    run.request.lines = looked.lines;
    const auto matched = listening(
        item,
        run.request.media.input,
        run.cancelled,
        [&](const core::TranscriptionObserver& observer) {
            return core::alignMedia(run.files, run.aligner, run.searchPath, run.request, observer);
        });
    if (!matched)
        return std::unexpected(failureOf(matched.error()));

    const core::SynchronizationPlan plan =
        core::planSynchronization(session.project(), looked.subtitles, *matched, run.limits);
    if (plan.report.positioned() == 0 && !looked.lines.empty())
        return std::unexpected(Failure{"no-line-found", [&] {
                                           return core::noLineFoundInTheAudio(looked.lines.size()) +
                                                  " " + core::speechMethodIgnoresLanguage();
                                       }});

    return conclude(run.files, run.options, item, session, plan);
}

/// Everything one file of the batch needs to be laid on the speech.
struct SpeechRun {
    core::FileSystem& files;
    const core::SpeechDetector& detector;
    std::string_view searchPath;
    core::SpeechAlignmentRequest& request;
    const std::function<bool()>& cancelled;
    const core::DoubtLimits& limits;
    const SyncOptions& options;
};

[[nodiscard]] OperationOutcome
speechOneFile(const SpeechRun& run, core::Session& session, const Reporter& item) {
    const core::Project& project = session.project();
    const core::LinesToSynchronize looked =
        core::subtitlesBeginningIn(project, run.request.media.start, run.request.media.end);
    run.request.lines.clear();
    for (const std::size_t at : looked.subtitles) {
        const core::Subtitle& subtitle = project.subtitleAt(core::SubtitleIndex::fromValue(at));
        run.request.lines.push_back({.start = subtitle.start, .end = subtitle.end});
    }
    const auto aligned =
        listening(item,
                  run.request.media.input,
                  run.cancelled,
                  [&](const core::TranscriptionObserver& observer) {
                      return core::alignMediaOnSpeech(
                          run.files, run.detector, run.searchPath, run.request, observer);
                  });
    if (!aligned)
        return std::unexpected(failureOf(aligned.error()));
    if (!aligned->correction.fitted() && !looked.lines.empty())
        return std::unexpected(
            Failure{"no-agreement", [] { return core::noAgreementWithTheSpeech(); }});

    const core::SynchronizationPlan plan =
        core::planSynchronization(project, looked.subtitles, aligned->matches, run.limits);
    OperationResult result = conclude(run.files, run.options, item, session, plan);
    if (aligned->correction.ratio != 1.0)
        result.sentence += " " + core::frameRateCorrectedText(aligned->correction.ratio);
    // Said every time: what this method does not do is what a person would otherwise expect of it.
    result.warnings.push_back(
        {.kind = "speech-only",
         .detail = core::speechOnlyWarning(),
         .english = core::inSourceLanguage([] { return core::speechOnlyWarning(); })});
    return result;
}

/// Re-times the files by matching their words to what the engine hears.
ExitCode syncOnWords(core::FileSystem& files,
                     const core::EngineRegistry& engines,
                     std::string_view searchPath,
                     const std::vector<std::string>& paths,
                     const std::optional<core::Encoding>& reading,
                     const SyncOptions& options,
                     const core::TranscriptionSettings& transcription,
                     const core::AlignmentSettings& alignment,
                     const Destination& destination,
                     const Reporter& reporter,
                     const std::function<bool()>& cancelled) {
    const std::string engineName = options.engine.empty() ? transcription.engine : options.engine;
    const core::TranscriptionEngine* const listener = engines.find(engineName);
    if (listener == nullptr) {
        reporter.error(core::noSuchTranscriptionEngine(engineName));
        return ExitCode::Usage;
    }
    if (core::makeAlignmentEngine(alignment.engine, *listener) == nullptr) {
        reporter.error(core::noSuchAlignmentEngine(alignment.engine));
        return ExitCode::Usage;
    }

    core::MediaAlignmentRequest request;
    if (const std::optional<ExitCode> refused = prepareListening(
            files, *listener, engineName, options, transcription, request, reporter))
        return *refused;

    const int percent = options.minimumMatchPercent.value_or(alignment.minimumMatchPercent);
    const core::TranscriptMatchingAligner aligner{*listener,
                                                  static_cast<double>(percent) / core::kPercent};
    const core::DoubtLimits limits{
        .partialBelow = core::kDefaultPartialBelow,
        .gapMilliseconds = options.doubtGapMilliseconds.value_or(alignment.doubtGapMilliseconds)};

    const SyncRun run{.files = files,
                      .aligner = aligner,
                      .searchPath = searchPath,
                      .request = request,
                      .cancelled = cancelled,
                      .limits = limits,
                      .options = options};
    const ChangingOperation sync = [&run](core::Session& session, const Request& asked) {
        return syncOneFile(run, session, *asked.reporter);
    };

    return rewriteAll(files, paths, reading, destination, reporter, Verb::Retimed, sync);
}

/// Re-times the files by laying them on where there is speech in the audio.
ExitCode syncOnSpeech(core::FileSystem& files,
                      std::string_view searchPath,
                      const std::vector<std::string>& paths,
                      const std::optional<core::Encoding>& reading,
                      const SyncOptions& options,
                      const core::TranscriptionSettings& transcription,
                      const core::AlignmentSettings& alignment,
                      const Destination& destination,
                      const Reporter& reporter,
                      const std::function<bool()>& cancelled) {
    const std::filesystem::path& voiceModel =
        options.vadModel.empty() ? transcription.vadModel : std::filesystem::path{options.vadModel};
    if (voiceModel.empty()) {
        reporter.error(withPointerApart(core::speechNeedsTheVoiceModel()));
        return ExitCode::AllFailed;
    }
    if (!files.exists(voiceModel)) {
        reporter.error(core::voiceModelNotFound(voiceModel.string()));
        return ExitCode::AllFailed;
    }
    const std::optional<std::filesystem::path> tool = core::findVadTool(files, searchPath);
    if (!tool) {
        reporter.error(withPointerApart(core::speechNeedsTheVoiceTool()));
        return ExitCode::AllFailed;
    }
    const core::WhisperVadDetector detector{*tool};

    core::SpeechAlignmentRequest request;
    request.media.input = options.video.empty() ? options.audio : options.video;
    request.media.audioTrack = options.audioTrack;
    request.media.vadModel = voiceModel;
    if (const std::optional<ExitCode> refused =
            readListenerChoices({}, options.start, options.end, request.media, reporter))
        return *refused;
    if (!options.keepAudio.empty())
        request.media.keepAudio = core::expandAudioName(options.keepAudio, core::kSpeechSampleRate);

    const core::DoubtLimits limits{
        .partialBelow = core::kDefaultPartialBelow,
        .gapMilliseconds = options.doubtGapMilliseconds.value_or(alignment.doubtGapMilliseconds)};
    const SpeechRun run{.files = files,
                        .detector = detector,
                        .searchPath = searchPath,
                        .request = request,
                        .cancelled = cancelled,
                        .limits = limits,
                        .options = options};
    const ChangingOperation sync = [&run](core::Session& session, const Request& asked) {
        return speechOneFile(run, session, *asked.reporter);
    };
    return rewriteAll(files, paths, reading, destination, reporter, Verb::Retimed, sync);
}

} // namespace

ExitCode syncAll(core::FileSystem& files,
                 const core::EngineRegistry& engines,
                 std::string_view searchPath,
                 const std::vector<std::string>& paths,
                 const std::optional<core::Encoding>& reading,
                 const SyncOptions& options,
                 const core::TranscriptionSettings& transcription,
                 const core::AlignmentSettings& alignment,
                 const Destination& destination,
                 const Reporter& reporter,
                 const std::function<bool()>& cancelled) {
    if (options.video.empty() == options.audio.empty()) {
        reporter.error(core::needsOneAudioSource());
        return ExitCode::Usage;
    }
    if (!options.report.empty() && paths.size() > 1) {
        reporter.error(std::string{
            core::translate("--report names one file: give one subtitle file when you use it")});
        return ExitCode::Usage;
    }

    const core::AlignmentMethod method =
        core::alignmentMethodNamed(options.method).value_or(alignment.method);
    if (method == core::AlignmentMethod::Speech)
        return syncOnSpeech(files,
                            searchPath,
                            paths,
                            reading,
                            options,
                            transcription,
                            alignment,
                            destination,
                            reporter,
                            cancelled);
    return syncOnWords(files,
                       engines,
                       searchPath,
                       paths,
                       reading,
                       options,
                       transcription,
                       alignment,
                       destination,
                       reporter,
                       cancelled);
}

} // namespace subedit::cli
