#include <subedit/core/model/encoding.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::EncodingRefusal;

/// The encoding of that name, or a failed test.
///
/// A function rather than `*Encoding::create(...)` at each call: the point of
/// the type is that a name may not name anything, and a test that dereferences
/// without asking says the opposite.
[[nodiscard]] Encoding named(std::string_view name) {
    const std::expected<Encoding, EncodingRefusal> encoding =
        Encoding::create(name, ByteOrderMark::Absent);
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
    CHECK(Encoding::create("utf-8", ByteOrderMark::Absent).value() ==
          Encoding::utf8(ByteOrderMark::Absent));
    CHECK(Encoding::create("utf-16-le", ByteOrderMark::Present).value() ==
          Encoding::utf16Le(ByteOrderMark::Present));
    CHECK(Encoding::create("utf-16-be", ByteOrderMark::Present).value() ==
          Encoding::utf16Be(ByteOrderMark::Present));
}

TEST_CASE("an encoding nobody can read does not get built", "[model][encoding]") {
    // The whole point of the type: what cannot be converted cannot travel
    // through the model to fail at the moment of reading.
    CHECK(Encoding::create("pas-un-encodage", ByteOrderMark::Absent).error() ==
          EncodingRefusal::Unknown);
    CHECK(Encoding::create("", ByteOrderMark::Absent).error() == EncodingRefusal::Unknown);
}

TEST_CASE("an encoding that writes its own mark does not get built", "[model][encoding]") {
    // ICU's `UTF-16` converter puts a mark in front of the text by itself, so
    // the mark stops being the caller's to give or withhold — `--no-bom` was
    // accepted and disobeyed, and the report said « no BOM » of a file that
    // carried one.
    for (const char* name : {"UTF-16", "UTF-32"}) {
        INFO("encodage : " << name);
        CHECK(Encoding::create(name, ByteOrderMark::Absent).error() ==
              EncodingRefusal::WritesItsOwnMark);
    }

    // The refusal is on the converter and not on the name, and these four are
    // what proves it: `UTF-16LE,version=1` says its byte order and writes a
    // mark all the same.
    for (const char* name :
         {"UTF-16,version=1", "UTF-16,version=2", "UTF-16LE,version=1", "UTF-16BE,version=1"}) {
        INFO("encodage : " << name);
        CHECK(Encoding::create(name, ByteOrderMark::Absent).error() ==
              EncodingRefusal::WritesItsOwnMark);
    }
}

TEST_CASE("the aliases of an encoding that writes its own mark are refused too",
          "[model][encoding]") {
    // The refusal is read off the canonical name, which is what makes it hold
    // for the four spellings a user might reach for.
    for (const char* name : {"UCS-2", "ISO-10646-UCS-2", "unicode", "utf16"}) {
        INFO("alias : " << name);
        CHECK(Encoding::create(name, ByteOrderMark::Absent).error() ==
              EncodingRefusal::WritesItsOwnMark);
    }
}

TEST_CASE("naming the byte order is what makes an encoding writable", "[model][encoding]") {
    // The way out the refusal names, and it costs nothing: the two explicit
    // spellings write the same bytes, mark included, under the caller's
    // control.
    for (const char* name : {"UTF-16LE", "UTF-16BE", "UTF-32LE", "UTF-32BE"}) {
        INFO("encodage : " << name);
        CHECK(Encoding::create(name, ByteOrderMark::Absent).has_value());
    }
}

TEST_CASE("an encoding that writes bytes of its own without a mark is kept", "[model][encoding]") {
    // **The counter-example that draws the line**, and the reason the refusal is
    // not « writes more bytes than the text ». `HZ-GB-2312` opens with `~}`, its
    // switch into ASCII mode: two bytes that belong to the encoded text. It is
    // the seventh converter the probe of `encoding.cpp` turns up, and the only
    // one of the seven that must stay writable.
    CHECK(Encoding::create("HZ", ByteOrderMark::Absent).has_value());
}

TEST_CASE("two names for one converter are one value", "[model][encoding]") {
    // `cp1252` and `windows-1252` are the same encoding, and equality has to
    // say so — the value keeps the spelling ICU settles on, not the one asked.
    CHECK(named("cp1252") == named("windows-1252"));
    CHECK(named("latin1") == named("iso-8859-1"));
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

TEST_CASE("an encoding answers its charset and its mark, and nothing worded", "[model][encoding]") {
    // **Issue #315.** The type used to answer `name()` — the charset with
    // `-sig` glued on when a mark was there. `UTF-8-sig` is Python's spelling;
    // `UTF-16LE-sig` is nobody's, Python having no name for that pair. Naming a
    // value for a reader is `wording.hpp`'s work, and the mark is said in words
    // there rather than invented here.
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).charset() == "UTF-16LE");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).byteOrderMark() == ByteOrderMark::Present);
    CHECK(Encoding::utf16Le(ByteOrderMark::Absent).byteOrderMark() == ByteOrderMark::Absent);
}
