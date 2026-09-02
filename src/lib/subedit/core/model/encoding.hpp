#pragma once

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
/// private, so an encoding nobody named cannot be built. The named
/// constructors below are the Unicode encodings *this code* speaks about,
/// because a byte order mark names one of them; the ninety-seven others come
/// in with the converter that can prove they exist.
class Encoding {

public:
    /// UTF-8, the encoding every file was read as until phase 8.
    [[nodiscard]] static Encoding utf8(ByteOrderMark mark) {
        return Encoding{"utf-8", "\xEF\xBB\xBF", mark};
    }

    /// UTF-16, little-endian.
    [[nodiscard]] static Encoding utf16Le(ByteOrderMark mark) {
        return Encoding{"utf-16-le", "\xFF\xFE", mark};
    }

    /// UTF-16, big-endian.
    [[nodiscard]] static Encoding utf16Be(ByteOrderMark mark) {
        return Encoding{"utf-16-be", "\xFE\xFF", mark};
    }

    /// Returns the encoding alone, mark excluded — the name a converter takes.
    ///
    /// Never the `-sig` spelling: that one names a file, this one names a way
    /// of turning bytes into text, and no converter knows what a mark is.
    [[nodiscard]] std::string_view charset() const { return m_charset; }

    /// Returns whether a mark precedes the text.
    [[nodiscard]] ByteOrderMark byteOrderMark() const { return m_mark; }

    /// Returns the bytes of that mark, empty when there is none.
    ///
    /// Held by the encoding rather than rendered from its name, and the
    /// difference is not cosmetic: `charactersOf(Newline)` renders a choice the
    /// writer makes, whereas a mark is decided by the encoding itself — the two
    /// bytes at the head of a UTF-16 file *are* what says its byte order. A
    /// mapping from names to marks somewhere else would be that same knowledge
    /// written twice, and dispatching on a name is the string-as-enumeration
    /// the design principles refuse.
    [[nodiscard]] std::string_view byteOrderMarkBytes() const {
        return m_mark == ByteOrderMark::Present ? m_markBytes : std::string_view{};
    }

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

    /// Returns the name of the whole answer, mark included.
    ///
    /// `utf-8-sig` is Python's own name for UTF-8 with a mark, and Gaupol shows
    /// it as such. Python has no name for the UTF-16 pair — it detects their
    /// mark rather than naming it — so the same suffix is carried over rather
    /// than a second convention invented for two cases.
    [[nodiscard]] std::string name() const {
        return m_mark == ByteOrderMark::Present ? m_charset + "-sig" : m_charset;
    }

    [[nodiscard]] friend bool operator==(const Encoding&, const Encoding&) = default;

private:
    Encoding(std::string_view charset, std::string_view markBytes, ByteOrderMark mark)
        : m_charset(charset), m_markBytes(markBytes), m_mark(mark) {}

    std::string m_charset;

    /// A literal, so the view outlives every copy of the encoding.
    std::string_view m_markBytes;

    ByteOrderMark m_mark;
};

} // namespace subedit::core
