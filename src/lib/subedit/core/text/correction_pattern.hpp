#pragma once

// One record of a Gaupol pattern file, read into a type.
//
// Gaupol keeps a record as a dictionary of strings and asks it for whatever it
// needs, wherever it needs it: a misspelt key is an empty answer. Here each
// kind of pattern has the fields it has, so a line-break penalty cannot be
// asked of a common-error record and the compiler says so — ADR 0037 and the
// design principles.

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace subedit::core {

/// What a pattern does, and the extension of the files that hold it.
enum class PatternKind {
    CommonError,
    Capitalization,
    HearingImpaired,
    LineBreak,
};

/// The extension of the files that hold `kind` — `common-error` in
/// `Latn-en.common-error`.
[[nodiscard]] std::string_view fileExtensionOf(PatternKind kind);

/// The flags a record may name. Gaupol evaluates `Flags` with `getattr(re, …)`,
/// so any flag of Python's `re` module would do; the shipped files use these
/// three, and a record naming another is refused rather than guessed at.
struct PatternFlags {
    bool dotAll = false;
    bool multiline = false;
    bool ignoreCase = false;

    friend bool operator==(const PatternFlags&, const PatternFlags&) = default;
};

/// The classes of common error. A pattern may carry both.
struct ErrorClasses {
    bool human = false;
    bool ocr = false;

    friend bool operator==(const ErrorClasses&, const ErrorClasses&) = default;
};

/// What a capitalization pattern capitalizes, relative to its match.
enum class CapitalizeAt {
    Start, ///< the first letter from the start of the match
    After, ///< the first letter after the end of the match
};

struct CommonErrorFields {
    ErrorClasses classes;
    std::string replacement;

    /// Applied again until nothing changes.
    bool repeat = false;

    friend bool operator==(const CommonErrorFields&, const CommonErrorFields&) = default;
};

struct CapitalizationFields {
    CapitalizeAt capitalize = CapitalizeAt::Start;

    friend bool operator==(const CapitalizationFields&, const CapitalizationFields&) = default;
};

struct HearingImpairedFields {
    std::string replacement;

    friend bool operator==(const HearingImpairedFields&, const HearingImpairedFields&) = default;
};

struct LineBreakFields {
    /// The capture group whose start is where the line breaks.
    int group = 0;
    double penalty = 0.0;

    friend bool operator==(const LineBreakFields&, const LineBreakFields&) = default;
};

/// One record of a pattern file.
///
/// **The expression and the replacement are still Gaupol's text**, written for
/// Python's `re`. Translating them is the engine's business (ADR 0036), and
/// keeping them as read is what lets a diagnostic quote the record.
struct CorrectionPattern {
    /// `Script[-language[-COUNTRY]]`, or `Zyyy` for every script.
    std::string code;

    /// The record's place in its file, counted from one and **counting the
    /// records that were refused** — so that `Latn-en:3` names the same record
    /// whatever its neighbours were worth.
    std::size_t rank = 0;

    /// The English name, which is the identifier: Gaupol translates it for
    /// display and finds a pattern by the untranslated one. Several records may
    /// share a name, and share a single tick box.
    std::string name;
    std::string description;

    std::string expression;
    PatternFlags flags;

    /// `Policy=Replace`: this record takes the place of the less specific ones
    /// that bear its name, instead of following them.
    bool replacesSameName = false;

    /// The codes this record is not applied in, though a less specific code
    /// holds it.
    std::vector<std::string> skipIn;

    /// What the shipped `.conf` says, `true` where it is silent.
    bool enabled = true;

    std::variant<CommonErrorFields, CapitalizationFields, HearingImpairedFields, LineBreakFields>
        fields;

    /// Kept in step with `PatternKind` by `correction_pattern.cpp`.
    [[nodiscard]] PatternKind kind() const;
};

} // namespace subedit::core
