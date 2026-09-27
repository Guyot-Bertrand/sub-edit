#pragma once

// Gaupol's expressions, written for Python's `re`, in the syntax ICU reads.
//
// ADR 0036: the engine is ICU, and what this file is for is the part of the
// decision that is not the engine — **a pattern of Gaupol must mean here what
// it means there**. The translation is a closed list, written case by case; what
// it does not know it refuses, and the pattern is disabled and named rather
// than applied at random.

#include <expected>
#include <string>
#include <string_view>

namespace subedit::core {

/// Why a Python expression could not be turned into an ICU one.
struct SyntaxError {
    std::string reason;

    friend bool operator==(const SyntaxError&, const SyntaxError&) = default;
};

/// The ICU spelling of a Python expression.
///
/// What changes:
///
/// - **`\w`, `\W`, `\b` and `\B` are Python's.** In Python a word character is a
///   letter, a number or `_`; in ICU it is also a combining mark, so `î` written
///   as `i` and an accent is two word characters there and one here. The
///   measurement of ADR 0036 found twenty such texts in the private corpus,
///   and none once the classes are written out. `\b` and `\B` are rewritten
///   the same way, since they are defined by `\w`.
/// - **`\Z` is `\z`**: Python's `\Z` is the absolute end, ICU's also accepts a
///   line break before it.
/// - **`(?P<name>…)` and `(?P=name)` are `(?<name>…)` and `\k<name>`.** ICU
///   names are letters and digits only, so a name with an underscore is refused.
/// - **Inside a class, `[` and `&&` are literal**, as in Python; in ICU they
///   open a nested set and intersect two.
///
/// What does not: everything else is written the same in both, and ICU says so
/// itself when it is not — at compilation, not here.
[[nodiscard]] std::expected<std::string, SyntaxError> icuExpressionOf(std::string_view python);

} // namespace subedit::core
