#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/hearing_impaired.hpp>
#include <subedit/core/text/hearing_impaired_correction.hpp>
#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/replacement_template.hpp>
#include <subedit/core/text/utf8.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::core {

namespace {

[[nodiscard]] PatternFailure failureOf(FailureKind kind,
                                       const CorrectionPattern& pattern,
                                       std::optional<std::size_t> text,
                                       std::string detail) {
    return PatternFailure{.kind = kind,
                          .code = pattern.code,
                          .rank = pattern.rank,
                          .name = pattern.name,
                          .text = text,
                          .detail = std::move(detail)};
}

struct PassFailure {
    FailureKind kind;
    std::string detail;
};

struct PassOutcome {
    std::string text;
    bool changed = false;
};

/// One pass of `Finder.replace_all`, not repeated — `remove_hearing_impaired`
/// of Gaupol never re-runs a pass, unlike `correct_common_errors`.
[[nodiscard]] std::expected<PassOutcome, PassFailure>
replaceOnce(PatternMatcher& matcher, const ReplacementTemplate& replacement, std::string text) {
    std::size_t pos = 0;
    std::optional<MatchSpan> previous;
    bool changed = false;

    while (true) {
        const std::expected<std::optional<Match>, SearchFailure> searched = matcher.find(text, pos);
        if (!searched) {
            return std::unexpected{
                PassFailure{.kind = FailureKind::TimedOut, .detail = "the search was given up"}};
        }
        if (!searched->has_value())
            break;

        const Match& match = **searched;
        const MatchSpan span = match.whole();
        if (previous == span && span.start == pos && span.end == pos) {
            if (pos >= text.size())
                break;
            pos = nextCodePoint(text, pos);
            continue;
        }

        const std::string written = replacement.expandedFor(text, match);
        text.replace(span.start, span.end - span.start, written);
        pos = span.start + written.size();
        previous = span;
        if (span.start == span.end)
            previous = MatchSpan{.start = pos, .end = pos};
        changed = true;

        if (text.size() > kMaxTextBytes) {
            return std::unexpected{
                PassFailure{.kind = FailureKind::TooLong, .detail = "the text outgrew a subtitle"}};
        }
    }
    return PassOutcome{.text = std::move(text), .changed = changed};
}

struct Prepared {
    const CorrectionPattern* pattern;
    std::unique_ptr<PatternMatcher> matcher;
    ReplacementTemplate replacement;
};

/// The names of the two records the scan of ADR 0017 alone plays — giving
/// them to the engine here would run their expression a second time, on text
/// the scan already resolved.
[[nodiscard]] bool isScanOnly(const CorrectionPattern& pattern) {
    return pattern.name == "Sound in brackets" || pattern.name == "Sound in parentheses";
}

[[nodiscard]] std::vector<Prepared> prepare(const PatternEngine& engine,
                                            std::span<const CorrectionPattern* const> patterns,
                                            std::vector<PatternFailure>& failures) {
    std::vector<Prepared> prepared;
    for (const CorrectionPattern* pattern : patterns) {
        const auto* fields = std::get_if<HearingImpairedFields>(&pattern->fields);
        if (fields == nullptr || isScanOnly(*pattern))
            continue;

        std::expected<std::unique_ptr<PatternMatcher>, CompileError> matcher =
            engine.compile(pattern->expression, pattern->flags);
        if (!matcher) {
            failures.push_back(failureOf(matcher.error().kind == CompileFailure::Untranslatable
                                             ? FailureKind::Untranslatable
                                             : FailureKind::CompileError,
                                         *pattern,
                                         std::nullopt,
                                         matcher.error().reason));
            continue;
        }

        const PatternMatcher& compiled = **matcher;
        const auto groupNamed = [&compiled](std::string_view name) {
            return compiled.groupNumber(name);
        };
        std::expected<ReplacementTemplate, TemplateError> replacement =
            ReplacementTemplate::parse(fields->replacement, compiled.groupCount(), groupNamed);
        if (!replacement) {
            failures.push_back(failureOf(FailureKind::InvalidReplacement,
                                         *pattern,
                                         std::nullopt,
                                         replacement.error().reason));
            continue;
        }
        prepared.push_back(Prepared{.pattern = pattern,
                                    .matcher = std::move(*matcher),
                                    .replacement = std::move(*replacement)});
    }
    return prepared;
}

struct BuiltIn {
    std::unique_ptr<PatternMatcher> matcher;
    ReplacementTemplate replacement;
};

/// Gaupol's `_remove_leftover_hi`, seven passes in its own order: edge
/// spaces, doubled spaces, lines with nothing alphanumeric, empty lines, a
/// space owed after a dialogue dash, a dash dropped where the other line
/// carries none, a dash dropped from a subtitle of a single line.
[[nodiscard]] std::vector<BuiltIn> builtInCleanups(const PatternEngine& engine) {
    struct Rule {
        const char* expression;
        const char* replacement;
    };

    static constexpr std::array<Rule, 7> kRules{{
        {.expression = R"((^\s+|\s+$))", .replacement = ""},
        {.expression = R"( {2,})", .replacement = " "},
        {.expression = R"(^\W*$)", .replacement = ""},
        {.expression = R"((^\n|\n$))", .replacement = ""},
        {.expression = R"(^([-\x{2013}\x{2014}])(\S))", .replacement = R"(\1 \2)"},
        {.expression = R"(^[-\x{2013}\x{2014}] (.*?^[^-\x{2013}\x{2014}]))",
         .replacement = R"(\1)"},
        {.expression = R"(\A[-\x{2013}\x{2014}] ([^\n]*)\Z)", .replacement = R"(\1)"},
    }};

    // Fixed, hand-written expressions: refusing one would be a bug in it, not
    // in data, so `.value()` is right where the rest of this file reports a
    // failure instead — a mistake here is meant to stop a test, loudly.
    std::vector<BuiltIn> cleanups;
    cleanups.reserve(kRules.size());
    for (const Rule& rule : kRules) {
        std::unique_ptr<PatternMatcher> matcher =
            engine.compile(rule.expression, PatternFlags{.dotAll = true, .multiline = true})
                .value();
        const PatternMatcher& compiled = *matcher;
        ReplacementTemplate replacement =
            ReplacementTemplate::parse(rule.replacement,
                                       compiled.groupCount(),
                                       [](std::string_view) { return std::nullopt; })
                .value();
        cleanups.push_back(
            BuiltIn{.matcher = std::move(matcher), .replacement = std::move(replacement)});
    }
    return cleanups;
}

} // namespace

HearingImpairedCorrection correctHearingImpaired(const PatternEngine& engine,
                                                 std::span<const CorrectionPattern* const> patterns,
                                                 std::span<const std::string> texts,
                                                 SubtitleFormat format) {
    HearingImpairedCorrection result;
    const std::vector<Prepared> prepared = prepare(engine, patterns, result.failures);
    const std::vector<BuiltIn> cleanups = builtInCleanups(engine);

    result.texts.reserve(texts.size());
    for (std::size_t index = 0; index < texts.size(); ++index) {
        const std::optional<std::string> swept = withoutHearingImpaired(texts[index], format);
        if (!swept) {
            result.texts.emplace_back(std::nullopt);
            continue;
        }

        std::string current = *swept;
        bool cascadeChanged = false;
        for (const Prepared& one : prepared) {
            std::expected<PassOutcome, PassFailure> outcome =
                replaceOnce(*one.matcher, one.replacement, current);
            if (!outcome) {
                result.failures.push_back(
                    failureOf(outcome.error().kind, *one.pattern, index, outcome.error().detail));
                continue;
            }
            cascadeChanged = cascadeChanged || outcome->changed;
            current = std::move(outcome->text);
        }

        if (cascadeChanged) {
            for (const BuiltIn& cleanup : cleanups) {
                std::expected<PassOutcome, PassFailure> outcome =
                    replaceOnce(*cleanup.matcher, cleanup.replacement, current);
                if (outcome)
                    current = std::move(outcome->text);
            }
        }

        if (current.empty())
            result.texts.emplace_back(std::nullopt);
        else
            result.texts.emplace_back(std::move(current));
    }
    return result;
}

} // namespace subedit::core
