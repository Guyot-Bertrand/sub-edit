#include <subedit/cli/changes.hpp>
#include <subedit/cli/correcting.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/hearing_impaired_correction.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/spell_dictionary.hpp>
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

    const std::string quoted = std::string{option} + ": \"" + written + "\"";
    if (found.empty()) {
        return std::unexpected{quoted + " names no pattern of the tasks given, under the code " +
                               code};
    }
    for (const Found& one : found) {
        if (one.kind != found.front().kind) {
            return std::unexpected{quoted +
                                   " names patterns of several types: write type:name, with the "
                                   "type one of common-error, capitalization, hearing-impaired, "
                                   "line-break"};
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

[[nodiscard]] std::string abandonedSentence(const core::PatternFailure& failure,
                                            const std::vector<std::size_t>& subtitles) {
    std::string sentence = "pattern \"" + failure.name + "\" (" +
                           std::string{core::reasonOf(failure.kind)} + ") was not applied";
    if (failure.text && *failure.text < subtitles.size()) {
        sentence += " to subtitle " + std::to_string(subtitles[*failure.text] + 1);
    }
    return sentence;
}

/// The tasks `--tasks` names, once each, or the first word that is none.
[[nodiscard]] std::expected<std::vector<const TaskName*>, std::string>
tasksOf(const std::string& list) {
    std::vector<const TaskName*> tasks;
    for (const std::string_view name : splitList(list)) {
        const TaskName* task = taskNamed(name);
        if (task == nullptr) {
            return std::unexpected{"--tasks: \"" + std::string{name} +
                                   "\" is not a task: expected mentions, join-words, split-words, "
                                   "common-errors, capitalization or line-break"};
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
                   : std::optional<std::string>{
                         "--code is for the tasks that read patterns, and none was asked for"};
    }
    if (options.code.empty()) {
        return std::string{"--code is required by the tasks that read patterns: "} + options.tasks;
    }
    if (!isPatternCode(options.code)) {
        return "--code: \"" + options.code +
               "\" is not a pattern code: expected Script[-language[-COUNTRY]], "
               "like Zyyy, Latn or Latn-en-US";
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
            return "--classes: \"" + std::string{name} +
                   "\" is not a class: expected human, ocr or human,ocr";
        }
    }
    return std::nullopt;
}

/// More lines than a subtitle can hold on a screen: a typo, not a wish.
constexpr double kMostLines = 1000.0;

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
                         "--language is for the tasks join-words and split-words, which were not "
                         "asked for"};
    }
    if (options.language.empty()) {
        return std::string{"--language is required by the tasks that check words: "} +
               options.tasks;
    }
    if (!core::isValidSpellLanguage(options.language)) {
        return "--language: \"" + options.language +
               "\" is not a language: expected a locale code, like fr, en_US or sr@Latn";
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
        return std::unexpected{std::string{option} + ": \"" + written +
                               "\" is not a bound: expected a number greater than zero, or off"};
    }
    return SkipBound{.on = true, .value = *value};
}

/// Reads the four options of the line break into `settings`, or says which cannot
/// be honoured. **In characters, and without a default length**: the 24 of Gaupol
/// is in ems, and copied as letters it would cut nearly everything, silently.
[[nodiscard]] std::optional<std::string>
lineBreakInto(const CorrectionOptions& options, bool asked, core::CorrectionSettings& settings) {
    const std::array<std::pair<std::string_view, const std::string*>, 4> given{{
        {"--max-length", &options.maxLength},
        {"--max-lines", &options.maxLines},
        {"--skip-length", &options.skipLength},
        {"--skip-lines", &options.skipLines},
    }};
    if (!asked) {
        for (const auto& [name, written] : given) {
            if (!written->empty()) {
                return std::string{name} + " is for the task line-break, which was not asked for";
            }
        }
        return std::nullopt;
    }

    if (options.maxLength.empty()) {
        return std::string{"--max-length is required by the task line-break: Gaupol's 24 is a "
                           "width in ems, and has no value in characters"};
    }
    const std::optional<double> length = positiveNumber(options.maxLength);
    if (!length) {
        return "--max-length: \"" + options.maxLength +
               "\" is not a length: expected a number of characters greater than zero";
    }
    int lines = core::kDefaultLineBreakMaxLines;
    if (!options.maxLines.empty()) {
        const std::optional<double> written = positiveNumber(options.maxLines);
        if (!written || *written != std::floor(*written) || *written > kMostLines) {
            return "--max-lines: \"" + options.maxLines +
                   "\" is not a number of lines: expected a whole number greater than zero";
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
        return "--skip-lines: \"" + options.skipLines +
               "\" is not a number of lines: expected a whole number greater than zero, or off";
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

/// Applies `--enable` and `--disable`, or says why a name cannot be honoured.
[[nodiscard]] std::optional<std::string> switchesInto(const CorrectionOptions& options,
                                                      const std::vector<const TaskName*>& tasks,
                                                      const core::PatternCatalogue& catalogue,
                                                      core::CorrectionSettings& settings) {
    for (const std::string& written : options.enable) {
        if (std::ranges::find(options.disable, written) != options.disable.end()) {
            return "\"" + written + "\" is given to --enable and to --disable: choose one";
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
    const bool breaks = std::ranges::any_of(
        tasks, [](const TaskName* task) { return task->task == core::CorrectionTask::LineBreak; });
    if (std::optional<std::string> refused = lineBreakInto(options, breaks, settings)) {
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
            return std::unexpected{std::string{task->name} +
                                   ": no pattern is active under the code " + options.code +
                                   "; switch one on with --enable"};
        }
    }

    settings.removeBlankSubtitles = !options.keepBlankSubtitles;
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
                   const core::SpellChecker* spellChecker) {
    // One engine for the whole run: it is read, never changed.
    const auto engine = std::make_shared<const core::IcuPatternEngine>();
    const core::CharacterLineMeasure measure;

    const ChangingOperation correct = [&catalogue, &settings, &measure, engine, spellChecker](
                                          core::Session& session,
                                          const Request& request) -> OperationOutcome {
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
            warnings.push_back(
                {.kind = "pattern-failed", .detail = abandonedSentence(failure, subtitles)});
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
                               .warnings = std::move(warnings)};
    };

    return rewriteAll(
        files, paths, reading, destination, reporter, "corrected", correct, range, pairing);
}

} // namespace subedit::cli
