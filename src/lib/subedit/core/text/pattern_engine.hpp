#pragma once

// What a regular-expression engine is to the correction of texts.
//
// **One implementation, and the interface is still worth having** — ADR 0036:
// the variation is identified, the ADR says in which case PCRE2 would come
// back, and it is this interface that would make the return local. What crosses
// it is deliberately small: a pattern compiled once, a search from an offset,
// the spans of what it found.

#include <subedit/core/text/correction_pattern.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// A span of a text, in bytes.
struct MatchSpan {
    std::size_t start = 0;
    std::size_t end = 0;

    friend bool operator==(const MatchSpan&, const MatchSpan&) = default;
};

/// What a search found: the whole match, then each group of the pattern — and
/// nothing for a group that took no part.
struct Match {
    /// Index 0 is the whole match, so a group is at its own number.
    std::vector<std::optional<MatchSpan>> groups;

    [[nodiscard]] MatchSpan whole() const { return groups.front().value_or(MatchSpan{}); }
};

/// Why a search gave no answer.
///
/// **One reason, and it is deliberate**: a matcher or a text the engine could
/// not make is a search that finds nothing, not a distinct failure — writing
/// the two apart would leave a branch no test can reach, the same choice the
/// search of the window already made (`core/edit/search.cpp`). A timeout is
/// the one failure that is both reachable and worth naming to a caller: it is
/// the guard ADR 0036 asks for, and `correctCommonErrors` reports it by the
/// text and the pattern that caused it.
enum class SearchFailure {
    TimedOut, ///< the engine gave up: a pattern that backtracks without end
};

/// A pattern compiled by an engine.
class PatternMatcher {

public:
    virtual ~PatternMatcher() = default;

    /// How many groups the pattern has, not counting the whole match.
    [[nodiscard]] virtual std::size_t groupCount() const = 0;

    /// The number of the group called `name`, if the pattern has one.
    [[nodiscard]] virtual std::optional<int> groupNumber(std::string_view name) const = 0;

    /// The first match at or after byte `from` of `text`, which is a UTF-8 text.
    ///
    /// **The whole text is what is searched**, `from` only where the search
    /// starts: a lookbehind sees what comes before it, as it does in Gaupol.
    [[nodiscard]] virtual std::expected<std::optional<Match>, SearchFailure>
    find(std::string_view text, std::size_t from) = 0;

protected:
    PatternMatcher() = default;
    PatternMatcher(const PatternMatcher&) = default;
    PatternMatcher(PatternMatcher&&) = default;
    PatternMatcher& operator=(const PatternMatcher&) = default;
    PatternMatcher& operator=(PatternMatcher&&) = default;
};

/// Why an expression was not compiled.
enum class CompileFailure {
    Untranslatable, ///< it uses something the translation to the engine's syntax refuses
    Invalid,        ///< the engine refused it
};

struct CompileError {
    CompileFailure kind;
    std::string reason;

    friend bool operator==(const CompileError&, const CompileError&) = default;
};

class PatternEngine {

public:
    virtual ~PatternEngine() = default;

    /// Compiles `expression`, **written as Gaupol writes it** — for Python's
    /// `re` — with the flags its record names.
    [[nodiscard]] virtual std::expected<std::unique_ptr<PatternMatcher>, CompileError>
    compile(std::string_view expression, const PatternFlags& flags) const = 0;

protected:
    PatternEngine() = default;
    PatternEngine(const PatternEngine&) = default;
    PatternEngine(PatternEngine&&) = default;
    PatternEngine& operator=(const PatternEngine&) = default;
    PatternEngine& operator=(PatternEngine&&) = default;
};

} // namespace subedit::core
