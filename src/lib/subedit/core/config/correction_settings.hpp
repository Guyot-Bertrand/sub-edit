#pragma once

// What the correction assistant retains, from one opening to the next and
// from one session to the next — decision D8 of the spec of phase 12, issue
// #504.

#include <subedit/core/text/correction_pattern.hpp>

#include <span>
#include <string>
#include <vector>

namespace subedit::core {

/// Gaupol's own default line-break limits, in characters — decision D5.
inline constexpr double kDefaultLineBreakMaxLength = 24.0;
inline constexpr int kDefaultLineBreakMaxLines = 3;

/// One task's own settings: whether it runs, and which cascade it reads —
/// `Script[-language[-COUNTRY]]`, `Zyyy` for every script.
struct TaskSettings {
    bool enabled = false;
    std::string code = "Zyyy";

    friend bool operator==(const TaskSettings&, const TaskSettings&) = default;
};

/// What one pattern's activation was set to, on top of what its shipped
/// `.conf` gives by default — decision D2: activation is kept in this
/// program's own settings, by kind, code and name.
///
/// `code` and `name` name the record as its own file does — a pattern of
/// `Latn-en.common-error` is named by `Latn-en`, never by the cascade a task
/// was asked to run under, which may reach it from a more specific code.
struct PatternActivation {
    PatternKind kind = PatternKind::CommonError;
    std::string code;
    std::string name;
    bool enabled = true;

    friend bool operator==(const PatternActivation&, const PatternActivation&) = default;
};

/// Every setting the assistant retains — D8's own list.
struct CorrectionSettings {
    TaskSettings mentions{};
    TaskSettings commonErrors{.enabled = true};
    TaskSettings capitalization{.enabled = true};
    TaskSettings lineBreak{};

    /// D4: a record that carries both classes applies if either is checked.
    /// Common errors alone: the other three kinds carry no class.
    bool human = true;
    bool ocr = true;

    /// The two mention records the phase-4 scan answers for, not the pattern
    /// engine — D7. Decoupled from `patternActivations` because neither names
    /// a compiled pattern: the scan runs, or it does not.
    bool soundInBrackets = false;
    bool soundInParentheses = false;

    /// Overrides of what a `.conf` or a shipped default gives, one entry per
    /// pattern a user has touched from the assistant's own pages. A name that
    /// no longer exists is ignored, like every other setting ADR 0022 keeps.
    std::vector<PatternActivation> patternActivations{};

    double lineBreakMaxLength = kDefaultLineBreakMaxLength;
    int lineBreakMaxLines = kDefaultLineBreakMaxLines;

    bool removeBlankSubtitles = true;

    friend bool operator==(const CorrectionSettings&, const CorrectionSettings&) = default;
};

/// Whether `pattern` applies under `settings` — decision D2: an explicit
/// override by kind, code and name, or the shipped `.conf` default when there
/// is none. **Class filtering (D4) is not this function's**: it answers
/// activation alone, the same split `correction_run.cpp`'s own `isEnabled`
/// and `classesAllow` already keep apart.
[[nodiscard]] bool patternEnabled(const CorrectionPattern& pattern,
                                  const CorrectionSettings& settings);

/// Folds what a page of the assistant shows into `activations` — issue #505.
///
/// **Erase, then add.** Every override on a record of `shown` is removed
/// first, then `overrides` are appended: a record whose box has come back to
/// its shipped default thereby loses the override an earlier run wrote,
/// instead of keeping it forever. An override on a record `shown` does not
/// name — another code's, another kind's — is left exactly as it was.
///
/// `overrides` is expected to name only records of `shown`, the ones whose
/// box disagrees with their default; `shown` is every record the page gave a
/// box to, agreeing or not.
void foldActivations(std::vector<PatternActivation>& activations,
                     std::span<const CorrectionPattern* const> shown,
                     std::span<const PatternActivation> overrides);

} // namespace subedit::core
