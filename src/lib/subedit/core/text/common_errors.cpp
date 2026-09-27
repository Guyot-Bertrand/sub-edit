#include <subedit/core/text/common_errors.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/replacement_template.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace subedit::core {

namespace {

/// A pattern ready to be applied: compiled, and its replacement read.
struct Prepared {
    const CorrectionPattern* pattern;
    bool repeat;
    std::unique_ptr<PatternMatcher> matcher;
    ReplacementTemplate replacement;
};

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

/// The offset of the character after the one at `at`.
[[nodiscard]] std::size_t nextCharacter(const std::string& text, std::size_t at) {
    constexpr unsigned kContinuationMask = 0xC0U;
    constexpr unsigned kContinuation = 0x80U;
    ++at;
    while (at < text.size() &&
           (static_cast<unsigned char>(text[at]) & kContinuationMask) == kContinuation)
        ++at;
    return at;
}

/// Why one pass over a text gave up.
struct PassFailure {
    FailureKind kind;
    std::string detail;
};

/// `Finder.replace_all` of Gaupol, on `text`: how many replacements it made.
///
/// **Each match is searched in the text as the previous replacement left it**,
/// from the end of what was written — which is what a lookbehind sees, and what
/// a global « replace all » would not give. After an empty match the next
/// character is skipped, once, so that a pattern matching nothing does not find
/// the same place for ever.
[[nodiscard]] std::expected<std::size_t, PassFailure> replaceAll(const Prepared& one,
                                                                 std::string& text) {
    std::size_t pos = 0;
    std::optional<MatchSpan> previous;
    std::size_t count = 0;

    while (true) {
        const std::expected<std::optional<Match>, SearchFailure> searched =
            one.matcher->find(text, pos);
        if (!searched) {
            // The one way a search fails — pattern_engine.hpp's SearchFailure.
            return std::unexpected{
                PassFailure{.kind = FailureKind::TimedOut, .detail = "the search was given up"}};
        }
        if (!searched->has_value())
            return count;

        const Match& match = **searched;
        const MatchSpan span = match.whole();
        if (previous == span && span.start == pos && span.end == pos) {
            if (pos >= text.size())
                return count;
            pos = nextCharacter(text, pos);
            continue;
        }

        const std::string written = one.replacement.expandedFor(text, match);
        text.replace(span.start, span.end - span.start, written);
        pos = span.start + written.size();
        previous = span;
        if (span.start == span.end)
            previous = MatchSpan{.start = pos, .end = pos};
        ++count;

        if (text.size() > kMaxTextBytes) {
            return std::unexpected{
                PassFailure{.kind = FailureKind::TooLong, .detail = "the text outgrew a subtitle"}};
        }
    }
}

/// One pattern on one text: `Repeat` until the text no longer changes.
[[nodiscard]] std::optional<PassFailure> applyPattern(const Prepared& one, std::string& text) {
    std::expected<std::size_t, PassFailure> count = replaceAll(one, text);
    for (int passes = 1; one.repeat && count && *count > 0; ++passes) {
        if (passes > kMaxRepeatPasses) {
            return PassFailure{.kind = FailureKind::TooManyPasses,
                               .detail = "the text still changed after " +
                                         std::to_string(kMaxRepeatPasses) + " passes"};
        }
        const std::string before = text;
        count = replaceAll(one, text);
        if (text == before)
            break;
    }
    if (!count)
        return count.error();
    return std::nullopt;
}

/// Compiles the patterns, reporting the ones that do not compile.
[[nodiscard]] std::vector<Prepared> prepare(const PatternEngine& engine,
                                            std::span<const CorrectionPattern* const> patterns,
                                            std::vector<PatternFailure>& failures) {
    std::vector<Prepared> prepared;
    for (const CorrectionPattern* pattern : patterns) {
        const auto* fields = std::get_if<CommonErrorFields>(&pattern->fields);
        if (fields == nullptr)
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
                                    .repeat = fields->repeat,
                                    .matcher = std::move(*matcher),
                                    .replacement = std::move(*replacement)});
    }
    return prepared;
}

} // namespace

CorrectedTexts correctCommonErrors(const PatternEngine& engine,
                                   std::span<const CorrectionPattern* const> patterns,
                                   std::span<const std::string> texts) {
    CorrectedTexts result;
    const std::vector<Prepared> prepared = prepare(engine, patterns, result.failures);

    result.texts.reserve(texts.size());
    for (std::size_t index = 0; index < texts.size(); ++index) {
        std::string text = texts[index];
        for (const Prepared& one : prepared) {
            // On a copy: a pattern given up halfway leaves the text as it was.
            std::string attempt = text;
            if (const std::optional<PassFailure> gaveUp = applyPattern(one, attempt)) {
                result.failures.push_back(
                    failureOf(gaveUp->kind, *one.pattern, index, gaveUp->detail));
            } else {
                text = std::move(attempt);
            }
        }
        result.texts.push_back(std::move(text));
    }
    return result;
}

} // namespace subedit::core
