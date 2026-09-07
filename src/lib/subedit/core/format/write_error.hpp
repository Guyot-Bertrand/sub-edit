#pragma once

#include <string>

namespace subedit::core {

/// Why subtitles could not be turned into the bytes of a file.
///
/// **Writing has no file system to refuse it** — `writeSubtitles` returns a
/// string, and putting it on disk is `writeAtomically`'s business, with
/// `FileError` for what the system says. What is left are the two things the
/// request itself can be short of.
///
/// The first is the text: a character the chosen encoding has no room for.
///
/// The second arrived with phase 9 and is transient by design: nine formats
/// are named, and the writers land one issue at a time. **It is a refusal and
/// not an `std::unreachable`** because `writeSubtitles` is public, so a caller
/// can name any of the nine; answering with undefined behaviour what is only a
/// gap in the calendar would be a poor trade.
///
/// An enumeration rather than a bare struct, for the reason `ReadErrorKind`
/// gives: the words live in `core/wording.hpp`, keyed by a category a test can
/// assert on without comparing prose.
enum class WriteErrorKind {
    Unencodable, ///< a character the chosen encoding cannot write
    NoWriter,    ///< that format cannot be written yet
};

/// A writing that could not happen.
struct WriteError {
    WriteErrorKind kind;

    /// The offending character, in UTF-8. May be empty when ICU kept none.
    std::string detail;

    friend bool operator==(const WriteError&, const WriteError&) = default;
};

} // namespace subedit::core
