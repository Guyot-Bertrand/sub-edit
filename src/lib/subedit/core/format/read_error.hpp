#pragma once

#include <string>

namespace subedit::core {

/// Why a file could not be read at all.
///
/// The line is strict, and ADR 0008 draws it: what cannot be read at all is a
/// failure; what the reader recovers from is a `Diagnostic` and the reading
/// carries on. Returning an empty result with a warning would let the caller
/// lose a file without noticing.
/// **Every one of these is produced by a reader**, as for `DiagnosticKind`. An
/// enumerator nothing emits is a promise not kept: every caller translating one
/// into a message would carry a branch no test can reach.
///
/// One was dropped for that reason — `FileNotReadable`, which nothing ever
/// emitted. `readSubtitles` is given bytes and never a path, so it opens
/// nothing and cannot fail to; opening belongs to `FileSystem`, whose
/// `FileErrorKind` already tells absent from refused from broken.
enum class ReadErrorKind {
    Undecodable,     ///< bytes that do not decode in the encoding they were read in
    NoSubtitleFound, ///< nothing recognisable as a subtitle
    UnknownFormat,   ///< no format matched, and none is ever assumed
};

/// A reading that could not happen.
struct ReadError {
    ReadErrorKind kind;

    /// Free context. May be empty.
    std::string detail;

    friend bool operator==(const ReadError&, const ReadError&) = default;
};

} // namespace subedit::core
