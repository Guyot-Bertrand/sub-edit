#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/command_preview.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/disjoint_ranges.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/join_repeats.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/replace_subtitles.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/edit/sort_command.hpp>
#include <subedit/core/edit/split_project.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/transcription/synchronization.hpp>
#include <subedit/core/wording/alignment.hpp>
#include <subedit/core/wording/conversion.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/editing.hpp>
#include <subedit/core/wording/transcription.hpp>
#include <subedit/gui/disjoint_dialog.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/hearing_impaired_dialog.hpp>
#include <subedit/gui/project_operations.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/snap_dialog.hpp>
#include <subedit/gui/split_project_dialog.hpp>
#include <subedit/gui/status_line.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/synchronization_preview.hpp>
#include <subedit/gui/synchronization_preview_dialog.hpp>
#include <subedit/gui/target.hpp>
#include <subedit/gui/transcription_preview_dialog.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QObject>
#include <QString>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// What an operation of `page` applies to: the rows selected in it, or the
/// whole file when none is.
[[nodiscard]] core::Selection targetIn(const ProjectPage& page) {
    return targetOf(*page.tableSelection, page.session->project());
}

/// What an operation of `page` could apply to, for the dialog to offer the choice.
[[nodiscard]] OperationScope scopeOf(const ProjectPage& page) {
    return OperationScope{page.session->project().count(),
                          selectionOf(*page.tableSelection).count()};
}

/// What `dialog` says to apply to: every subtitle, or the rows selected.
///
/// Read once the dialog is accepted — or at each preview — since the choice is
/// the user's and may have changed since the dialog opened.
[[nodiscard]] core::Selection chosenIn(const ProjectPage& page, const OperationDialog& dialog) {
    return dialog.wholeProject() ? core::Selection::all(page.session->project())
                                 : selectionOf(*page.tableSelection);
}

/// How many changes a preview lists: a glance, and the rest is counted.
constexpr std::size_t kPreviewRows = 12;

/// Builds the command a dialog is about to ask for, over a target — or nothing
/// while what was typed makes none.
using CommandMaker = std::function<std::unique_ptr<core::Command>(const core::Selection& target)>;

/// Gives `dialog` its preview: what `make` would do, shown before it is done.
void offerPreviewOf(OperationDialog& dialog, const ProjectPage& page, CommandMaker make) {
    dialog.offerPreview([&dialog, &page, make = std::move(make)] {
        std::unique_ptr<core::Command> command = make(chosenIn(page, dialog));
        if (!command)
            return OperationPreview{};
        return describedPreview(core::previewOf(page.session->project(), *command, kPreviewRows));
    });
}

/// The transform two typed references define, over `target`.
///
/// What the dialog read becomes the core's own value here: it holds widgets,
/// not the vocabulary of a command.
[[nodiscard]] std::optional<core::TransformCommand> transformOf(const core::Project& project,
                                                                const core::Selection& target,
                                                                const TypedReference& first,
                                                                const TypedReference& second) {
    const auto referenceOf = [](const TypedReference& typed) {
        return core::TransformReference{
            .index = core::SubtitleIndex::fromNumber(static_cast<std::size_t>(typed.number)),
            .target = typed.target,
        };
    };
    return core::TransformCommand::create(project, target, referenceOf(first), referenceOf(second));
}

/// Why a shift of `target` by `by` is refused, or nothing.
///
/// A position before the origin is representable, but no subtitle file can
/// hold one. The rule has lived in the core since #132, shared with the
/// command line.
[[nodiscard]] std::optional<std::string>
refusalOfShift(const core::Project& project, const core::Selection& target, core::Duration by) {
    const std::optional<core::SubtitleIndex> refused = core::firstBeforeOrigin(project, target, by);
    if (!refused.has_value())
        return std::nullopt;
    return core::shiftBeforeTheOrigin(refused->number());
}

/// Shows the bar of the window for as long as it lives.
class BusyScope final {

public:
    explicit BusyScope(ProjectOperations::View& view) : m_view(&view) { m_view->setBusy(true); }

    ~BusyScope() { m_view->setBusy(false); }

    BusyScope(const BusyScope&) = delete;
    BusyScope& operator=(const BusyScope&) = delete;
    BusyScope(BusyScope&&) = delete;
    BusyScope& operator=(BusyScope&&) = delete;

private:
    ProjectOperations::View* m_view;
};

/// The settings of the core a form of the window spells out.
[[nodiscard]] core::DisjointSettings settingsOf(const DisjointChoice& choice) {
    return core::DisjointSettings{.shortest = core::Duration::fromMilliseconds(choice.shortestMs),
                                  .mostLines = static_cast<std::size_t>(choice.mostLines),
                                  .dashes = choice.dashes};
}

/// Whether the account of an operation says something the user should stop and
/// read: what could not be done in full.
[[nodiscard]] bool hasCaveat(const core::DisjointReport& report) {
    return report.windowed > 0 || report.absorbed > 0 || report.tooShort > 0 ||
           report.extrasDropped > 0 || report.endingBeforeStart > 0;
}

/// Joins the repeated subtitles of `page`, saying nothing: the callers word the
/// account.
[[nodiscard]] core::JoinReport joinedQuietly(ProjectPage& page) {
    core::JoinedRepeats joined = core::joinRepeats(page.session->project());
    if (joined.command != nullptr) {
        const std::vector<core::Change> changes = page.session->apply(std::move(joined.command));
        page.model->applied(changes);
        page.placedAt = -1;
    }
    return joined.report;
}

/// Puts the project of `page` in order and returns how many subtitles moved,
/// saying nothing: the callers word the account.
[[nodiscard]] std::size_t sortedQuietly(ProjectPage& page) {
    if (page.session->project().isInOrder())
        return 0;

    const std::vector<core::Change> changes =
        page.session->apply(std::make_unique<core::SortCommand>());
    page.model->applied(changes);
    // Playback was placed on a row that holds another subtitle now.
    page.placedAt = -1;

    std::size_t moved = 0;
    for (const core::Change& change : changes)
        moved += change.subtitles.count();
    return moved;
}

} // namespace

std::string joinedNotices(const std::string& done, const std::string& warning) {
    if (done.empty())
        return warning;
    return warning.empty() ? done : done + "\n" + warning;
}

ProjectOperations::ProjectOperations(Prompts& prompts, View& view)
    : m_prompts(&prompts), m_view(&view) {}

void ProjectOperations::apply(ProjectPage& page,
                              std::unique_ptr<core::Command> command,
                              const core::Selection& target) {
    // A notice and not a failure: nothing was prevented, and the sentence is
    // written to be read after the fact.
    if (const std::string notice = applyQuietly(page, std::move(command), target); !notice.empty())
        m_prompts->reportOutcome(notice);
}

std::string ProjectOperations::applyQuietly(ProjectPage& page,
                                            std::unique_ptr<core::Command> command,
                                            const core::Selection& target) {
    // Read before the command goes: what it is, is what the notice names.
    const core::CommandKind kind = command->kind();

    page.model->applied(page.session->apply(std::move(command)));

    // The row playback was placed at holds something else now — a shift moved
    // it, a removal may have taken it away. Forgetting it is what lets a click
    // on that same row send playback where the subtitle has gone.
    page.placedAt = -1;

    return whatPassesTheEnd(page, kind, target);
}

std::string ProjectOperations::whatPassesTheEnd(const ProjectPage& page,
                                                core::CommandKind kind,
                                                const core::Selection& target) const {
    // **Only the operations that move a position.** `beyondEnd` reads the state
    // an operation produced; on its own it cannot tell whether that operation
    // put anything there. A subtitle already past the end because the film is
    // the wrong one is nobody's doing, least of all that of a removal of
    // hearing-impaired mentions.
    if (!core::movesPositions(kind))
        return {};

    const std::optional<core::BeyondEnd> beyond =
        core::beyondEnd(page.session->project(), target, m_view->videoLength(page));
    return beyond.has_value() ? core::noticeOf(kind, *beyond) : std::string{};
}

void ProjectOperations::shift(ProjectPage& page) {
    ShiftDialog dialog{scopeOf(page), m_view->dialogParent()};
    offerPreviewOf(dialog, page, [&dialog](const core::Selection& target) {
        const std::optional<core::Duration> by = dialog.shift();
        return by.has_value() ? std::make_unique<core::ShiftCommand>(target, *by)
                              : std::unique_ptr<core::ShiftCommand>{};
    });
    if (!m_prompts->run(dialog))
        return;
    const core::Selection target = chosenIn(page, dialog);

    const std::optional<core::Duration> by = dialog.shift();
    if (!by.has_value())
        return;

    if (const std::optional<std::string> refusal =
            refusalOfShift(page.session->project(), target, *by);
        refusal.has_value()) {
        m_prompts->reportFailure(*refusal);
        return;
    }

    apply(page, std::make_unique<core::ShiftCommand>(target, *by), target);
}

void ProjectOperations::transform(ProjectPage& page) {
    const core::Project& project = page.session->project();

    // What a number stands for, said where the user types it: the start the
    // subtitle has now, and its text — the two things they would otherwise
    // look up in the table.
    AnchorLookup lookup = [&project](int number) { return anchorIn(project, number); };

    TransformDialog dialog{scopeOf(page), std::move(lookup), m_view->dialogParent()};
    offerPreviewOf(dialog, page, [&dialog, &project](const core::Selection& target) {
        const std::optional<TypedReference> first = dialog.first();
        const std::optional<TypedReference> second = dialog.second();
        std::optional<core::TransformCommand> command =
            first.has_value() && second.has_value() ? transformOf(project, target, *first, *second)
                                                    : std::nullopt;
        return command.has_value() ? std::make_unique<core::TransformCommand>(std::move(*command))
                                   : std::unique_ptr<core::TransformCommand>{};
    });
    if (!m_prompts->run(dialog))
        return;

    const std::optional<TypedReference> first = dialog.first();
    const std::optional<TypedReference> second = dialog.second();
    if (!first.has_value() || !second.has_value())
        return;

    const core::Selection target = chosenIn(page, dialog);
    std::optional<core::TransformCommand> command = transformOf(project, target, *first, *second);
    if (!command.has_value()) {
        m_prompts->reportFailure(
            std::string{core::translate("the two references define no correction")});
        return;
    }

    apply(page, std::make_unique<core::TransformCommand>(std::move(*command)), target);
}

void ProjectOperations::convertFrameRate(ProjectPage& page) {
    convertFrameRateFrom(page, std::nullopt);
}

void ProjectOperations::convertFrameRateFrom(ProjectPage& page,
                                             const std::optional<core::RateConversion>& proposed) {
    const core::Project& project = page.session->project();

    // Pre-filled with the project's own, never guessed: the file does not
    // carry it, and getting it wrong shifts everything without a word. What the
    // film declares is handed over beside it, and the dialog decides what to do
    // with it — proposed, never imposed (D6).
    const std::optional<core::AssociatedVideo>& associated = project.video();
    // **Only a clean grid pre-fills the field.** A partial one is evidence the
    // deduction itself calls partial, and this field decides an operation on
    // the whole file; the status bar and the analysis carry that case instead.
    //
    // **And a document counted in frames leaves it out entirely.** Its
    // positions come from its frames at the rate it was read at, so the
    // deduction can only find that rate again; what is offered instead is the
    // rate itself, said for what it is.
    const std::optional<core::FrameRate> read = rateReadInFrames(project);
    const core::FrameRateDeduction grid = core::deduceFrameRate(project);
    const std::optional<core::FrameRate> measured =
        !read.has_value() && grid.verdict == core::GridVerdict::Clean
            ? std::optional{grid.retained.rate}
            : std::nullopt;

    FrameRateDialog dialog{scopeOf(page),
                           project.frameRate(),
                           associated.has_value() ? associated->declared : std::nullopt,
                           measured,
                           read,
                           m_view->dialogParent()};
    // **Filled and not applied**: the analysis found these two, and the dialog still asks.
    if (proposed.has_value())
        dialog.setRates(proposed->input, proposed->output);
    offerPreviewOf(dialog, page, [&dialog, &project](const core::Selection& target) {
        return std::make_unique<core::ConvertFrameRateCommand>(
            project, target, dialog.input(), dialog.output());
    });
    if (!m_prompts->run(dialog))
        return;

    const core::Selection target = chosenIn(page, dialog);
    apply(page,
          std::make_unique<core::ConvertFrameRateCommand>(
              project, target, dialog.input(), dialog.output()),
          target);
}

void ProjectOperations::adjustDurations(ProjectPage& page) {
    DurationAdjustDialog dialog{scopeOf(page), m_durationSettings, m_view->dialogParent()};
    offerPreviewOf(dialog, page, [&dialog, &page](const core::Selection& target) {
        return core::adjustDurations(
                   page.session->project(), target, core::constraintsOf(dialog.settings()))
            .command;
    });
    if (!m_prompts->run(dialog))
        return;
    const core::Selection target = chosenIn(page, dialog);

    // Kept even if nothing moves: it is what was asked, and the next dialog
    // offers it again.
    m_durationSettings = dialog.settings();

    core::DurationAdjustment adjustment = core::adjustDurations(
        page.session->project(), target, core::constraintsOf(m_durationSettings));
    const std::string account =
        core::noticeOfAdjustment(adjustment.adjusted, adjustment.sacrificed);

    // One box for both: lengthening an end is exactly what can carry it past
    // the film, and two modal boxes in a row was one too many — issue #418.
    std::string pastTheEnd;
    if (adjustment.command != nullptr)
        pastTheEnd = applyQuietly(page, std::move(adjustment.command), target);

    m_prompts->reportOutcome(joinedNotices(account, pastTheEnd));
}

ToolsView ProjectOperations::currentTools() const {
    return ToolsView{.settings = m_transcriptionSettings,
                     .tools = m_view->transcriptionTools(m_transcriptionSettings)};
}

void ProjectOperations::transcribeAudio(ProjectPage& page) {
    const std::optional<std::filesystem::path> video = m_view->videoOf(page);
    if (!video.has_value())
        return;

    const std::vector<core::AudioTrack> tracks = m_view->audioTracksOf(page);
    TranscribeDialog dialog{
        tracks,
        TranscribeChoice{.language = m_transcriptionSettings.language,
                         .suppressNotes = m_transcriptionSettings.suppressNotes},
        [this] { return currentTools(); },
        [this] { m_view->openPreferences(); },
        m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;
    const TranscribeChoice choice = dialog.choice();

    // Kept as asked, even when the transcription then fails: the next one offers it again.
    m_transcriptionSettings.language = choice.language;
    m_transcriptionSettings.suppressNotes = choice.suppressNotes;

    // **Asked before the work, not after the preview**: an hour of computing is not started by
    // mistake. Whoever declines does not wait for a result they will not use; the preview shows the
    // result and asks again what to do with it. Nothing to ask when there is nothing to lose.
    const std::size_t held = page.session->project().count();
    if (held > 0 && !m_prompts->confirmReplacement(core::noticeOfReplacement(held)))
        return;

    const TranscriptionJob job{.video = *video,
                               .track = choice.track,
                               .language = choice.language,
                               .model = m_transcriptionSettings.model,
                               .settings = m_transcriptionSettings,
                               .start = choice.start,
                               .end = choice.end};
    std::expected<TranscriptionOutcome, TranscriptionFailure> outcome = m_view->transcribe(job);

    // The pieces to show, whole when the work ended, partial when it stopped having produced some.
    std::vector<core::TranscribedSegment> pieces;
    std::string language;
    std::optional<core::Timestamp> partialUpTo;
    if (outcome) {
        pieces = std::move(outcome->segments);
        language = std::move(outcome->language);
    } else {
        const TranscriptionFailure& failure = outcome.error();
        if (!failure.cancelled)
            m_prompts->reportFailure(failure.message);
        // **What a long run had obtained is not thrown away unasked**: it is offered below,
        // partial.
        if (failure.partial.empty()) {
            if (failure.cancelled)
                m_view->announce(core::transcriptionErrorText(
                    {.kind = core::TranscriptionErrorKind::Cancelled}));
            return;
        }
        pieces = failure.partial;
        partialUpTo = pieces.back().end;
    }

    // **The decision comes after the result is seen**: "apply" replaces the subtitles of the
    // project (as one undoable operation, so a person with work in it can still go back), "ignore"
    // loses what was transcribed. That is the confirmation; nothing is asked before the work.
    core::Assembly assembly = core::assemble(pieces, core::assemblyOf(m_transcriptionSettings));
    TranscriptionPreviewDialog preview{TranscriptionPreview{.pieces = std::move(pieces),
                                                            .assembly = std::move(assembly),
                                                            .settings = m_transcriptionSettings,
                                                            .partialUpTo = partialUpTo},
                                       m_view->dialogParent()};
    const bool applied = m_prompts->run(preview);

    // **Tuned for this transcript, kept only when asked**: the button hands the three limits to the
    // settings of the application, whether the result is then applied or ignored.
    if (const auto& kept = preview.keptSettings()) {
        m_transcriptionSettings.mergeGapMilliseconds = kept->mergeGapMilliseconds;
        m_transcriptionSettings.maxDurationMilliseconds = kept->maxDurationMilliseconds;
        m_transcriptionSettings.maxCharacters = kept->maxCharacters;
    }
    if (!applied)
        return;

    std::vector<core::Subtitle> subtitles = preview.applied();
    const std::size_t made = subtitles.size();
    std::unique_ptr<core::Command> command =
        core::replaceAllSubtitles(page.session->project(), std::move(subtitles));
    if (command != nullptr) {
        const core::Selection everything = core::Selection::all(page.session->project());
        apply(page, std::move(command), everything);
    }
    m_view->announce(partialUpTo ? core::noticeOfPartialTranscription(made, *partialUpTo)
                                 : core::noticeOfTranscription(made, language));
}

void ProjectOperations::synchronizeWithAudio(ProjectPage& page, const core::Selection& selection) {
    const std::optional<std::filesystem::path> video = m_view->videoOf(page);
    if (!video.has_value() || page.session->project().count() == 0)
        return;

    const std::vector<core::AudioTrack> tracks = m_view->audioTracksOf(page);
    const bool selected = !selection.isEmpty();
    TranscribeDialog dialog{
        tracks,
        TranscribeChoice{.language = m_transcriptionSettings.language,
                         .minimumMatchPercent = m_alignmentSettings.minimumMatchPercent,
                         .doubtGapMilliseconds =
                             static_cast<int>(m_alignmentSettings.doubtGapMilliseconds)},
        [this] { return currentTools(); },
        [this] { m_view->openPreferences(); },
        m_view->dialogParent()};
    dialog.forSynchronization(
        {.language = m_transcriptionSettings.language,
         .minimumMatchPercent = m_alignmentSettings.minimumMatchPercent,
         .doubtGapMilliseconds = static_cast<int>(m_alignmentSettings.doubtGapMilliseconds)},
        selected);
    if (!m_prompts->run(dialog))
        return;
    const TranscribeChoice choice = dialog.choice();

    // Kept as asked, even when the run then fails: the next one offers it again.
    m_transcriptionSettings.language = choice.language;
    m_alignmentSettings.minimumMatchPercent = choice.minimumMatchPercent;
    m_alignmentSettings.doubtGapMilliseconds = choice.doubtGapMilliseconds;

    const core::Project& project = page.session->project();
    const core::LinesToSynchronize looked =
        choice.selectionOnly ? core::subtitlesOf(project, selection)
                             : core::subtitlesBeginningIn(project, choice.start, choice.end);
    if (looked.subtitles.empty())
        return;

    // The selected subtitles are looked for in the part of the film around them: a margin on each
    // side, so that a file that is late or early is still inside what is listened to.
    std::optional<core::Timestamp> from = choice.start;
    std::optional<core::Timestamp> to = choice.end;
    if (choice.selectionOnly) {
        constexpr std::int64_t kMarginMilliseconds = 60000;
        const core::Subtitle& first =
            project.subtitleAt(core::SubtitleIndex::fromValue(looked.subtitles.front()));
        const core::Subtitle& last =
            project.subtitleAt(core::SubtitleIndex::fromValue(looked.subtitles.back()));
        from = core::Timestamp::fromMilliseconds(
            std::max<std::int64_t>(0, first.start.milliseconds() - kMarginMilliseconds));
        to = core::Timestamp::fromMilliseconds(last.end.milliseconds() + kMarginMilliseconds);
    }

    const AlignmentJob job{.video = *video,
                           .track = choice.track,
                           .language = choice.language,
                           .model = m_transcriptionSettings.model,
                           .settings = m_transcriptionSettings,
                           .alignment = m_alignmentSettings,
                           .lines = looked.lines,
                           .start = from,
                           .end = to};
    const std::expected<AlignmentOutcome, TranscriptionFailure> outcome = m_view->align(job);
    if (!outcome) {
        if (!outcome.error().cancelled)
            m_prompts->reportFailure(outcome.error().message);
        else
            m_view->announce(
                core::transcriptionErrorText({.kind = core::TranscriptionErrorKind::Cancelled}));
        return;
    }

    const core::DoubtLimits limits{.partialBelow = core::kDefaultPartialBelow,
                                   .gapMilliseconds = m_alignmentSettings.doubtGapMilliseconds};
    const core::SynchronizationPlan plan =
        core::planSynchronization(project, looked.subtitles, outcome->matches, limits);
    if (plan.report.positioned() == 0) {
        m_prompts->reportFailure(core::noLineFoundInTheAudio(looked.lines.size()));
        return;
    }

    // **The decision comes after the result is seen**: nothing moves until it is applied.
    const core::AlignmentReport report = plan.report;
    SynchronizationPreviewDialog preview{
        SynchronizationPreview{.texts = looked.lines, .plan = plan}, m_view->dialogParent()};
    if (!m_prompts->run(preview))
        return;

    std::unique_ptr<core::Command> command = core::synchronizationCommand(project, plan);
    if (command != nullptr) {
        const core::Selection everything = core::Selection::all(page.session->project());
        apply(page, std::move(command), everything);
    }
    m_view->announce(core::noticeOfAlignment(report));
}

void ProjectOperations::appendFile(ProjectPage& page) {
    const std::optional<std::filesystem::path> chosen = m_view->fileToAppend();
    if (!chosen.has_value())
        return;

    std::expected<core::OpenedFile, std::string> opened = m_view->read(*chosen);
    if (!opened) {
        m_prompts->reportFailure(opened.error());
        return;
    }

    // What the reading ran into, whether or not there was anything to append —
    // the panel of what the last reading met, as an ordinary opening shows it.
    if (!opened->diagnostics.empty())
        m_view->showDiagnostics(opened->diagnostics);

    core::AppendedFile appended = core::appendFile(page.session->project(), opened->project);
    if (appended.command == nullptr)
        return;

    // Read before the command goes: the project it names is about to grow.
    const core::SubtitleFormat from = opened->project.sourceFile().format;
    const core::SubtitleFormat to = page.session->project().sourceFile().format;
    const std::size_t first = page.session->project().count();
    const core::Selection target =
        core::Selection::range(core::SubtitleIndex::fromValue(first),
                               core::SubtitleIndex::fromValue(first + appended.inserted - 1));

    const std::string pastTheEnd = applyQuietly(page, std::move(appended.command), target);

    // The rows the append just wrote: what a second append starts past, and
    // what selecting them shows was added.
    m_view->selectRows(static_cast<int>(first), static_cast<int>(first + appended.inserted - 1));

    // **In the status bar when there was nothing else to say, in a box to
    // close otherwise** — the rule #398 set for a gesture that has something
    // to say.
    const std::string account = core::noticeOfAppend(appended.inserted, appended.loss, from, to);
    if (!appended.loss.isAny() && pastTheEnd.empty()) {
        m_view->announce(account);
        return;
    }
    m_prompts->reportOutcome(joinedNotices(account, pastTheEnd));
}

std::optional<core::DisjointReport>
ProjectOperations::disjointedQuietly(ProjectPage& page, const core::DisjointSettings& settings) {
    std::expected<core::DisjointRanges, core::DisjointRefusal> made =
        core::makeDisjoint(page.session->project(), settings);
    if (!made)
        return std::nullopt;

    if (made->command != nullptr) {
        const std::vector<core::Change> changes = page.session->apply(std::move(made->command));
        page.model->applied(changes);
        page.placedAt = -1;
    }
    if (made->touched.has_value()) {
        m_view->selectRows(static_cast<int>(made->touched->first.value()),
                           static_cast<int>(made->touched->last.value()));
    }
    return made->report;
}

void ProjectOperations::tell(const std::string& account, bool stopAndRead) {
    if (stopAndRead)
        m_prompts->reportOutcome(account);
    else
        m_view->announce(account);
}

void ProjectOperations::sortSubtitles(ProjectPage& page) {
    if (page.session->project().isInOrder()) {
        m_view->announce(core::noticeOfSort(0));
        return;
    }

    // **The bar is for the file of two thousand lines**, where the window would
    // otherwise sit still for as long as it takes to move them and to tell the
    // table so, and a user cannot tell that from a window that has hung.
    const BusyScope busy{*m_view};
    m_view->announce(core::noticeOfSort(sortedQuietly(page)));
}

void ProjectOperations::joinRepeats(ProjectPage& page) {
    const BusyScope busy{*m_view};
    m_view->announce(core::noticeOfJoin(joinedQuietly(page)));
}

void ProjectOperations::makeDisjoint(ProjectPage& page) {
    if (!page.session->project().isInOrder()) {
        m_prompts->reportFailure(core::refusalOfDisjoint(core::DisjointRefusal::NotInOrder));
        return;
    }

    DisjointDialog dialog{m_disjointChoice, m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;
    m_disjointChoice = dialog.choice();

    const BusyScope busy{*m_view};
    const std::optional<core::DisjointReport> report =
        disjointedQuietly(page, settingsOf(m_disjointChoice));
    if (report.has_value())
        tell(core::noticeOfDisjoint(*report), hasCaveat(*report));
}

void ProjectOperations::repair(ProjectPage& page) {
    DisjointDialog dialog{m_disjointChoice, m_view->dialogParent(), true};
    if (!m_prompts->run(dialog))
        return;
    m_disjointChoice = dialog.choice();

    fixAtOpening(page, true, true, true);
}

void ProjectOperations::fixAtOpening(ProjectPage& page, bool join, bool sort, bool disjoint) {
    if (!join && !sort && !disjoint)
        return;

    const BusyScope busy{*m_view};
    std::vector<std::string> accounts;
    bool stopAndRead = false;
    if (join)
        accounts.push_back(core::noticeOfJoin(joinedQuietly(page)));
    if (sort)
        accounts.push_back(core::noticeOfSort(sortedQuietly(page)));
    if (disjoint) {
        const std::optional<core::DisjointReport> report =
            disjointedQuietly(page, settingsOf(m_disjointChoice));
        if (report.has_value()) {
            accounts.push_back(core::noticeOfDisjoint(*report));
            stopAndRead = hasCaveat(*report);
        }
    }

    std::string account;
    for (const std::string& each : accounts)
        account += (account.empty() ? "" : "; ") + each;
    if (!account.empty())
        tell(account, stopAndRead);
}

void ProjectOperations::splitProject(ProjectPage& page) {
    const core::Project& project = page.session->project();
    QItemSelectionModel& selection = *page.tableSelection;

    // Opens on the current row, the natural place to cut: « from here ».
    const int current = selection.currentIndex().row();
    SplitProjectDialog dialog{project.count(),
                              static_cast<std::size_t>(std::max(current, 0)) + 1,
                              m_view->dialogParent()};

    // **Where the cut falls, shown before it is made**, as Gaupol does: each
    // number the box takes selects its row. Cancelling gives the selection
    // back — issue #462.
    const QItemSelection before = selection.selection();
    const QModelIndex wasCurrent = selection.currentIndex();
    QObject::connect(&dialog, &SplitProjectDialog::rowChosen, &dialog, [this](int row) {
        m_view->selectRows(row, row);
    });
    if (!m_prompts->run(dialog)) {
        selection.setCurrentIndex(wasCurrent, QItemSelectionModel::NoUpdate);
        selection.select(before, QItemSelectionModel::ClearAndSelect);
        return;
    }

    const core::SubtitleIndex from = core::SubtitleIndex::fromValue(dialog.firstOfTail());
    std::expected<core::SplitProject, core::SplitRefusal> split = core::splitProject(project, from);
    if (!split) {
        m_prompts->reportFailure(
            core::refusalOfSplit(from.value() + 1, split.error().before.value() + 1));
        return;
    }

    // The origin loses the tail in one entry of its own history; the new
    // project begins another, and the two know nothing of each other.
    const core::Selection tail =
        core::Selection::range(from, core::SubtitleIndex::fromValue(project.count() - 1));
    (void)applyQuietly(page, std::move(split->command), tail);

    const std::size_t moved = split->tail.count();
    m_view->openAside(std::move(split->tail));
    m_view->announce(core::noticeOfSplit(moved));
}

void ProjectOperations::removeHearingImpaired(ProjectPage& page) {
    HearingImpairedDialog dialog{scopeOf(page), m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;
    const core::Selection target = chosenIn(page, dialog);

    // Built before being applied, and asked what it will do: the count is read
    // from the command, never by counting again afterwards.
    std::unique_ptr<core::Command> command =
        core::removeHearingImpaired(page.session->project(), target, m_view->targetDocument());
    if (!command) {
        // Nothing bit. Say so, and put nothing in the history: an operation
        // that changes nothing is not an operation to undo.
        m_prompts->reportOutcome(core::noMentionToRemove());
        return;
    }

    const core::HearingImpairedTally tally = core::tallyOf(*command);
    apply(page, std::move(command), target);

    m_prompts->reportOutcome(core::noticeOfMentionsRemoved(tally.cleaned, tally.removed));
}

void ProjectOperations::toggleItalics(ProjectPage& page) {
    const core::Selection target = targetIn(page);

    // Asked before anything is built, and of the target rather than of the
    // document: the button says what it will do to what is selected.
    const core::Document document = m_view->targetDocument();
    const bool italic = core::wouldItalicise(page.session->project(), target, document);

    std::unique_ptr<core::Command> command =
        core::setItalics(page.session->project(), target, document, italic);
    if (!command) {
        // Every text was already the way it was asked for. Say so, and put
        // nothing in the history: an operation that changes nothing is not an
        // operation to undo.
        m_view->announce(core::nothingToChange());
        return;
    }

    // Read from the command before it goes, never by counting again after.
    const std::size_t rewritten = core::rewrittenCount(*command);
    apply(page, std::move(command), target);
    m_view->announce(core::noticeOfItalics(rewritten, italic));
}

void ProjectOperations::changeCase(ProjectPage& page, core::LetterCase wanted) {
    const core::Selection target = targetIn(page);

    std::unique_ptr<core::Command> command =
        core::setLetterCase(page.session->project(), target, m_view->targetDocument(), wanted);
    if (!command) {
        m_view->announce(core::nothingToChange());
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    apply(page, std::move(command), target);
    m_view->announce(core::noticeOfRecase(rewritten));
}

void ProjectOperations::toggleDialogueDashes(ProjectPage& page) {
    const core::Selection target = targetIn(page);

    // Asked of the target before anything is built: the entry says what it will
    // do to what is selected.
    const core::Document document = m_view->targetDocument();
    const bool dashed = core::wouldAddDialogueDashes(page.session->project(), target, document);

    std::unique_ptr<core::Command> command =
        core::setDialogueDashes(page.session->project(), target, document, dashed);
    if (!command) {
        m_view->announce(core::nothingToChange());
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    apply(page, std::move(command), target);
    m_view->announce(core::noticeOfDialogueDashes(rewritten, dashed));
}

void ProjectOperations::snap(ProjectPage& page) {
    const std::optional<core::AssociatedVideo>& associated = page.session->project().video();

    SnapDialog dialog{scopeOf(page),
                      page.session->project().frameRate(),
                      associated.has_value() ? associated->declared : std::nullopt,
                      m_view->dialogParent()};
    offerPreviewOf(dialog, page, [&dialog, &page](const core::Selection& target) {
        return std::make_unique<core::SnapCommand>(page.session->project(), target, dialog.rate());
    });
    if (!m_prompts->run(dialog))
        return;
    const core::Selection target = chosenIn(page, dialog);

    const std::string pastTheEnd = applyQuietly(
        page,
        std::make_unique<core::SnapCommand>(page.session->project(), target, dialog.rate()),
        target);

    // **What the table showed and the two grid surfaces did not** — issue #324.
    // An operation takes the selection; the grid speaks of the document. Align
    // five rows out of a hundred and seventy-six and the timestamps move under
    // the user's eyes while the status bar and the analysis stay put, which
    // reads as a refresh that failed. It is not one: they have nothing to say.
    //
    // Said here rather than in `apply`, which knows a command and a target and
    // not the rate that was asked for — and this is the only operation that
    // asks for one.
    //
    // The same box as what the alignment left past the end of the film, when it
    // left anything — issue #418.
    const std::optional<core::PartialAlignment> partial =
        core::partialAlignment(page.session->project(), target, dialog.rate());
    const std::string behind = partial.has_value() ? core::noticeOf(*partial) : std::string{};
    if (const std::string notice = joinedNotices(behind, pastTheEnd); !notice.empty())
        m_prompts->reportOutcome(notice);
}

void ProjectOperations::shiftOntoGrid(ProjectPage& page) {
    const std::optional<core::Duration> by =
        core::shiftOntoGrid(core::deduceFrameRate(page.session->project()));
    if (!by.has_value())
        return;

    const core::Selection whole = core::Selection::all(page.session->project());
    if (const std::optional<std::string> refusal =
            refusalOfShift(page.session->project(), whole, *by);
        refusal.has_value()) {
        m_prompts->reportFailure(*refusal);
        return;
    }

    apply(page, std::make_unique<core::ShiftCommand>(whole, *by), whole);
}

void ProjectOperations::analyseGrid(ProjectPage& page) {
    const core::Project& project = page.session->project();
    GridAnalysisDialog dialog{
        core::deduceFrameRate(project), core::findGridRepair(project), m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

    // The button was pressed: the conversion found is handed to the dialog that asks.
    const std::optional<core::FrameRate> input = dialog.requestedInput();
    const std::optional<core::FrameRate> output = dialog.requestedOutput();
    if (input.has_value() && output.has_value())
        convertFrameRateFrom(page, core::RateConversion{.input = *input, .output = *output});
}

} // namespace subedit::gui
