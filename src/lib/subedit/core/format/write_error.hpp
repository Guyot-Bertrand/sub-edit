#pragma once

#include <string>

namespace subedit::core {

/// Why subtitles could not be turned into the bytes of a file.
///
/// **One cause, and it is the only one there can be.** Writing has no file
/// system to refuse it — `writeSubtitles` returns a string, and putting it on
/// disk is `writeAtomically`'s business, with `FileError` for what the system
/// says. What is left is the text itself: a character the chosen encoding has
/// no room for.
///
/// A single-valued enumeration rather than a bare struct, for the reason
/// `ReadErrorKind` gives: the words live in `core/wording.hpp`, keyed by a
/// category a test can assert on without comparing prose.
enum class WriteErrorKind {
    Unencodable, ///< a character the chosen encoding cannot write
};

/// A writing that could not happen.
struct WriteError {
    WriteErrorKind kind;

    /// The offending character, in UTF-8. May be empty when ICU kept none.
    std::string detail;

    friend bool operator==(const WriteError&, const WriteError&) = default;
};

} // namespace subedit::core
