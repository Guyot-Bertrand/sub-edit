#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/hearing_impaired.hpp>
#include <subedit/core/text/hearing_impaired_correction.hpp>
#include <subedit/core/text/markup_parser.hpp>
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

/// One pass of `Finder.replace_all`, not repeated — `remove_hearing_impaired`
/// of Gaupol never re-runs a pass, unlike `correct_common_errors`.
///
/// **A `MarkupParser::transform`, as decision D9 asks** — `parser` is rewritten
/// in place, so a caller that wants to keep a text as it was before a pass
/// that failed reads from a fresh copy, the way `common_errors.cpp` does.
[[nodiscard]] std::expected<bool, PassFailure>
replaceOnce(PatternMatcher& matcher, const ReplacementTemplate& replacement, MarkupParser& parser) {
    std::size_t pos = 0;
    std::optional<MatchSpan> previous;
    bool changed = false;

    while (true) {
        const std::expected<std::optional<Match>, SearchFailure> searched =
            matcher.find(parser.visible(), pos);
        if (!searched) {
            return std::unexpected{
                PassFailure{.kind = FailureKind::TimedOut, .detail = "the search was given up"}};
        }
        if (!searched->has_value())
            break;

        const Match& match = **searched;
        const MatchSpan span = match.whole();
        if (previous == span && span.start == pos && span.end == pos) {
            if (pos >= parser.visible().size())
                break;
            pos = nextCodePoint(parser.visible(), pos);
            continue;
        }

        const std::string written = replacement.expandedFor(parser.visible(), match);
        parser.transform(span.start, span.end - span.start, written);
        pos = span.start + written.size();
        previous = span;
        if (span.start == span.end)
            previous = MatchSpan{.start = pos, .end = pos};
        changed = true;

        if (parser.visible().size() > kMaxTextBytes) {
            return std::unexpected{
                PassFailure{.kind = FailureKind::TooLong, .detail = "the text outgrew a subtitle"}};
        }
    }
    return changed;
}

struct Prepared {
    const CorrectionPattern* pattern;
    std::unique_ptr<PatternMatcher> matcher;
    ReplacementTemplate replacement;
};

[[nodiscard]] std::vector<Prepared> prepare(const PatternEngine& engine,
                                            std::span<const CorrectionPattern* const> patterns,
                                            std::vector<PatternFailure>& failures) {
    std::vector<Prepared> prepared;
    for (const CorrectionPattern* pattern : patterns) {
        const auto* fields = std::get_if<HearingImpairedFields>(&pattern->fields);
        if (fields == nullptr || isScanOnlyPattern(*pattern))
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

bool isScanOnlyPattern(const CorrectionPattern& pattern) {
    return pattern.kind() == PatternKind::HearingImpaired &&
           (pattern.name == "Sound in brackets" || pattern.name == "Sound in parentheses");
}

HearingImpairedCorrection correctHearingImpaired(const PatternEngine& engine,
                                                 std::span<const CorrectionPattern* const> patterns,
                                                 std::span<const std::string> texts,
                                                 SubtitleFormat format,
                                                 bool scanBracketsAndParentheses) {
    HearingImpairedCorrection result;
    const std::vector<Prepared> prepared = prepare(engine, patterns, result.failures);
    const std::vector<BuiltIn> cleanups = builtInCleanups(engine);

    result.texts.reserve(texts.size());
    for (std::size_t index = 0; index < texts.size(); ++index) {
        const std::optional<std::string> swept = scanBracketsAndParentheses
                                                     ? withoutHearingImpaired(texts[index], format)
                                                     : std::optional<std::string>{texts[index]};
        if (!swept) {
            result.texts.emplace_back(std::nullopt);
            continue;
        }

        MarkupParser parser{*swept, format};
        bool cascadeChanged = false;
        for (const Prepared& one : prepared) {
            // Reread from what was kept: `MarkupParser` cannot be copied, and a
            // pass given up halfway leaves the text — and its tags — as they
            // were before it, the same rule `common_errors.cpp` follows.
            MarkupParser attempt{parser.text(), format};
            const std::expected<bool, PassFailure> outcome =
                replaceOnce(*one.matcher, one.replacement, attempt);
            if (!outcome) {
                result.failures.push_back(
                    failureOf(outcome.error().kind, *one.pattern, index, outcome.error().detail));
                continue;
            }
            cascadeChanged = cascadeChanged || *outcome;
            parser = std::move(attempt);
        }

        if (cascadeChanged) {
            for (const BuiltIn& cleanup : cleanups) {
                MarkupParser attempt{parser.text(), format};
                const std::expected<bool, PassFailure> outcome =
                    replaceOnce(*cleanup.matcher, cleanup.replacement, attempt);
                if (outcome)
                    parser = std::move(attempt);
            }
        }

        std::string current = parser.text();
        if (current.empty())
            result.texts.emplace_back(std::nullopt);
        else
            result.texts.emplace_back(std::move(current));
    }
    return result;
}

} // namespace subedit::core
