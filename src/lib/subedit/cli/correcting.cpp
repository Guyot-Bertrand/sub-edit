#include <subedit/cli/changes.hpp>
#include <subedit/cli/correcting.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/hearing_impaired_correction.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/spell_dictionary.hpp>
#include <subedit/core/text/spell_replacements.hpp>
#include <subedit/core/wording/correction.hpp>
#include <subedit/core/wording/counts.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <span>
#include <string_view>
#include <system_error>
#include <utility>

namespace subedit::cli {

namespace {

/// The tasks this subcommand offers, and the kind of pattern each reads.
struct TaskName {
    std::string_view name;
    core::CorrectionTask task;
    core::PatternKind kind;

    /// Whether the task plays patterns. **Joining and splitting words do not**: they
    /// ask the spell-checker, and have no cascade, no code and no name to switch.
    bool readsPatterns = true;
};

constexpr std::array<TaskName, 6> kTasks{
    TaskName{.name = "mentions",
             .task = core::CorrectionTask::Mentions,
             .kind = core::PatternKind::HearingImpaired},
    TaskName{.name = "common-errors",
             .task = core::CorrectionTask::CommonErrors,
             .kind = core::PatternKind::CommonError},
    TaskName{.name = "capitalization",
             .task = core::CorrectionTask::Capitalization,
             .kind = core::PatternKind::Capitalization},
    TaskName{.name = "line-break",
             .task = core::CorrectionTask::LineBreak,
             .kind = core::PatternKind::LineBreak},
    // The kind is not read: these two have no patterns.
    TaskName{.name = "join-words",
             .task = core::CorrectionTask::JoinSplitWords,
             .kind = core::PatternKind::CommonError,
             .readsPatterns = false},
    TaskName{.name = "split-words",
             .task = core::CorrectionTask::JoinSplitWords,
             .kind = core::PatternKind::CommonError,
             .readsPatterns = false},
};

/// `text` cut at each `separator`, the empty pieces kept.
[[nodiscard]] std::vector<std::string_view> splitOn(std::string_view text, char separator) {
    std::vector<std::string_view> parts;
    while (true) {
        const std::size_t at = text.find(separator);
        parts.push_back(text.substr(0, at));
        if (at == std::string_view::npos) {
            return parts;
        }
        text.remove_prefix(at + 1);
    }
}

[[nodiscard]] std::vector<std::string_view> splitList(std::string_view text) {
    return splitOn(text, ',');
}

[[nodiscard]] const TaskName* taskNamed(std::string_view name) {
    const auto* const found =
        std::ranges::find_if(kTasks, [name](const TaskName& task) { return task.name == name; });
    return found == kTasks.end() ? nullptr : found;
}

[[nodiscard]] core::TaskSettings& settingsOfTask(core::CorrectionSettings& settings,
                                                 core::CorrectionTask task) {
    switch (task) {
    case core::CorrectionTask::Mentions:
        return settings.mentions;
    case core::CorrectionTask::CommonErrors:
        return settings.commonErrors;
    case core::CorrectionTask::LineBreak:
        return settings.lineBreak;
    default:
        return settings.capitalization;
    }
}

/// `Script[-language[-COUNTRY]]`: four letters with a capital, then a language
/// of two or three small ones, then a country of two capitals.
[[nodiscard]] bool isPatternCode(std::string_view code) {
    const auto isUpper = [](char c) { return c >= 'A' && c <= 'Z'; };
    const auto isLower = [](char c) { return c >= 'a' && c <= 'z'; };
    const std::vector<std::string_view> parts = splitOn(code, '-');
    if (parts.size() > 3 || parts[0].size() != 4 || !isUpper(parts[0][0]) ||
        !std::ranges::all_of(parts[0].substr(1), isLower)) {
        return false;
    }
    const auto isLanguage = [&](std::string_view language) {
        return (language.size() == 2 || language.size() == 3) &&
               std::ranges::all_of(language, isLower);
    };
    if (parts.size() >= 2 && !isLanguage(parts[1])) {
        return false;
    }
    return parts.size() < 3 || (parts[2].size() == 2 && std::ranges::all_of(parts[2], isUpper));
}

/// A name as `--enable` and `--disable` take it: `type:name`, or the name alone.
struct NameWritten {
    std::optional<core::PatternKind> kind;
    std::string name;
};

[[nodiscard]] NameWritten nameWritten(const std::string& written) {
    const std::size_t colon = written.find(':');
    if (colon != std::string::npos) {
        const std::string_view type = std::string_view{written}.substr(0, colon);
        for (const core::PatternKind kind : {core::PatternKind::CommonError,
                                             core::PatternKind::Capitalization,
                                             core::PatternKind::HearingImpaired,
                                             core::PatternKind::LineBreak}) {
            if (core::fileExtensionOf(kind) == type) {
                return {.kind = kind, .name = written.substr(colon + 1)};
            }
        }
    }
    return {.kind = std::nullopt, .name = written};
}

/// Switches the patterns `written` names on or off, or says why it names none.
[[nodiscard]] std::expected<void, std::string>
switchPattern(const std::string& written,
              bool enabled,
              std::string_view option,
              const std::vector<const TaskName*>& tasks,
              const std::string& code,
              const core::PatternCatalogue& catalogue,
              core::CorrectionSettings& settings) {
    const NameWritten wanted = nameWritten(written);

    struct Found {
        core::PatternKind kind;
        const core::CorrectionPattern* pattern;
    };

    std::vector<Found> found;
    for (const TaskName* task : tasks) {
        if (wanted.kind && *wanted.kind != task->kind) {
            continue;
        }
        for (const core::CorrectionPattern* pattern : catalogue.cascade(task->kind, code)) {
            if (pattern->name == wanted.name) {
                found.push_back({.kind = task->kind, .pattern = pattern});
            }
        }
    }

    if (found.empty()) {
        return std::unexpected{core::substitute(
            // TRANSLATORS: %1 is --enable or --disable, %2 what the user typed, %3 a pattern code.
            core::translate("%1: \"%2\" names no pattern of the tasks given, under the code %3"),
            {option, written, code})};
    }
    for (const Found& one : found) {
        if (one.kind != found.front().kind) {
            return std::unexpected{core::substitute(
                // TRANSLATORS: %1 is --enable or --disable, %2 what the user typed. The types
                // are values the user types, as written.
                core::translate("%1: \"%2\" names patterns of several types: write type:name, "
                                "with the type one of common-error, capitalization, "
                                "hearing-impaired, line-break"),
                {option, written})};
        }
    }

    for (const Found& one : found) {
        // The two records of the scan are not compiled patterns: they command the
        // scan of brackets and parentheses, which has settings of its own.
        if (core::isScanOnlyPattern(*one.pattern)) {
            (one.pattern->name == "Sound in brackets" ? settings.soundInBrackets
                                                      : settings.soundInParentheses) = enabled;
            continue;
        }
        std::erase_if(settings.patternActivations, [&](const core::PatternActivation& activation) {
            return activation.kind == one.kind && activation.code == one.pattern->code &&
                   activation.name == one.pattern->name;
        });
        settings.patternActivations.push_back({.kind = one.kind,
                                               .code = one.pattern->code,
                                               .name = one.pattern->name,
                                               .enabled = enabled});
    }
    return {};
}

/// Whether the task plays anything under `settings`.
///
/// **The line-break always does**: its patterns only weigh where to cut, and
/// the cut is made, balanced, with none — which is what `Zyyy` gives, the cascade
/// that holds no line-break pattern. « Nothing to do » is for the tasks whose
/// patterns *are* the correction.
[[nodiscard]] bool hasWork(const TaskName& task,
                           const core::PatternCatalogue& catalogue,
                           const core::CorrectionSettings& settings,
                           const std::string& code) {
    if (task.task == core::CorrectionTask::LineBreak) {
        return true;
    }
    if (task.task == core::CorrectionTask::Mentions &&
        (settings.soundInBrackets || settings.soundInParentheses)) {
        return true;
    }
    const std::vector<const core::CorrectionPattern*> active =
        core::activePatterns(catalogue, task.kind, code, settings);
    return std::ranges::any_of(active, [](const core::CorrectionPattern* pattern) {
        return !core::isScanOnlyPattern(*pattern);
    });
}

/// The pattern is named as it is shown, translated (D6); built under `SourceLanguage` for the
/// JSON record, the name is the file's English one with the rest of the sentence.
[[nodiscard]] std::string abandonedSentence(const core::PatternFailure& failure,
                                            const std::vector<std::size_t>& subtitles) {
    if (failure.text && *failure.text < subtitles.size()) {
        return core::substitute(
            // TRANSLATORS: %1 is the name of a pattern, %2 why it gave up, %3 a subtitle number.
            core::translate("pattern \"%1\" (%2) was not applied to subtitle %3"),
            {core::patternName(failure.name),
             core::reasonOf(failure.kind),
             std::to_string(subtitles[*failure.text] + 1)});
    }
    // TRANSLATORS: %1 is the name of a pattern, %2 why it gave up.
    return core::substitute(core::translate("pattern \"%1\" (%2) was not applied"),
                            {core::patternName(failure.name), core::reasonOf(failure.kind)});
}

/// The tasks `--tasks` names, once each, or the first word that is none.
[[nodiscard]] std::expected<std::vector<const TaskName*>, std::string>
tasksOf(const std::string& list) {
    std::vector<const TaskName*> tasks;
    for (const std::string_view name : splitList(list)) {
        const TaskName* task = taskNamed(name);
        if (task == nullptr) {
            return std::unexpected{core::substitute(
                // TRANSLATORS: %1 is what the user typed; the tasks are typed as written.
                core::translate("--tasks: \"%1\" is not a task: expected mentions, join-words, "
                                "split-words, common-errors, capitalization or line-break"),
                {name})};
        }
        if (std::ranges::find(tasks, task) == tasks.end()) {
            tasks.push_back(task);
        }
    }
    return tasks;
}

/// Why `--code` cannot be honoured, or nothing. It is required as soon as a task
/// reads patterns, and is a mistake when none does.
[[nodiscard]] std::optional<std::string> codeRefusal(const CorrectionOptions& options,
                                                     bool readsPatterns) {
    if (!readsPatterns) {
        return options.code.empty()
                   ? std::nullopt
                   : std::optional<std::string>{core::translate(
                         "--code is for the tasks that read patterns, and none was asked for")};
    }
    if (options.code.empty()) {
        return core::substitute(
            // TRANSLATORS: %1 is the list of tasks the user gave.
            core::translate("--code is required by the tasks that read patterns: %1"),
            {options.tasks});
    }
    if (!isPatternCode(options.code)) {
        // TRANSLATORS: %1 is what the user typed.
        return core::substitute(core::translate("--code: \"%1\" is not a pattern code: expected "
                                                "Script[-language[-COUNTRY]], like Zyyy, Latn or "
                                                "Latn-en-US"),
                                {options.code});
    }
    return std::nullopt;
}

/// Reads `--classes` into `settings`, or says which word is no class.
[[nodiscard]] std::optional<std::string> classesInto(const std::string& classes,
                                                     core::CorrectionSettings& settings) {
    if (classes.empty()) {
        return std::nullopt;
    }
    settings.human = false;
    settings.ocr = false;
    for (const std::string_view name : splitList(classes)) {
        if (name == "human") {
            settings.human = true;
        } else if (name == "ocr") {
            settings.ocr = true;
        } else {
            return core::substitute(
                // TRANSLATORS: %1 is what the user typed; human and ocr are typed as written.
                core::translate(
                    "--classes: \"%1\" is not a class: expected human, ocr or human,ocr"),
                {name});
        }
    }
    return std::nullopt;
}

/// More lines than a subtitle can hold on a screen: a typo, not a wish. The
/// settings file's reader holds the same bound.
constexpr double kMostLines = core::kMostLineBreakLines;

/// Reads the dictionary's language and the two tasks that need it into `settings`,
/// or says why `--language` cannot be honoured. **Required with a task that
/// checks words**, and a mistake without one: a language nothing uses is an
/// omission, not a preference.
[[nodiscard]] std::optional<std::string> languageInto(const CorrectionOptions& options,
                                                      const std::vector<const TaskName*>& tasks,
                                                      core::CorrectionSettings& settings) {
    bool join = false;
    bool split = false;
    for (const TaskName* task : tasks) {
        join = join || task->name == "join-words";
        split = split || task->name == "split-words";
    }
    if (!join && !split) {
        return options.language.empty()
                   ? std::nullopt
                   : std::optional<std::string>{
                         core::translate("--language is for the tasks join-words and split-words, "
                                         "which were not asked for")};
    }
    if (options.language.empty()) {
        return core::substitute(
            // TRANSLATORS: %1 is the list of tasks the user gave.
            core::translate("--language is required by the tasks that check words: %1"),
            {options.tasks});
    }
    if (!core::isValidSpellLanguage(options.language)) {
        // TRANSLATORS: %1 is what the user typed.
        return core::substitute(core::translate("--language: \"%1\" is not a language: expected "
                                                "a locale code, like fr, en_US or sr@Latn"),
                                {options.language});
    }
    settings.joinSplitEnabled = true;
    settings.joinWords = join;
    settings.splitWords = split;
    settings.spellLanguage = options.language;
    return std::nullopt;
}

/// A number greater than zero, written as digits with an optional fraction.
[[nodiscard]] std::optional<double> positiveNumber(const std::string& text) {
    double value = 0;
    const char* const end = text.data() + text.size();
    const auto [stopped, error] = std::from_chars(text.data(), end, value);
    if (text.empty() || error != std::errc{} || stopped != end || !(value > 0.0) ||
        !std::isfinite(value)) {
        return std::nullopt;
    }
    return value;
}

/// One bound of the skip gate: a number, or `off`, or nothing — the default, which
/// is the bound of the break itself.
struct SkipBound {
    bool on = true;
    double value = 0;
};

[[nodiscard]] std::expected<SkipBound, std::string>
skipBoundOf(const std::string& written, std::string_view option, double fallback) {
    if (written.empty()) {
        return SkipBound{.on = true, .value = fallback};
    }
    if (written == "off") {
        return SkipBound{.on = false, .value = fallback};
    }
    const std::optional<double> value = positiveNumber(written);
    if (!value) {
        return std::unexpected{core::substitute(
            // TRANSLATORS: %1 is an option, %2 what the user typed; « off » is typed as written.
            core::translate(
                "%1: \"%2\" is not a bound: expected a number greater than zero, or off"),
            {option, written})};
    }
    return SkipBound{.on = true, .value = *value};
}

/// Reads the four options of the line break into `settings`, or says which cannot
/// be honoured. **In characters, and without a default length**: the 24 of Gaupol
/// is in ems, and copied as letters it would cut nearly everything, silently.
/// `defaultLines` is what an empty `--max-lines` means — the settings file's, else
/// Gaupol's — and the skip gate's lines follow it when `--skip-lines` is empty.
[[nodiscard]] std::optional<std::string> lineBreakInto(const CorrectionOptions& options,
                                                       bool asked,
                                                       int defaultLines,
                                                       core::CorrectionSettings& settings) {
    const std::array<std::pair<std::string_view, const std::string*>, 4> given{{
        {"--max-length", &options.maxLength},
        {"--max-lines", &options.maxLines},
        {"--skip-length", &options.skipLength},
        {"--skip-lines", &options.skipLines},
    }};
    if (!asked) {
        for (const auto& [name, written] : given) {
            if (!written->empty()) {
                return core::substitute(
                    // TRANSLATORS: %1 is an option of the task line-break.
                    core::translate("%1 is for the task line-break, which was not asked for"),
                    {name});
            }
        }
        return std::nullopt;
    }

    if (options.maxLength.empty()) {
        return std::string{core::translate("--max-length is required by the task line-break: "
                                           "Gaupol's 24 is a width in ems, and has no value in "
                                           "characters")};
    }
    const std::optional<double> length = positiveNumber(options.maxLength);
    if (!length) {
        // TRANSLATORS: %1 is what the user typed.
        return core::substitute(core::translate("--max-length: \"%1\" is not a length: expected a "
                                                "number of characters greater than zero"),
                                {options.maxLength});
    }
    int lines = defaultLines;
    if (!options.maxLines.empty()) {
        const std::optional<double> written = positiveNumber(options.maxLines);
        if (!written || *written != std::floor(*written) || *written > kMostLines) {
            // TRANSLATORS: %1 is what the user typed.
            return core::substitute(core::translate("--max-lines: \"%1\" is not a number of lines: "
                                                    "expected a whole number greater than zero"),
                                    {options.maxLines});
        }
        lines = static_cast<int>(*written);
    }
    const std::expected<SkipBound, std::string> skipLength =
        skipBoundOf(options.skipLength, "--skip-length", *length);
    if (!skipLength) {
        return skipLength.error();
    }
    const std::expected<SkipBound, std::string> skipLines =
        skipBoundOf(options.skipLines, "--skip-lines", lines);
    if (!skipLines) {
        return skipLines.error();
    }
    if (skipLines->value != std::floor(skipLines->value)) {
        // TRANSLATORS: %1 is what the user typed; « off » is typed as written.
        return core::substitute(
            core::translate("--skip-lines: \"%1\" is not a number of lines: "
                            "expected a whole number greater than zero, or off"),
            {options.skipLines});
    }

    settings.lineBreakMaxLength = *length;
    settings.lineBreakMaxLines = lines;
    settings.lineBreakInEms = false;
    settings.lineBreakSkipOnLength = skipLength->on;
    settings.lineBreakSkipMaxLength = skipLength->value;
    settings.lineBreakSkipOnLines = skipLines->on;
    settings.lineBreakSkipMaxLines = static_cast<int>(skipLines->value);
    return std::nullopt;
}

/// Whether the settings file turns both classes off: a state `--classes` cannot
/// write, and which would leave common-errors nothing to do.
[[nodiscard]] bool fileClassesBothOff(const core::CorrectionSettings& shared) {
    return !shared.human && !shared.ocr;
}

/// Applies `--enable` and `--disable`, or says why a name cannot be honoured.
[[nodiscard]] std::optional<std::string> switchesInto(const CorrectionOptions& options,
                                                      const std::vector<const TaskName*>& tasks,
                                                      const core::PatternCatalogue& catalogue,
                                                      core::CorrectionSettings& settings) {
    for (const std::string& written : options.enable) {
        if (std::ranges::find(options.disable, written) != options.disable.end()) {
            return core::substitute(
                // TRANSLATORS: %1 is the name of a pattern, as the user typed it.
                core::translate("\"%1\" is given to --enable and to --disable: choose one"),
                {written});
        }
        if (std::expected<void, std::string> done =
                switchPattern(written, true, "--enable", tasks, options.code, catalogue, settings);
            !done) {
            return std::move(done.error());
        }
    }
    for (const std::string& written : options.disable) {
        if (std::expected<void, std::string> done = switchPattern(
                written, false, "--disable", tasks, options.code, catalogue, settings);
            !done) {
            return std::move(done.error());
        }
    }
    return std::nullopt;
}

/// What a proposed correction becomes in the list of changes: the text after it,
/// nothing for a subtitle that goes, the empty text for one that is kept empty.
[[nodiscard]] std::optional<std::string> afterOf(const core::ProposedCorrection& correction,
                                                 bool removeBlankSubtitles) {
    if (correction.proposed) {
        return correction.proposed;
    }
    if (removeBlankSubtitles) {
        return std::nullopt;
    }
    return std::string{};
}

} // namespace

std::expected<core::CorrectionSettings, std::string>
correctionSettingsOf(const CorrectionOptions& options, const core::PatternCatalogue& catalogue) {
    return correctionSettingsOf(options, catalogue, core::CorrectionSettings{});
}

std::expected<core::CorrectionSettings, std::string>
correctionSettingsOf(const CorrectionOptions& options,
                     const core::PatternCatalogue& catalogue,
                     const core::CorrectionSettings& shared) {
    // Fresh, and not a copy of `shared`: tasks, codes, activations, length,
    // language and the skip gate never come from the file (spec 15, S8).
    core::CorrectionSettings settings;
    // **Nothing is ticked beforehand**: the defaults of the window are for a
    // person looking at a list, and a script that named no task asked for none.
    settings.mentions.enabled = false;
    settings.commonErrors.enabled = false;
    settings.capitalization.enabled = false;

    const std::expected<std::vector<const TaskName*>, std::string> asked = tasksOf(options.tasks);
    if (!asked) {
        return std::unexpected{asked.error()};
    }
    // The tasks that play patterns, which is where a code, a name and a limit apply.
    std::vector<const TaskName*> tasks;
    std::ranges::copy_if(*asked, std::back_inserter(tasks), [](const TaskName* task) {
        return task->readsPatterns;
    });
    if (std::optional<std::string> refused = codeRefusal(options, !tasks.empty())) {
        return std::unexpected{*std::move(refused)};
    }
    if (std::optional<std::string> refused = languageInto(options, *asked, settings)) {
        return std::unexpected{*std::move(refused)};
    }
    if (std::optional<std::string> refused = classesInto(options.classes, settings)) {
        return std::unexpected{*std::move(refused)};
    }
    if (options.classes.empty() && !fileClassesBothOff(shared)) {
        settings.human = shared.human;
        settings.ocr = shared.ocr;
    }
    const bool breaks = std::ranges::any_of(
        tasks, [](const TaskName* task) { return task->task == core::CorrectionTask::LineBreak; });
    if (std::optional<std::string> refused =
            lineBreakInto(options, breaks, shared.lineBreakMaxLines, settings)) {
        return std::unexpected{*std::move(refused)};
    }

    for (const TaskName* task : tasks) {
        core::TaskSettings& chosen = settingsOfTask(settings, task->task);
        chosen.enabled = true;
        chosen.code = options.code;
    }

    if (std::optional<std::string> refused = switchesInto(options, tasks, catalogue, settings)) {
        return std::unexpected{*std::move(refused)};
    }

    for (const TaskName* task : tasks) {
        if (!hasWork(*task, catalogue, settings, options.code)) {
            return std::unexpected{core::substitute(
                // TRANSLATORS: %1 is a task, as typed; %2 a pattern code.
                core::translate("%1: no pattern is active under the code %2; switch one on with "
                                "--enable"),
                {task->name, options.code})};
        }
    }

    settings.removeBlankSubtitles = !options.keepBlankSubtitles && shared.removeBlankSubtitles;
    return settings;
}

ExitCode correctIn(core::FileSystem& files,
                   const std::vector<std::string>& paths,
                   const std::optional<core::Encoding>& reading,
                   const core::PatternCatalogue& catalogue,
                   const core::CorrectionSettings& settings,
                   const std::optional<Range>& range,
                   const Destination& destination,
                   const Reporter& reporter,
                   const std::optional<Pairing>& pairing,
                   const core::SpellChecker* spellChecker,
                   const Json& settingsFile) {
    // One engine for the whole run: it is read, never changed.
    const auto engine = std::make_shared<const core::IcuPatternEngine>();
    const core::CharacterLineMeasure measure;

    const ChangingOperation correct =
        [&catalogue, &settings, &measure, engine, spellChecker, &settingsFile](
            core::Session& session, const Request& request) -> OperationOutcome {
        const core::CorrectionTarget target{.project = &session.project(),
                                            .selection = request.selection,
                                            .document = request.document};
        core::CorrectionProposal proposal = core::proposeCorrections(
            *engine, catalogue, settings, measure, std::span{&target, 1}, spellChecker);

        // The subtitles looked at, in the order a failure's `text` counts them.
        std::vector<std::size_t> subtitles;
        for (const core::SubtitleIndex index : request.selection.indices()) {
            subtitles.push_back(index.value());
        }
        std::vector<Warning> warnings;
        warnings.reserve(proposal.failures.size());
        for (const core::PatternFailure& failure : proposal.failures) {
            // Worded twice: translated for standard error, English for the record (D3).
            warnings.push_back({.kind = "pattern-failed",
                                .detail = abandonedSentence(failure, subtitles),
                                .english = core::inSourceLanguage(
                                    [&] { return abandonedSentence(failure, subtitles); })});
        }

        std::optional<std::vector<TextChange>> changes;
        if (request.changes) {
            changes.emplace();
            for (const core::ProposedCorrection& one : proposal.corrections) {
                TextChange change{.subtitle = one.index.value() + 1,
                                  .document = one.document,
                                  .before = one.original};
                change.after = afterOf(one, settings.removeBlankSubtitles);
                changes->push_back(std::move(change));
            }
        }

        std::vector<core::AppliedCorrection> composed =
            core::applyCorrections(proposal.corrections, settings.removeBlankSubtitles);
        // Read before the commands are moved: the tally is of what they will do.
        const core::CorrectionTally tally = core::tallyOf(composed);
        for (core::AppliedCorrection& one : composed) {
            session.apply(std::move(one.command));
        }

        return OperationResult{.sentence = core::noticeOfCorrection(tally.corrected, tally.removed),
                               .counts = {{"corrected", static_cast<std::int64_t>(tally.corrected)},
                                          {"removed", static_cast<std::int64_t>(tally.removed)}},
                               .changes = std::move(changes),
                               .fields = {{"settings", settingsFile}},
                               .warnings = std::move(warnings)};
    };

    return rewriteAll(
        files, paths, reading, destination, reporter, Verb::Corrected, correct, range, pairing);
}

std::expected<CorrectionRun, std::string>
prepareCorrection(core::FileSystem& files,
                  const Reporter& reporter,
                  const CorrectionOptions& options,
                  const std::filesystem::path& installedPatterns,
                  const std::filesystem::path& userPatterns,
                  const SpellProviderFactory& makeProvider) {
    return prepareCorrection(files,
                             reporter,
                             options,
                             installedPatterns,
                             userPatterns,
                             makeProvider,
                             core::CorrectionSettings{});
}

std::expected<CorrectionRun, std::string>
prepareCorrection(core::FileSystem& files,
                  const Reporter& reporter,
                  const CorrectionOptions& options,
                  const std::filesystem::path& installedPatterns,
                  const std::filesystem::path& userPatterns,
                  const SpellProviderFactory& makeProvider,
                  const core::CorrectionSettings& shared) {
    core::PatternCatalogue catalogue =
        core::readPatternCatalogue(files, installedPatterns, userPatterns);
    for (const core::PatternDiagnostic& diagnostic : catalogue.diagnostics()) {
        // TRANSLATORS: %1 is what was wrong with a pattern file.
        reporter.say(
            1, core::substitute(core::translate("patterns: %1"), {core::describe(diagnostic)}));
    }

    std::expected<core::CorrectionSettings, std::string> settings =
        correctionSettingsOf(options, catalogue, shared);
    if (!settings) {
        return std::unexpected{settings.error()};
    }
    // A nonsense the file holds is not taken silently: it was put there by hand.
    // (A number of lines out of bounds never gets this far: the file's reader
    // refuses it, and says so.)
    if (settings->commonErrors.enabled && options.classes.empty() && fileClassesBothOff(shared)) {
        reporter.say(1,
                     core::translate("settings: correction.human and correction.ocr are both "
                                     "false, ignored"));
    }

    CorrectionRun run{.catalogue = std::move(catalogue), .settings = *std::move(settings)};
    if (run.settings.joinSplitEnabled) {
        run.provider = makeProvider();
        std::expected<core::SpellChecker, core::NoDictionary> opened =
            core::openSpellChecker(*run.provider,
                                   run.settings.spellLanguage,
                                   files,
                                   core::spellReplacementFile({}, run.settings.spellLanguage));
        if (!opened) {
            return std::unexpected{core::noDictionaryFor(opened.error().language)};
        }
        run.spellChecker.emplace(std::move(*opened));
    }
    return run;
}

} // namespace subedit::cli
