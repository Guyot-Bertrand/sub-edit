#include <subedit/core/model/encoding.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;

/// The encoding of that name, or a failed test.
///
/// A function rather than `*Encoding::create(...)` at each call: the point of
/// the type is that a name may not name anything, and a test that dereferences
/// without asking says the opposite.
[[nodiscard]] Encoding named(std::string_view name) {
    const std::optional<Encoding> encoding = Encoding::create(name, ByteOrderMark::Absent);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas l'encodage " + std::string{name});
        return Encoding::utf8(ByteOrderMark::Absent);
    }
    return *encoding;
}

} // namespace

TEST_CASE("an encoding names a charset a converter would take", "[model][encoding]") {
    CHECK(Encoding::utf8(ByteOrderMark::Absent).charset() == "UTF-8");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).charset() == "UTF-16LE");
    CHECK(Encoding::utf16Be(ByteOrderMark::Present).charset() == "UTF-16BE");
}

TEST_CASE("the encodings named here are the ones ICU knows under those names",
          "[model][encoding]") {
    // The three named constructors spell their charset out rather than ask ICU
    // at every default construction. This is what keeps that shortcut honest:
    // the day the two spellings part, the gate says so instead of two values of
    // one encoding quietly comparing unequal.
    CHECK(Encoding::create("utf-8", ByteOrderMark::Absent) ==
          Encoding::utf8(ByteOrderMark::Absent));
    CHECK(Encoding::create("utf-16-le", ByteOrderMark::Present) ==
          Encoding::utf16Le(ByteOrderMark::Present));
    CHECK(Encoding::create("utf-16-be", ByteOrderMark::Present) ==
          Encoding::utf16Be(ByteOrderMark::Present));
}

TEST_CASE("an encoding nobody can read does not get built", "[model][encoding]") {
    // The whole point of the type: what cannot be converted cannot travel
    // through the model to fail at the moment of reading.
    CHECK_FALSE(Encoding::create("pas-un-encodage", ByteOrderMark::Absent).has_value());
    CHECK_FALSE(Encoding::create("", ByteOrderMark::Absent).has_value());
}

TEST_CASE("two names for one converter are one value", "[model][encoding]") {
    // `cp1252` and `windows-1252` are the same encoding, and equality has to
    // say so — the value keeps the spelling ICU settles on, not the one asked.
    CHECK(Encoding::create("cp1252", ByteOrderMark::Absent) ==
          Encoding::create("windows-1252", ByteOrderMark::Absent));
    CHECK(Encoding::create("latin1", ByteOrderMark::Absent) ==
          Encoding::create("iso-8859-1", ByteOrderMark::Absent));
}

TEST_CASE("the name kept is the one a reader recognises", "[model][encoding]") {
    // Neither of ICU's two spellings on its own: its internal name for
    // Windows-1252 is `ibm-5348_P100-1997`, and the IANA registry calls
    // Latin-1 `ISO_8859-1:1987`. Both are correct and neither is readable.
    CHECK(named("cp1252").charset() == "windows-1252");
    CHECK(named("latin1").charset() == "ISO-8859-1");
}

TEST_CASE("the byte order mark belongs to the encoding, not beside it", "[model][encoding]") {
    // The whole point of the type: one comparison decides whether a file comes
    // back as it arrived. A boolean kept apart could not say it for UTF-16,
    // where the mark carries the byte order as well.
    CHECK(Encoding::utf8(ByteOrderMark::Present) != Encoding::utf8(ByteOrderMark::Absent));
    CHECK(Encoding::utf16Le(ByteOrderMark::Present) != Encoding::utf16Be(ByteOrderMark::Present));
    CHECK(Encoding::utf8(ByteOrderMark::Absent) == Encoding::utf8(ByteOrderMark::Absent));
}

TEST_CASE("a mark is read back off the encoding it belongs to", "[model][encoding]") {
    CHECK(Encoding::utf8(ByteOrderMark::Present).byteOrderMark() == ByteOrderMark::Present);
    CHECK(Encoding::utf8(ByteOrderMark::Absent).byteOrderMark() == ByteOrderMark::Absent);
}

TEST_CASE("an encoding carries the bytes of its own mark", "[model][encoding]") {
    // Two bytes for UTF-16, and the order is the whole point: they are what
    // says which of the two files it is. The bytes are those of the charset,
    // whether or not this value says a mark is there — reading asks the first
    // question to answer the second.
    CHECK(Encoding::utf8(ByteOrderMark::Present).byteOrderMarkBytes() == "\xEF\xBB\xBF");
    CHECK(Encoding::utf8(ByteOrderMark::Absent).byteOrderMarkBytes() == "\xEF\xBB\xBF");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).byteOrderMarkBytes() == "\xFF\xFE");
    CHECK(Encoding::utf16Be(ByteOrderMark::Present).byteOrderMarkBytes() == "\xFE\xFF");
}

TEST_CASE("an encoding ICU knows and no registry names keeps ICU's own name", "[model][encoding]") {
    // Not every converter has a name in the MIME or IANA registries — this one
    // encodes the names of IMAP mailboxes, and belongs to no registry. The name
    // ICU knows it by is then the only one there is, and it is kept.
    CHECK(named("IMAP-mailbox-name").charset() == "IMAP-mailbox-name");
}

TEST_CASE("an encoding of the single-byte kind has no mark to carry", "[model][encoding]") {
    // A byte order mark exists for the Unicode encodings and for no other:
    // there is no order to announce where a character is one byte.
    CHECK(named("cp1252").byteOrderMarkBytes().empty());
}

TEST_CASE("a mark can be forced either way without losing the encoding", "[model][encoding]") {
    // What `--bom` and `--no-bom` ask for, and the reason the mark is a
    // variant rather than a value of its own.
    const Encoding read = Encoding::utf16Be(ByteOrderMark::Present);

    CHECK(read.withByteOrderMark(ByteOrderMark::Absent) ==
          Encoding::utf16Be(ByteOrderMark::Absent));
    CHECK(read.withByteOrderMark(ByteOrderMark::Present) == read);
}

TEST_CASE("a marked encoding is named as Python names it", "[model][encoding]") {
    CHECK(Encoding::utf8(ByteOrderMark::Absent).name() == "UTF-8");
    CHECK(Encoding::utf8(ByteOrderMark::Present).name() == "UTF-8-sig");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).name() == "UTF-16LE-sig");
}
