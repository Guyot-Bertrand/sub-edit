#pragma once

#include <subedit/core/model/source_file.hpp>

#include <string_view>

namespace subedit::core {

/// Tells whether `bytes` decode as UTF-8.
///
/// Checked rather than assumed: a file read as UTF-8 when it was Latin-1 does
/// not fail, it produces text with the accents replaced by nonsense — and the
/// user only finds out once the file is saved over. Refusing outright is the
/// only honest answer while this phase handles no other encoding.
///
/// Overlong encodings and surrogate halves are refused too. They are not
/// merely unusual: accepting them is how a length check gets bypassed.
[[nodiscard]] bool isValidUtf8(std::string_view bytes);

/// Tells whether `bytes` begin with a UTF-8 byte order mark.
[[nodiscard]] bool hasUtf8Bom(std::string_view bytes);

/// Returns `bytes` without their byte order mark, if they had one.
///
/// Kept out of the text and put back on writing: a mark that survived into the
/// first subtitle would show up as an invisible character at the head of the
/// file.
[[nodiscard]] std::string_view withoutUtf8Bom(std::string_view bytes);

/// What the line endings of a file look like.
struct NewlineScan {
    /// The ending used most often, and the one a caller would put back.
    Newline newline = Newline::Lf;

    /// Set when the file mixes several kinds of ending.
    bool mixed = false;

    /// The line where the second kind first appears, `0` when there is none.
    int mixedAtLine = 0;
};

/// Looks at how `content` ends its lines.
///
/// The most frequent ending wins. A file assembled out of two others mixes
/// them, which happens more often than it should and is worth reporting rather
/// than silently normalising.
[[nodiscard]] NewlineScan scanNewlines(std::string_view content);

} // namespace subedit::core
