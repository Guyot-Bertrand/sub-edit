#include <subedit/core/model/encoding.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;

} // namespace

TEST_CASE("an encoding names a charset a converter would take", "[model][encoding]") {
    CHECK(Encoding::utf8(ByteOrderMark::Absent).charset() == "utf-8");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).charset() == "utf-16-le");
    CHECK(Encoding::utf16Be(ByteOrderMark::Present).charset() == "utf-16-be");
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
    // says which of the two files it is.
    CHECK(Encoding::utf8(ByteOrderMark::Present).byteOrderMarkBytes() == "\xEF\xBB\xBF");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).byteOrderMarkBytes() == "\xFF\xFE");
    CHECK(Encoding::utf16Be(ByteOrderMark::Present).byteOrderMarkBytes() == "\xFE\xFF");
}

TEST_CASE("an encoding without a mark has no bytes to put back", "[model][encoding]") {
    CHECK(Encoding::utf8(ByteOrderMark::Absent).byteOrderMarkBytes().empty());
    CHECK(Encoding::utf16Le(ByteOrderMark::Absent).byteOrderMarkBytes().empty());
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
    CHECK(Encoding::utf8(ByteOrderMark::Absent).name() == "utf-8");
    CHECK(Encoding::utf8(ByteOrderMark::Present).name() == "utf-8-sig");
    CHECK(Encoding::utf16Le(ByteOrderMark::Present).name() == "utf-16-le-sig");
}
