#pragma once

// The correction assistant's calculation, at the core — decision D8 of the
// spec of phase 12, issue #504.
//
// A function of texts, like every correction the phase already wrote: it
// takes the tasks and their settings, the projects and the target, and gives
// back the changes it would make, **without touching a single project** —
// Gaupol copies each project to do this; a correction being a function of
// texts, there is nothing to copy. A second function applies an accepted
// subset, one composed command per project.

#include <subedit/core/command/command.hpp>
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/common_errors.hpp> // for PatternFailure
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/text/pattern_engine.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {

class Project;

/// The tasks the assistant computes, in Gaupol's own order — the one
/// `proposeCorrections` applies them in. Joining and splitting words is
/// absent without a correcteur (decision D6) and comes with #508.
enum class CorrectionTask {
    Mentions,
    CommonErrors,
    Capitalization,
    LineBreak,
};

/// One project, the subtitles of `document` the assistant looks at.
///
/// `selection` names the subtitles in ascending order, as every `Selection`
/// already keeps them; resolving "the selection", "the current project" or
/// "every project open" into one of these is the caller's — a window's, once
/// there is one — not this function's.
struct CorrectionTarget {
    const Project* project = nullptr;
    Selection selection;
    Document document = Document::Main;
};

/// One text the assistant proposes to change, `proposed` absent for a
/// subtitle it would remove.
///
/// **Only real changes are here.** A subtitle a task left exactly as it found
/// it earns no entry: decision D8 shows a confirmation page of changed texts,
/// never a page a user has to read past to find the ones that matter.
struct ProposedCorrection {
    const Project* project = nullptr;
    SubtitleIndex index;
    Document document = Document::Main;
    std::string original;
    std::optional<std::string> proposed;
};

/// What `proposeCorrections` answered: the changes it would make, and every
/// pattern that could not do its part — named once each, however many texts
/// or targets it was asked to work on (GUI-CORRECT-06).
struct CorrectionProposal {
    std::vector<ProposedCorrection> corrections;
    std::vector<PatternFailure> failures;
};

/// Computes what `tasks` — read from `settings` — would do to `targets`,
/// under `catalogue` and `engine`, measuring line breaks with `measure`.
///
/// **The order is Gaupol's**: mentions, common errors, capitalization,
/// line-break — each on the text the one before it left. A subtitle mentions
/// empties plays no part in what follows: the tasks after it never see it.
///
/// A mention that would empty a translation is written empty instead of
/// removed — the subtitle carries a main text nobody aimed at taking away,
/// the same rule `removeHearingImpaired` already keeps for phase 4's direct
/// command.
[[nodiscard]] CorrectionProposal proposeCorrections(const PatternEngine& engine,
                                                    const PatternCatalogue& catalogue,
                                                    const CorrectionSettings& settings,
                                                    const LineMeasure& measure,
                                                    std::span<const CorrectionTarget> targets);

/// One project's worth of what `applyCorrections` composed for it.
struct AppliedCorrection {
    const Project* project = nullptr;
    std::unique_ptr<Command> command;
};

/// Composes `accepted` — a subset of what `proposeCorrections` answered —
/// into **one `CompositeCommand` per project, not yet applied**: the caller
/// runs it the ordinary way, `Session::apply(command)`, so undoing it is one
/// gesture per project and the session's own history stays the only road to a
/// change (`Session::project()`'s own rule). `tallyOf` below reads a composed
/// command before it is applied — the same order every other tally in this
/// codebase already uses.
///
/// A removed subtitle is taken away when `removeBlankSubtitles`, and left
/// holding its emptied text otherwise — Gaupol's own checkbox, cochée par
/// défaut.
///
/// A project with nothing accepted is absent from the answer: an operation
/// that changes nothing is not an operation to undo.
[[nodiscard]] std::vector<AppliedCorrection>
applyCorrections(std::span<const ProposedCorrection> accepted, bool removeBlankSubtitles);

/// What `applyCorrections` did — decision D8's own count, never of matches.
struct CorrectionTally {
    std::size_t corrected = 0;
    std::size_t removed = 0;

    friend bool operator==(const CorrectionTally&, const CorrectionTally&) = default;
};

/// Reads what `applied` did from the commands themselves, the same rule
/// `tallyOf(const Command&)` already follows for phase 4's removal.
[[nodiscard]] CorrectionTally tallyOf(std::span<const AppliedCorrection> applied);

} // namespace subedit::core
