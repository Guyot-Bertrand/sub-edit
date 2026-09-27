#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/pattern_engine.hpp>
#include <subedit/core/text/python_regex_syntax.hpp>

#include <unicode/parseerr.h>
#include <unicode/regex.h>
#include <unicode/stringpiece.h>
#include <unicode/uregex.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

[[nodiscard]] bool failed(UErrorCode status) {
    return status > U_ZERO_ERROR;
}

class IcuMatcher final : public PatternMatcher {

public:
    IcuMatcher(std::unique_ptr<icu::RegexPattern> pattern,
               std::unique_ptr<icu::RegexMatcher> matcher)
        : m_pattern(std::move(pattern)), m_matcher(std::move(matcher)) {}

    ~IcuMatcher() override { utext_close(m_text); }

    IcuMatcher(const IcuMatcher&) = delete;
    IcuMatcher& operator=(const IcuMatcher&) = delete;
    IcuMatcher(IcuMatcher&&) = delete;
    IcuMatcher& operator=(IcuMatcher&&) = delete;

    [[nodiscard]] std::size_t groupCount() const override {
        return static_cast<std::size_t>(m_matcher->groupCount());
    }

    [[nodiscard]] std::optional<int> groupNumber(std::string_view name) const override {
        UErrorCode status = U_ZERO_ERROR;
        const int number = m_pattern->groupNumberFromName(
            icu::UnicodeString::fromUTF8(
                icu::StringPiece{name.data(), static_cast<std::int32_t>(name.size())}),
            status);
        if (failed(status))
            return std::nullopt;
        return number;
    }

    [[nodiscard]] std::expected<std::optional<Match>, SearchFailure>
    find(std::string_view text, std::size_t from) override {
        if (from > text.size())
            return std::optional<Match>{};

        UErrorCode status = U_ZERO_ERROR;
        // The same `UText` is opened again over each text, which is what ICU
        // means by passing the previous one back.
        m_text =
            utext_openUTF8(m_text, text.data(), static_cast<std::int64_t>(text.size()), &status);

        // **One answer for « nothing found » and « nothing to search with »**,
        // the choice `core/edit/search.cpp` already made for the same engine:
        // a text ICU could not open is a search that finds nothing, not a
        // second failure to report — `search.cpp` leaves the equivalent guard
        // untested for the same reason this one is: ICU opens malformed UTF-8
        // leniently rather than refusing it, so no text this engine is ever
        // handed reaches this branch.
        const bool ready = !failed(status);
        if (ready) {
            m_matcher->reset(m_text);
            m_matcher->setTimeLimit(IcuPatternEngine::kSearchLimit, status);
        }
        const bool found = ready && m_matcher->find(static_cast<std::int64_t>(from), status) != 0 &&
                           !failed(status);
        if (status == U_REGEX_TIME_OUT)
            return std::unexpected{SearchFailure::TimedOut};
        if (!found)
            return std::optional<Match>{};

        Match match;
        for (std::int32_t group = 0; group <= m_matcher->groupCount(); ++group) {
            // Status is not checked again here: a `find()` that succeeded
            // guarantees every group in range answers, the same trust
            // `search.cpp`'s `expanded()` places in `matcher.group()`.
            const std::int64_t start = m_matcher->start64(group, status);
            const std::int64_t end = m_matcher->end64(group, status);
            if (start < 0) {
                match.groups.emplace_back(std::nullopt);
            } else {
                match.groups.emplace_back(MatchSpan{.start = static_cast<std::size_t>(start),
                                                    .end = static_cast<std::size_t>(end)});
            }
        }
        return std::optional<Match>{std::move(match)};
    }

private:
    std::unique_ptr<icu::RegexPattern> m_pattern;
    std::unique_ptr<icu::RegexMatcher> m_matcher;
    UText* m_text = nullptr;
};

[[nodiscard]] std::uint32_t icuFlagsOf(const PatternFlags& flags) {
    // `.`, `^` and `$` know `\n` alone as a line terminator in Python's `re`.
    std::uint32_t icu = UREGEX_UNIX_LINES;
    if (flags.dotAll)
        icu |= UREGEX_DOTALL;
    if (flags.multiline)
        icu |= UREGEX_MULTILINE;
    if (flags.ignoreCase)
        icu |= UREGEX_CASE_INSENSITIVE;
    return icu;
}

} // namespace

std::expected<std::unique_ptr<PatternMatcher>, CompileError>
IcuPatternEngine::compile(std::string_view expression, const PatternFlags& flags) const {
    const std::expected<std::string, SyntaxError> translated = icuExpressionOf(expression);
    if (!translated) {
        return std::unexpected{CompileError{.kind = CompileFailure::Untranslatable,
                                            .reason = translated.error().reason}};
    }

    UParseError where{};
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::RegexPattern> pattern{icu::RegexPattern::compile(
        icu::UnicodeString::fromUTF8(
            icu::StringPiece{translated->data(), static_cast<std::int32_t>(translated->size())}),
        icuFlagsOf(flags),
        where,
        status)};
    if (failed(status) || pattern == nullptr) {
        return std::unexpected{CompileError{.kind = CompileFailure::Invalid,
                                            .reason = std::string{u_errorName(status)} + " at " +
                                                      std::to_string(where.offset)}};
    }

    // Not checked for failure: `core/edit/search.cpp` trusts the same call the
    // same way, once the pattern itself compiled — the only realistic way this
    // could fail is memory exhaustion, which no test can stage honestly.
    std::unique_ptr<icu::RegexMatcher> matcher{pattern->matcher(status)};
    return std::make_unique<IcuMatcher>(std::move(pattern), std::move(matcher));
}

} // namespace subedit::core
