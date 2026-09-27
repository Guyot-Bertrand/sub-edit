#pragma once

#include <subedit/core/text/pattern_engine.hpp>

#include <memory>
#include <string_view>

namespace subedit::core {

/// The engine of ADR 0036: `icu::RegexMatcher`, the one the search of the window
/// already reads its expressions with.
///
/// **Expressions are translated to ICU's syntax when they are compiled**, by
/// `icuExpressionOf`, and matched with Python's line terminators — `\n` alone,
/// which is what `.`, `^` and `$` know of a line in `re`.
///
/// **Every search is bounded.** A pattern that backtracks without end would
/// otherwise freeze the program, and one written by a user can do it; the
/// search is given up after `kSearchLimit` and reported as `TimedOut`.
class IcuPatternEngine final : public PatternEngine {

public:
    /// ICU counts its limit in steps of the match engine, which "will typically
    /// be on the order of milliseconds". Measured: a catastrophic pattern on a
    /// forty-letter text gives up in a few tenths of a second, and no
    /// shipped pattern comes near it.
    static constexpr int kSearchLimit = 1000;

    [[nodiscard]] std::expected<std::unique_ptr<PatternMatcher>, CompileError>
    compile(std::string_view expression, const PatternFlags& flags) const override;
};

} // namespace subedit::core
