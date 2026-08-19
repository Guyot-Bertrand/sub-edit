#include <subedit/core/model/source_file.hpp>
#include <subedit/core/text/encoding.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {

using subedit::core::hasUtf8Bom;
using subedit::core::isValidUtf8;
using subedit::core::Newline;
using subedit::core::scanNewlines;
using subedit::core::withoutUtf8Bom;

constexpr std::string_view kBom = "\xEF\xBB\xBF";

} // namespace

TEST_CASE("plain ASCII is valid UTF-8", "[format][encoding]") {
    CHECK(isValidUtf8("Hello, world."));
    CHECK(isValidUtf8(""));
}

TEST_CASE("accented and non-latin text is valid UTF-8", "[format][encoding]") {
    CHECK(isValidUtf8("Été à Noël"));
    CHECK(isValidUtf8("日本語"));
    CHECK(isValidUtf8("🎬"));
}

TEST_CASE("a lone continuation byte is not valid UTF-8", "[format][encoding]") {
    CHECK_FALSE(isValidUtf8("\x80"));
    CHECK_FALSE(isValidUtf8("abc\xBF"));
}

TEST_CASE("a truncated sequence is not valid UTF-8", "[format][encoding]") {
    // A file cut in the middle of a character, or read as UTF-8 when it was
    // Latin-1 — the common way a subtitle file arrives broken.
    CHECK_FALSE(isValidUtf8("\xC3"));
    CHECK_FALSE(isValidUtf8("\xE9t\xE9"));
}

TEST_CASE("an overlong encoding is not valid UTF-8", "[format][encoding]") {
    // Two bytes for a character that fits in one: accepted once, and a source
    // of security holes ever since.
    CHECK_FALSE(isValidUtf8("\xC0\xAF"));
}

TEST_CASE("a code point beyond Unicode is not valid UTF-8", "[format][encoding]") {
    // Four bytes can express more than Unicode defines; what lies past
    // U+10FFFF has no character to be.
    CHECK_FALSE(isValidUtf8("\xF4\x90\x80\x80"));
}

TEST_CASE("a surrogate half is not valid UTF-8", "[format][encoding]") {
    CHECK_FALSE(isValidUtf8("\xED\xA0\x80"));
}

TEST_CASE("a byte order mark is seen and taken off", "[format][encoding]") {
    const std::string content = std::string{kBom} + "WEBVTT\n";

    CHECK(hasUtf8Bom(content));
    CHECK(withoutUtf8Bom(content) == "WEBVTT\n");
}

TEST_CASE("content without a mark is left alone", "[format][encoding]") {
    CHECK_FALSE(hasUtf8Bom("WEBVTT\n"));
    CHECK(withoutUtf8Bom("WEBVTT\n") == "WEBVTT\n");
    CHECK_FALSE(hasUtf8Bom(""));
}

TEST_CASE("line feeds alone are Unix endings", "[format][encoding]") {
    const auto scan = scanNewlines("un\ndeux\ntrois\n");

    CHECK(scan.newline == Newline::Lf);
    CHECK_FALSE(scan.mixed);
}

TEST_CASE("carriage return and line feed are Windows endings", "[format][encoding]") {
    const auto scan = scanNewlines("un\r\ndeux\r\n");

    CHECK(scan.newline == Newline::CrLf);
    CHECK_FALSE(scan.mixed);
}

TEST_CASE("lone carriage returns are classic Mac endings", "[format][encoding]") {
    const auto scan = scanNewlines("un\rdeux\r");

    CHECK(scan.newline == Newline::Cr);
    CHECK_FALSE(scan.mixed);
}

TEST_CASE("the most frequent ending wins, and the mixture is noticed", "[format][encoding]") {
    // A file assembled from two others, which happens more often than it
    // should. The dominant ending is kept and the mixture reported.
    const auto scan = scanNewlines("un\r\ndeux\r\ntrois\nquatre\r\n");

    CHECK(scan.newline == Newline::CrLf);
    CHECK(scan.mixed);
    CHECK(scan.mixedAtLine == 3);
}

TEST_CASE("a file without a single ending is taken as Unix", "[format][encoding]") {
    // Nothing to go on, and LF is what the project writes.
    const auto scan = scanNewlines("une seule ligne");

    CHECK(scan.newline == Newline::Lf);
    CHECK_FALSE(scan.mixed);
}
