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
#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/transcription/alignment_retiming.hpp>
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
        reporter.failed(path + ": " + std::string{core::reasonOfWriting(written.error().kind)});
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

/// Everything one file of the batch needs to be re-timed.
struct SyncRun {
    core::FileSystem& files;
    const core::TranscriptMatchingAligner& aligner;
    std::string_view searchPath;
    core::MediaAlignmentRequest& request;
    const core::TranscriptionObserver& observer;
    const core::DoubtLimits& limits;
    const SyncOptions& options;
    const Reporter& reporter;
};

[[nodiscard]] OperationOutcome syncOneFile(const SyncRun& run, core::Session& session) {
    const core::LinesToSynchronize looked = core::subtitlesBeginningIn(
        session.project(), run.request.media.start, run.request.media.end);
    run.request.lines = looked.lines;
    const auto matched =
        core::alignMedia(run.files, run.aligner, run.searchPath, run.request, run.observer);
    if (!matched)
        return std::unexpected(failureOf(matched.error()));

    const core::SynchronizationPlan plan =
        core::planSynchronization(session.project(), looked.subtitles, *matched, run.limits);
    if (plan.report.positioned() == 0 && !looked.lines.empty())
        return std::unexpected(Failure{
            "no-line-found", [&] { return core::noLineFoundInTheAudio(looked.lines.size()); }});

    const std::size_t moved = plan.changed();
    if (std::unique_ptr<core::Command> command =
            core::synchronizationCommand(session.project(), plan))
        session.apply(std::move(command));

    const std::vector<Flag> flags = flagsOf(plan.report, plan.retimed, plan.subtitles);
    if (!run.options.report.empty())
        writeReport(run.files, run.options.report, flags, run.reporter);

    OperationResult result{.sentence = core::noticeOfAlignment(plan.report),
                           .counts = countsOf(plan.report, moved)};
    result.fields.emplace_back("flagged", flagsRecord(flags));
    return result;
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
        reporter.failed(core::needsOneAudioSource());
        return ExitCode::Usage;
    }
    if (!options.report.empty() && paths.size() > 1) {
        reporter.failed(std::string{
            core::translate("--report names one file: give one subtitle file when you use it")});
        return ExitCode::Usage;
    }

    const std::string engineName = options.engine.empty() ? transcription.engine : options.engine;
    const core::TranscriptionEngine* const listener = engines.find(engineName);
    if (listener == nullptr) {
        reporter.failed(core::noSuchTranscriptionEngine(engineName));
        return ExitCode::Usage;
    }
    if (core::makeAlignmentEngine(alignment.engine, *listener) == nullptr) {
        reporter.failed(core::noSuchAlignmentEngine(alignment.engine));
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

    ProgressNarration narration{reporter};
    const core::TranscriptionObserver observer{
        .advanced =
            [&narration](const core::TranscriptionProgress& progress) { narration(progress); },
        .cancelled = cancelled,
        .waiting = [&narration] { narration.idle(); }};

    const SyncRun run{.files = files,
                      .aligner = aligner,
                      .searchPath = searchPath,
                      .request = request,
                      .observer = observer,
                      .limits = limits,
                      .options = options,
                      .reporter = reporter};
    const Operation sync = [&run](core::Session& session) { return syncOneFile(run, session); };

    return rewriteAll(files, paths, reading, destination, reporter, Verb::Retimed, sync);
}

} // namespace subedit::cli
