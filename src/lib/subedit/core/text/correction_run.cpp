#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
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
#include <subedit/core/text/line_breaking.hpp>

#include <algorithm>
#include <tuple>
#include <variant>

namespace subedit::core {

namespace {

/// The activation `settings` sets for `pattern`, on top of its shipped
/// default — decision D2, by kind, code and name.
[[nodiscard]] bool isEnabled(const CorrectionPattern& pattern, const CorrectionSettings& settings) {
    for (const PatternActivation& activation : settings.patternActivations) {
        if (activation.kind == pattern.kind() && activation.code == pattern.code &&
            activation.name == pattern.name)
            return activation.enabled;
    }
    return pattern.enabled;
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

/// The patterns of `kind` the cascade of `code` gives, activation and D4's
/// classes both applied.
[[nodiscard]] std::vector<const CorrectionPattern*>
selectedPatterns(const PatternCatalogue& catalogue,
                 PatternKind kind,
                 const std::string& code,
                 const CorrectionSettings& settings) {
    std::vector<const CorrectionPattern*> chosen;
    for (const CorrectionPattern* pattern : catalogue.cascade(kind, code)) {
        if (!isEnabled(*pattern, settings))
            continue;
        if (!classesAllow(*pattern, settings))
            continue;
        chosen.push_back(pattern);
    }
    return chosen;
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

/// Runs one target's tasks in Gaupol's order, cascading each on the text the
/// one before it left.
void runTasks(const PatternEngine& engine,
              const PatternCatalogue& catalogue,
              const CorrectionSettings& settings,
              const LineMeasure& measure,
              const CorrectionTarget& target,
              SubtitleFormat format,
              std::vector<std::optional<std::string>>& texts,
              std::vector<PatternFailure>& failures) {
    if (settings.mentions.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns = selectedPatterns(
            catalogue, PatternKind::HearingImpaired, settings.mentions.code, settings);
        const HearingImpairedCorrection done =
            correctHearingImpaired(engine,
                                   patterns,
                                   present.texts,
                                   format,
                                   settings.soundInBrackets || settings.soundInParentheses);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
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

    if (settings.commonErrors.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns = selectedPatterns(
            catalogue, PatternKind::CommonError, settings.commonErrors.code, settings);
        const CorrectedTexts done = correctCommonErrors(engine, patterns, present.texts, format);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done.texts[k];
    }

    if (settings.capitalization.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns = selectedPatterns(
            catalogue, PatternKind::Capitalization, settings.capitalization.code, settings);
        const CorrectedTexts done = correctCapitalization(engine, patterns, present.texts, format);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done.texts[k];
    }

    if (settings.lineBreak.enabled) {
        const Present present = presentOf(texts);
        const std::vector<const CorrectionPattern*> patterns =
            selectedPatterns(catalogue, PatternKind::LineBreak, settings.lineBreak.code, settings);
        const BrokenTexts done = breakLines(engine,
                                            patterns,
                                            present.texts,
                                            measure,
                                            settings.lineBreakMaxLength,
                                            settings.lineBreakMaxLines);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
        for (std::size_t k = 0; k < present.at.size(); ++k)
            texts[present.at[k]] = done.texts[k];
    }
}

} // namespace

CorrectionProposal proposeCorrections(const PatternEngine& engine,
                                      const PatternCatalogue& catalogue,
                                      const CorrectionSettings& settings,
                                      const LineMeasure& measure,
                                      std::span<const CorrectionTarget> targets) {
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
        runTasks(engine, catalogue, settings, measure, target, format, texts, proposal.failures);

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

    // De-duplicate: the same pattern reports the same compile failure once per
    // target it was asked to run on, and the confirmation names it once.
    std::ranges::sort(proposal.failures, {}, [](const PatternFailure& f) {
        return std::tuple{static_cast<int>(f.kind), f.code, f.rank, f.text.value_or(0)};
    });
    proposal.failures.erase(
        std::ranges::unique(proposal.failures,
                            [](const PatternFailure& a, const PatternFailure& b) {
                                return a.kind == b.kind && a.code == b.code && a.rank == b.rank &&
                                       a.name == b.name && a.text == b.text;
                            })
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
