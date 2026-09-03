#pragma once

#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// Turns `bytes` into UTF-8 text, reading them in `encoding`.
///
/// Nothing above this line speaks anything but UTF-8: the model holds text, the
/// readers parse text, and the encoding is a property of the file that stops at
/// this function. Which is why the answer is the text and not a converter.
///
/// **Nothing is substituted.** ICU replaces a byte it cannot map with U+FFFD by
/// default, which turns a file read in the wrong encoding into a file full of
/// question marks that no one refuses; here the conversion stops and the answer
/// is empty. A file read as UTF-8 when it was Latin-1 has to fail, or the user
/// finds out once they have saved over it.
///
/// Overlong sequences and surrogate halves are refused with the rest. They are
/// not merely unusual: accepting them is how a length check gets bypassed.
[[nodiscard]] std::optional<std::string> decodeToUtf8(std::string_view bytes,
                                                      const Encoding& encoding);

/// How the encoding of a file was arrived at.
enum class EncodingChoice {
    /// The file said so, with the only mark it can carry.
    ByteOrderMark,

    /// Nobody said, and the bytes were weighed.
    Detected,
};

/// An encoding proposed for these bytes, and how it was arrived at.
struct DetectedEncoding {
    Encoding encoding;
    EncodingChoice choice;
};

/// Proposes the encoding of `bytes`, or nothing when it can propose none.
///
/// **The mark first, always.** It is the only thing a subtitle file ever says
/// about its own encoding — there is no header and no naming convention — so
/// weighing bytes against it would be answering a question already settled.
///
/// **What comes after is a proposal and never a certainty**, and the type says
/// so by naming where the answer came from. Latin-1 and CP1252 are the same
/// encoding outside the range `0x80–0x9f`: on a text that does not use it, the
/// bytes do not carry the answer, and a detector that picks one is guessing.
/// ADR 0027 measured that, and `latin1.srt` of the labelled corpus is the case.
[[nodiscard]] std::optional<DetectedEncoding> detectEncoding(std::string_view bytes);

/// Tells whether `bytes` begin with the mark `encoding` would carry.
///
/// The mark of the charset, whether or not this value says it is there — that
/// is the question reading has to ask, and the answer is what it then records.
[[nodiscard]] bool startsWithByteOrderMark(std::string_view bytes, const Encoding& encoding);

/// Returns `bytes` without that mark, if they had it.
///
/// Kept out of the text and put back on writing: a mark that survived into the
/// first subtitle would show up as an invisible character at the head of the
/// file — and a converter would not take it off, since to a converter it is a
/// zero-width space like any other.
[[nodiscard]] std::string_view withoutByteOrderMark(std::string_view bytes,
                                                    const Encoding& encoding);

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
