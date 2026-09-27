#include <subedit/core/text/capitalization.hpp>
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/utf8.hpp>

#include <unicode/bytestream.h>
#include <unicode/casemap.h>
#include <unicode/edits.h>
#include <unicode/stringpiece.h>
#include <unicode/uchar.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>

#include <cstddef>
#include <cstdint>
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

/// Whether `point` is one of `\w`'s — a letter, a number, or `_`, the
/// rewriting decision D1 of the spec gives `\w`: `[\p{L}\p{N}_]`.
[[nodiscard]] bool isWordCodePoint(char32_t point) {
    if (point == U'_')
        return true;
    switch (u_charType(static_cast<UChar32>(point))) {
    case U_UPPERCASE_LETTER:
    case U_LOWERCASE_LETTER:
    case U_TITLECASE_LETTER:
    case U_MODIFIER_LETTER:
    case U_OTHER_LETTER:
    case U_DECIMAL_DIGIT_NUMBER:
    case U_LETTER_NUMBER:
    case U_OTHER_NUMBER:
        return true;
    default:
        return false;
    }
}

/// Whether the letter at `i` is blocked from capitalizing by an ellipsis
/// immediately before it, within `[from, i)` — Gaupol's
/// `(?<!\.\.\.)(?<!…)`, a lookbehind that cannot see before `from` because
/// that is where the search Gaupol runs it in was sliced.
[[nodiscard]] bool blockedByEllipsis(std::string_view text, std::size_t from, std::size_t i) {
    if (i >= from + 3 && text[i - 3] == '.' && text[i - 2] == '.' && text[i - 1] == '.')
        return true;
    if (i > from) {
        const std::size_t before = previousCodePoint(text, i);
        if (before >= from && codePointAt(text, before) == U'…')
            return true;
    }
    return false;
}

/// The one code point at `[at, end)`, titlecased — Python's `str.capitalize`
/// on a one-character string, which is a titlecase mapping and not a plain
/// uppercase: `letter_case.cpp` explains why a letter that grows on being
/// capitalised, `ß` into `Ss`, is written that way rather than avoided.
[[nodiscard]] std::string titledCodePoint(std::string_view oneCodePoint) {
    std::string out;
    icu::StringByteSink<std::string> sink{&out};
    icu::Edits edits;
    UErrorCode status = U_ZERO_ERROR;
    icu::CaseMap::utf8ToTitle(
        "",
        0,
        nullptr,
        icu::StringPiece{oneCodePoint.data(), static_cast<std::int32_t>(oneCodePoint.size())},
        sink,
        &edits,
        status);
    return out;
}

/// Gaupol's `_capitalize_first`: the first alphanumeric character from `pos`,
/// capitalized — skipping only non-word characters, and never one right
/// after an ellipsis. Returns whether one was found.
bool capitalizeFirstFrom(std::string& text, std::size_t pos) {
    std::size_t i = pos;
    while (i < text.size() && !isWordCodePoint(codePointAt(text, i)))
        i = nextCodePoint(text, i);
    if (i >= text.size())
        return false;
    if (blockedByEllipsis(text, pos, i))
        return false;

    const std::size_t end = nextCodePoint(text, i);
    const std::string titled = titledCodePoint(std::string_view{text}.substr(i, end - i));
    text.replace(i, end - i, titled);
    return true;
}

/// Gaupol's `_capitalize_text`: every match of one pattern, in order —
/// `Start` capitalizes at the match, `After` right past it and asks the next
/// text to capitalize its own start if this one found nothing to capitalize.
///
/// The zero-length dedupe is `Finder`'s: the same rule `common_errors.cpp`
/// applies before a replacement, here before nothing is written at all.
[[nodiscard]] std::optional<FailureKind>
capitalizeMatches(PatternMatcher& matcher, CapitalizeAt at, std::string& text, bool& capNext) {
    std::size_t pos = 0;
    std::optional<MatchSpan> previous;

    while (true) {
        const std::expected<std::optional<Match>, SearchFailure> searched = matcher.find(text, pos);
        if (!searched)
            return FailureKind::TimedOut;
        if (!searched->has_value())
            return std::nullopt;

        const Match& match = **searched;
        const MatchSpan span = match.whole();
        if (previous == span && span.start == pos && span.end == pos) {
            if (pos >= text.size())
                return std::nullopt;
            pos = nextCodePoint(text, pos);
            continue;
        }

        if (at == CapitalizeAt::Start)
            capitalizeFirstFrom(text, span.start);
        if (at == CapitalizeAt::After)
            capNext = !capitalizeFirstFrom(text, span.end);

        pos = span.end;
        previous = span;
        if (span.start == span.end)
            previous = MatchSpan{.start = pos, .end = pos};
    }
}

struct Prepared {
    const CorrectionPattern* pattern;
    CapitalizeAt at;
    std::unique_ptr<PatternMatcher> matcher;
};

[[nodiscard]] std::vector<Prepared> prepare(const PatternEngine& engine,
                                            std::span<const CorrectionPattern* const> patterns,
                                            std::vector<PatternFailure>& failures) {
    std::vector<Prepared> prepared;
    for (const CorrectionPattern* pattern : patterns) {
        const auto* fields = std::get_if<CapitalizationFields>(&pattern->fields);
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
        prepared.push_back(
            Prepared{.pattern = pattern, .at = fields->capitalize, .matcher = std::move(*matcher)});
    }
    return prepared;
}

} // namespace

CorrectedTexts correctCapitalization(const PatternEngine& engine,
                                     std::span<const CorrectionPattern* const> patterns,
                                     std::span<const std::string> texts) {
    CorrectedTexts result;
    const std::vector<Prepared> prepared = prepare(engine, patterns, result.failures);

    result.texts.reserve(texts.size());
    bool capNext = false;
    for (std::size_t index = 0; index < texts.size(); ++index) {
        std::string text = texts[index];
        if (capNext || index == 0) {
            capitalizeFirstFrom(text, 0);
            capNext = false;
        }

        for (const Prepared& one : prepared) {
            // On a copy: a pattern given up halfway leaves the text — and the
            // state it would have handed the next text — as they were before it.
            std::string attempt = text;
            bool attemptCapNext = capNext;
            if (const std::optional<FailureKind> gaveUp =
                    capitalizeMatches(*one.matcher, one.at, attempt, attemptCapNext)) {
                result.failures.push_back(
                    failureOf(*gaveUp, *one.pattern, index, "the search was given up"));
            } else {
                text = std::move(attempt);
                capNext = attemptCapNext;
            }
        }
        result.texts.push_back(std::move(text));
    }
    return result;
}

} // namespace subedit::core
