#pragma once

#include <string>

namespace subedit::core {

/// Why a file could not be read at all.
///
/// The line is strict, and ADR 0008 draws it: what cannot be read at all is a
/// failure; what the reader recovers from is a `Diagnostic` and the reading
/// carries on. Returning an empty result with a warning would let the caller
/// lose a file without noticing.
enum class ReadErrorKind {
    FileNotReadable, ///< the file could not be opened
    InvalidUtf8,     ///< bytes that are not valid UTF-8
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
