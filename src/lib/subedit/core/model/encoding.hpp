#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace subedit::core {

/// Whether the bytes of a file spell their encoding out with a byte order mark.
///
/// An enumeration rather than a boolean: `writeSubtitles(format, request,
/// true)` said nothing at the call site about what was true.
enum class ByteOrderMark {
    Absent,
    Present,
};

/// Why a name does not become an encoding.
///
/// Two answers and not one, because they are not the same news: ICU knows
/// nothing of `klingon-1`, and it knows `UTF-16` perfectly well — too well, in
/// fact, since its converter writes a mark of its own.
enum class EncodingRefusal {
    /// Nothing converts under that name.
    Unknown,

    /// The converter puts a byte order mark in front of the text by itself.
    ///
    /// **Which takes the mark out of the caller's hands, and that is the whole
    /// reason to refuse.** This type's contract is that the bytes of a file are
    /// the mark the caller asked for, followed by the text — one comparison
    /// then decides whether a file comes back as it arrived, `--bom` and
    /// `--no-bom` mean something, and a report that says « no BOM » is telling
    /// the truth. A converter that writes its own mark breaks all three at
    /// once, and silently.
    WritesItsOwnMark,
};

/// The encoding a file was read in, and the one writing puts back.
///
/// **The byte order mark is a variant of the encoding, not a flag beside it.**
/// `utf-8` and `utf-8-sig` are two different answers, as Python names them, and
/// the boolean that stood here until phase 8 could say neither of the two
/// things a UTF-16 mark says: which encoding, and in which byte order. Making
/// the mark part of the value is what lets one comparison decide whether a file
/// comes back as it arrived.
///
/// **A validated name and not an enumeration**, because the set of encodings is
/// ICU's — ADR 0027 — and no table of them is written in this repository. That
/// is not a `std::string` used as an enumeration either: the constructor is
/// private, and the only way in for a name is `create`, which asks ICU whether
/// it can convert it. An encoding nobody can read is unrepresentable rather
/// than reported at the point of use.
class Encoding {

public:
    /// Builds the encoding ICU knows under `name`, or says why it does not.
    ///
    /// Aliases are accepted the way ICU accepts them — `cp1252`, `windows-1252`
    /// and `WINDOWS 1252` all name one encoding — and the value keeps the
    /// canonical spelling, so that two names for the same converter compare
    /// equal. Answering "unknown" is the point: the alternative is a name that
    /// travels through the model and fails at the moment of reading, when the
    /// user is no longer being asked anything.
    ///
    /// **A converter that writes its own byte order mark is refused here too**,
    /// and for the same reason one door earlier: `UTF-16` used to travel
    /// through the model, and the file it produced carried a mark the model
    /// said was absent — so `--no-bom` was accepted and disobeyed. See
    /// `EncodingRefusal::WritesItsOwnMark`.
    [[nodiscard]] static std::expected<Encoding, EncodingRefusal> create(std::string_view name,
                                                                         ByteOrderMark mark);

    /// UTF-8, the encoding every file was read as until phase 8.
    [[nodiscard]] static Encoding utf8(ByteOrderMark mark) { return Encoding{"UTF-8", mark}; }

    /// UTF-16, little-endian.
    [[nodiscard]] static Encoding utf16Le(ByteOrderMark mark) { return Encoding{"UTF-16LE", mark}; }

    /// UTF-16, big-endian.
    [[nodiscard]] static Encoding utf16Be(ByteOrderMark mark) { return Encoding{"UTF-16BE", mark}; }

    /// Returns the encoding alone, mark excluded — the name a converter takes.
    ///
    /// The canonical spelling ICU gives it, which is its IANA name wherever it
    /// has one. Never the `-sig` spelling: that one names a file, this one
    /// names a way of turning bytes into text, and no converter knows what a
    /// mark is.
    [[nodiscard]] std::string_view charset() const { return m_charset; }

    /// Returns whether a mark precedes the text.
    [[nodiscard]] ByteOrderMark byteOrderMark() const { return m_mark; }

    /// Returns the bytes of the mark this charset uses, empty for the ones that
    /// use none.
    ///
    /// **What the charset would carry, not what this value carries** — whether
    /// it is actually there is `byteOrderMark`. Reading needs both apart: it
    /// takes the mark off the bytes to see whether it was ever there.
    ///
    /// Held by the encoding rather than mapped from its name somewhere else:
    /// the two bytes at the head of a UTF-16 file *are* what says its byte
    /// order, and dispatching on a name elsewhere would be the
    /// string-as-enumeration the design principles refuse.
    [[nodiscard]] std::string_view byteOrderMarkBytes() const { return m_markBytes; }

    /// Returns the same encoding, its mark forced either way.
    ///
    /// What `--bom` and `--no-bom` ask for: the mark is the one part of an
    /// encoding a caller overrides on its own, and the alternative would be to
    /// carry it beside the encoding again to put the two back together at the
    /// end.
    [[nodiscard]] Encoding withByteOrderMark(ByteOrderMark mark) const {
        Encoding other{*this};
        other.m_mark = mark;
        return other;
    }

    // **No `name()` here, and that is issue #315.** It used to answer the
    // charset with `-sig` glued on when a mark was there — Python's spelling
    // for UTF-8, and nobody's for `UTF-16LE-sig`. Naming a value for a reader
    // is `wording.hpp`'s work, and it is where `nameOf(const Encoding&)` lives
    // now; what this type answers is `charset()` and `byteOrderMark()`, which
    // are what it knows.

    [[nodiscard]] friend bool operator==(const Encoding& left, const Encoding& right) {
        return left.m_charset == right.m_charset && left.m_mark == right.m_mark;
    }

private:
    Encoding(std::string_view charset, ByteOrderMark mark);

    std::string m_charset;

    /// A literal, so the view outlives every copy of the encoding.
    std::string_view m_markBytes;

    ByteOrderMark m_mark;
};

} // namespace subedit::core
