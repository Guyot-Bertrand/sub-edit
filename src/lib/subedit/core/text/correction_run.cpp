#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/capitalization.hpp>
#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/hearing_impaired_correction.hpp>
#include <subedit/core/text/join_split_words.hpp>
#include <subedit/core/text/line_breaking.hpp>

#include <algorithm>
#include <optional>
#include <tuple>
#include <variant>

namespace subedit::core {

namespace {

/// Gaupol's skip gate: on if either check is, and a check turned off is a
/// threshold no text reaches.
[[nodiscard]] std::optional<SkipLimits> skipLimitsOf(const CorrectionSettings& settings) {
    if (!settings.lineBreakSkipOnLength && !settings.lineBreakSkipOnLines)
        return std::nullopt;
    SkipLimits skip;
    if (settings.lineBreakSkipOnLength)
        skip.maxLength = settings.lineBreakSkipMaxLength;
    if (settings.lineBreakSkipOnLines)
        skip.maxLines = settings.lineBreakSkipMaxLines;
    return skip;
}

/// Decision D4: a record of kind `CommonError` applies if one of its classes
/// is checked; the other three kinds carry no class, and are never filtered
/// here.
[[nodiscard]] bool classesAllow(const CorrectionPattern& pattern,
                                const CorrectionSettings& settings) {
    const auto* fields = std::get_if<CommonErrorFields>(&pattern.fields);
    if (fields == nullptr)
        return true;
    return (fields->classes.human && settings.human) || (fields->classes.ocr && settings.ocr);
}

/// The subtitles a task has not yet removed: their positions in `texts`, and
/// their texts, dense — what a task reads.
///
/// **One pass, not two**: a position and its text are found together, so
/// nothing downstream can hold a position without the text that goes with it.
struct Present {
    std::vector<std::size_t> at;
    std::vector<std::string> texts;
};

[[nodiscard]] Present presentOf(const std::vector<std::optional<std::string>>& texts) {
    Present found;
    for (std::size_t i = 0; i < texts.size(); ++i) {
        const std::optional<std::string>& text = texts[i];
        if (!text.has_value())
            continue;
        found.at.push_back(i);
        found.texts.push_back(*text);
    }
    return found;
}

/// What a task reports, placed where its text really is.
///
/// A task reads the subtitles not yet removed, **densely**: the index a failure
/// carries is a position in that list. What `PatternFailure::text` promises is a
/// position in the target's own texts — the one `Selection::indices()` walks —
/// and the two agree only until a task has removed something. Translated here,
/// once, so that no caller has to know the tasks before it.
void appendFailures(std::vector<PatternFailure>& into,
                    const std::vector<PatternFailure>& reported,
                    const Present& present) {
    for (PatternFailure failure : reported) {
        if (failure.text.has_value())
            failure.text = present.at[*failure.text];
        into.push_back(std::move(failure));
    }
}

/// The mentions task, on the subtitles not yet removed.
void runMentions(const PatternEngine& engine,
                 const PatternCatalogue& catalogue,
                 const CorrectionSettings& settings,
                 const CorrectionTarget& target,
                 SubtitleFormat format,
                 std::vector<std::optional<std::string>>& texts,
                 std::vector<PatternFailure>& failures) {
    const Present present = presentOf(texts);
    const std::vector<const CorrectionPattern*> patterns =
        activePatterns(catalogue, PatternKind::HearingImpaired, settings.mentions.code, settings);
    const HearingImpairedCorrection done =
        correctHearingImpaired(engine,
                               patterns,
                               present.texts,
                               format,
                               settings.soundInBrackets || settings.soundInParentheses);
    appendFailures(failures, done.failures, present);
    for (std::size_t k = 0; k < present.at.size(); ++k) {
        std::optional<std::string> corrected = done.texts[k];
        // A translation carries no timing of its own: emptying it takes
        // nothing away from the subtitle, whose main text nobody aimed at
        // removing — the rule `removeHearingImpaired` already keeps.
        if (!corrected.has_value() && target.document == Document::Translation)
            corrected = std::string{};
        texts[present.at[k]] = std::move(corrected);
    }
}

/// The join-and-split task: join first, then split, each on the text the
/// other left, as Gaupol's page does. **Nothing without a checker** — no
/// dictionary is not a failure, the window says why it is off.
void runJoinSplit(const CorrectionSettings& settings,
                  const SpellChecker& spellChecker,
                  std::vector<std::optional<std::string>>& texts) {
    if (settings.joinWords) {
        const Present present = presentOf(texts);
        const std::vector<std::string> done = joinWords(spellChecker, present.texts);
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done[k];
    }
    if (settings.splitWords) {
        const Present present = presentOf(texts);
        const std::vector<std::string> done = splitWords(spellChecker, present.texts);
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done[k];
    }
}

/// Runs one target's tasks in Gaupol's order, cascading each on the text the
/// one before it left.
void runTasks(const PatternEngine& engine,
              const PatternCatalogue& catalogue,
              const CorrectionSettings& settings,
              const LineMeasure& measure,
              const CorrectionTarget& target,
              SubtitleFormat format,
              const SpellChecker* spellChecker,
              std::vector<std::optional<std::string>>& texts,
              std::vector<PatternFailure>& failures) {
    if (settings.mentions.enabled)
        runMentions(engine, catalogue, settings, target, format, texts, failures);

    if (settings.joinSplitEnabled && spellChecker != nullptr)
        runJoinSplit(settings, *spellChecker, texts);

    if (settings.commonErrors.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns = activePatterns(
            catalogue, PatternKind::CommonError, settings.commonErrors.code, settings);
        const CorrectedTexts done = correctCommonErrors(engine, patterns, present.texts, format);
        appendFailures(failures, done.failures, present);
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done.texts[k];
    }

    if (settings.capitalization.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns = activePatterns(
            catalogue, PatternKind::Capitalization, settings.capitalization.code, settings);
        const CorrectedTexts done = correctCapitalization(engine, patterns, present.texts, format);
        appendFailures(failures, done.failures, present);
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done.texts[k];
    }

    if (settings.lineBreak.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns =
            activePatterns(catalogue, PatternKind::LineBreak, settings.lineBreak.code, settings);
        const BrokenTexts done = breakLines(engine,
                                            patterns,
                                            present.texts,
                                            measure,
                                            settings.lineBreakMaxLength,
                                            settings.lineBreakMaxLines,
                                            skipLimitsOf(settings));
        appendFailures(failures, done.failures, present);
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done.texts[k];
    }
}

} // namespace

std::vector<const CorrectionPattern*> activePatterns(const PatternCatalogue& catalogue,
                                                     PatternKind kind,
                                                     const std::string& code,
                                                     const CorrectionSettings& settings) {
    std::vector<const CorrectionPattern*> chosen;
    for (const CorrectionPattern* pattern : catalogue.cascade(kind, code)) {
        if (!patternEnabled(*pattern, settings))
            continue;
        if (!classesAllow(*pattern, settings))
            continue;
        chosen.push_back(pattern);
    }
    return chosen;
}

CorrectionProposal proposeCorrections(const PatternEngine& engine,
                                      const PatternCatalogue& catalogue,
                                      const CorrectionSettings& settings,
                                      const LineMeasure& measure,
                                      std::span<const CorrectionTarget> targets,
                                      const SpellChecker* spellChecker) {
    CorrectionProposal proposal;

    for (const CorrectionTarget& target : targets) {
        std::vector<SubtitleIndex> indices;
        for (const SubtitleIndex index : target.selection.indices())
            indices.push_back(index);
        if (indices.empty())
            continue;

        const SubtitleFormat format = target.project->sourceFile(target.document).format;

        std::vector<std::string> originals;
        originals.reserve(indices.size());
        for (const SubtitleIndex index : indices)
            originals.push_back(target.project->subtitleAt(index).text(target.document));

        std::vector<std::optional<std::string>> texts(originals.begin(), originals.end());
        runTasks(engine,
                 catalogue,
                 settings,
                 measure,
                 target,
                 format,
                 spellChecker,
                 texts,
                 proposal.failures);

        for (std::size_t i = 0; i < indices.size(); ++i) {
            if (texts[i] == originals[i])
                continue;
            proposal.corrections.push_back(ProposedCorrection{.project = target.project,
                                                              .index = indices[i],
                                                              .document = target.document,
                                                              .original = originals[i],
                                                              .proposed = std::move(texts[i])});
        }
    }

    // De-duplicate by pattern and reason, never by text: the same pattern
    // reports the same failure once per target it ran on — and a per-text
    // one (time-out, passes, length) once per text — while the confirmation
    // names it once. `text` is only an index into one target's own texts, so
    // it could not tell two targets apart anyway; the stable sort keeps the
    // first report's `text` and `detail`, and nothing downstream reads more.
    const auto key = [](const PatternFailure& f) {
        return std::tie(f.kind, f.code, f.rank, f.name);
    };
    std::ranges::stable_sort(proposal.failures, {}, key);
    proposal.failures.erase(
        std::ranges::unique(
            proposal.failures,
            [&key](const PatternFailure& a, const PatternFailure& b) { return key(a) == key(b); })
            .begin(),
        proposal.failures.end());
    return proposal;
}

std::vector<AppliedCorrection> applyCorrections(std::span<const ProposedCorrection> accepted,
                                                bool removeBlankSubtitles) {
    struct Group {
        const Project* project;
        std::vector<const ProposedCorrection*> items;
    };

    std::vector<Group> groups;
    for (const ProposedCorrection& one : accepted) {
        auto found =
            std::ranges::find_if(groups, [&](const Group& g) { return g.project == one.project; });
        if (found == groups.end()) {
            groups.push_back(Group{.project = one.project, .items = {}});
            found = groups.end() - 1;
        }
        found->items.push_back(&one);
    }

    std::vector<AppliedCorrection> result;
    for (const Group& group : groups) {
        std::vector<std::unique_ptr<Command>> commands;
        std::vector<SubtitleIndex> emptied;

        // Every command is built before any is applied — the rule
        // `removeHearingImpaired` already keeps: each `SetTextCommand`
        // captures the text as the project still holds it, and the indices a
        // removal will use are those of the project as it still stands.
        for (const ProposedCorrection* item : group.items) {
            if (item->proposed.has_value()) {
                commands.push_back(std::make_unique<SetTextCommand>(
                    *group.project, item->index, item->document, *item->proposed));
                continue;
            }
            if (removeBlankSubtitles)
                emptied.push_back(item->index);
            else
                commands.push_back(std::make_unique<SetTextCommand>(
                    *group.project, item->index, item->document, std::string{}));
        }
        if (!emptied.empty())
            commands.push_back(std::make_unique<RemoveCommand>(Selection::of(emptied)));
        if (commands.empty())
            continue;

        std::unique_ptr<Command> composite =
            std::make_unique<CompositeCommand>(CommandKind::CorrectTexts, std::move(commands));
        result.push_back(
            AppliedCorrection{.project = group.project, .command = std::move(composite)});
    }
    return result;
}

CorrectionTally tallyOf(std::span<const AppliedCorrection> applied) {
    CorrectionTally tally;
    for (const AppliedCorrection& one : applied) {
        for (const Change& change : one.command->describe()) {
            if (change.kind == ChangeKind::Removal)
                tally.removed += change.subtitles.count();
            else
                ++tally.corrected;
        }
    }
    return tally;
}

} // namespace subedit::core
