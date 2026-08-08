#pragma once

#include <string>

namespace subedit::core {

/// How much a reader had to do about an anomaly.
enum class Severity {
    /// Something is wrong and was left as it stands, because only the user can
    /// decide what it should have been — a subtitle ending before it starts.
    Warning,

    /// Something was wrong and the reader made a decision about it — missing
    /// numbering, regenerated on writing.
    Recovered,
};

/// What a reader ran into.
///
/// A category, not a sentence: it can be translated, and a test can assert on
/// it without comparing prose that will be reworded.
///
/// **Every one of these is produced by a reader.** An enumerator nothing emits
/// is a promise not kept: the interface would offer a filter that never
/// matches, and a test would have no way to cover it. Two were dropped for
/// that reason — timestamps written without their zero padding, which writing
/// normalises silently and which would flood the list; and unclosed tags,
/// which need the tag-aware parser of phase 4 and will come back with it.
enum class DiagnosticKind {
    IgnoredLine,            ///< a line that fits nowhere
    MalformedTimestamp,     ///< a timestamp that could not be read
    EndBeforeStart,         ///< a subtitle that ends before it starts
    OverlappingSubtitles,   ///< a subtitle starting before the previous one ends
    MissingNumbering,       ///< a SubRip block without its number
    InconsistentNumbering,  ///< SubRip numbers that do not follow
    TextBeforeAnyTimestamp, ///< text before the first timestamp of the file
    UnknownBlock,           ///< a WebVTT block of an unknown kind
    MixedNewlines,          ///< several line endings in the same file
};

/// One anomaly met while reading, and where it was.
struct Diagnostic {
    Severity severity;

    /// Line number **counted from one**, as an editor shows it.
    int line;

    DiagnosticKind kind;

    /// Free context, for what the category cannot carry. May be empty.
    std::string detail;

    friend bool operator==(const Diagnostic&, const Diagnostic&) = default;
};

} // namespace subedit::core
