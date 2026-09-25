#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/edit/split_project.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording.hpp>
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
#include <subedit/gui/target.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QObject>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <filesystem>
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
    return "subtitle " + std::to_string(refused->number()) +
           " would start before the origin, which no subtitle file can hold";
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
    const core::Selection target = targetIn(page);

    ShiftDialog dialog{target.count(), m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

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
    const core::Selection target = targetIn(page);

    TransformDialog dialog{target.count(), page.session->project().count(), m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

    const std::optional<TypedReference> first = dialog.first();
    const std::optional<TypedReference> second = dialog.second();
    if (!first.has_value() || !second.has_value())
        return;

    // What the dialog read becomes the core's own value here: it holds
    // widgets, not the vocabulary of a command.
    const auto referenceOf = [](const TypedReference& typed) {
        return core::TransformReference{
            .index = core::SubtitleIndex::fromNumber(static_cast<std::size_t>(typed.number)),
            .target = typed.target,
        };
    };

    std::optional<core::TransformCommand> command = core::TransformCommand::create(
        page.session->project(), target, referenceOf(*first), referenceOf(*second));
    if (!command.has_value()) {
        m_prompts->reportFailure("the two references define no correction");
        return;
    }

    apply(page, std::make_unique<core::TransformCommand>(std::move(*command)), target);
}

void ProjectOperations::convertFrameRate(ProjectPage& page) {
    const core::Selection target = targetIn(page);
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

    FrameRateDialog dialog{target.count(),
                           project.frameRate(),
                           associated.has_value() ? associated->declared : std::nullopt,
                           measured,
                           read,
                           m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

    apply(page,
          std::make_unique<core::ConvertFrameRateCommand>(
              project, target, dialog.input(), dialog.output()),
          target);
}

void ProjectOperations::adjustDurations(ProjectPage& page) {
    const core::Selection target = targetIn(page);

    DurationAdjustDialog dialog{target.count(), m_durationSettings, m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

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
        m_prompts->reportFailure("Cannot split at subtitle " + std::to_string(from.value() + 1) +
                                 ": subtitle " + std::to_string(split.error().before.value() + 1) +
                                 " would fall before the start of the video. Cut somewhere else.");
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
    const core::Selection target = targetIn(page);

    HearingImpairedDialog dialog{target.count(), m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

    // Built before being applied, and asked what it will do: the count is read
    // from the command, never by counting again afterwards.
    std::unique_ptr<core::Command> command =
        core::removeHearingImpaired(page.session->project(), target, m_view->targetDocument());
    if (!command) {
        // Nothing bit. Say so, and put nothing in the history: an operation
        // that changes nothing is not an operation to undo.
        m_prompts->reportOutcome("no mention to remove");
        return;
    }

    const core::HearingImpairedTally tally = core::tallyOf(*command);
    apply(page, std::move(command), target);

    m_prompts->reportOutcome(core::countOf(tally.cleaned, "subtitle") + " cleaned, " +
                             std::to_string(tally.removed) + " removed");
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
    const core::Selection target = targetIn(page);
    const std::optional<core::AssociatedVideo>& associated = page.session->project().video();

    SnapDialog dialog{target.count(),
                      page.session->project().frameRate(),
                      associated.has_value() ? associated->declared : std::nullopt,
                      m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return;

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

void ProjectOperations::analyseGrid(const ProjectPage& page) {
    GridAnalysisDialog dialog{core::deduceFrameRate(page.session->project()),
                              m_view->dialogParent()};
    (void)m_prompts->run(dialog);
}

} // namespace subedit::gui
