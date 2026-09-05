#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/text/encoding.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::decodeToUtf8;
using subedit::core::DetectedEncoding;
using subedit::core::detectEncoding;
using subedit::core::encodeFromUtf8;
using subedit::core::Encoding;
using subedit::core::EncodingChoice;
using subedit::core::EncodingRefusal;
using subedit::core::Newline;
using subedit::core::scanNewlines;
using subedit::core::startsWithByteOrderMark;
using subedit::core::UnwritableCharacter;
using subedit::core::withoutByteOrderMark;

constexpr std::string_view kBom = "\xEF\xBB\xBF";

const Encoding kUtf8 = Encoding::utf8(ByteOrderMark::Absent);

/// What the detection proposes for these bytes, or a failed test.
///
/// A function rather than a dereference at each call: the proposal may not
/// exist, and a test that reaches through the option without asking says the
/// opposite of what the type promises.
[[nodiscard]] DetectedEncoding proposedFor(std::string_view bytes);

/// The encoding of that name, or a failed test — a fixture is not the place to
/// find out that ICU is missing a converter.
[[nodiscard]] Encoding named(std::string_view name) {
    const std::expected<Encoding, EncodingRefusal> encoding =
        Encoding::create(name, ByteOrderMark::Absent);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas l'encodage " + std::string{name});
        return kUtf8;
    }
    return *encoding;
}

DetectedEncoding proposedFor(std::string_view bytes) {
    const std::optional<DetectedEncoding> detected = detectEncoding(bytes);
    if (!detected.has_value()) {
        FAIL("aucun encodage proposé pour ces octets");
        return DetectedEncoding{.encoding = kUtf8, .choice = EncodingChoice::Detected};
    }
    return *detected;
}

} // namespace

TEST_CASE("plain ASCII decodes as UTF-8", "[format][encoding]") {
    CHECK(decodeToUtf8("Hello, world.", kUtf8) == "Hello, world.");
    CHECK(decodeToUtf8("", kUtf8) == "");
}

TEST_CASE("text beyond ASCII decodes as UTF-8", "[format][encoding]") {
    CHECK(decodeToUtf8("Été à Noël", kUtf8) == "Été à Noël");
    CHECK(decodeToUtf8("日本語", kUtf8) == "日本語");
    CHECK(decodeToUtf8("🎬", kUtf8) == "🎬");
}

TEST_CASE("a lone continuation byte does not decode as UTF-8", "[format][encoding]") {
    CHECK_FALSE(decodeToUtf8("\x80", kUtf8).has_value());
    CHECK_FALSE(decodeToUtf8("abc\xBF", kUtf8).has_value());
}

TEST_CASE("a truncated sequence does not decode as UTF-8", "[format][encoding]") {
    CHECK_FALSE(decodeToUtf8("\xC3", kUtf8).has_value());
    CHECK_FALSE(decodeToUtf8("\xE9t\xE9", kUtf8).has_value());
}

TEST_CASE("an overlong sequence does not decode as UTF-8", "[format][encoding]") {
    // Two bytes for a character that takes one. Accepting them is how a check
    // on a shorter form gets walked past.
    CHECK_FALSE(decodeToUtf8("\xC0\xAF", kUtf8).has_value());
}

TEST_CASE("a code point beyond Unicode does not decode as UTF-8", "[format][encoding]") {
    CHECK_FALSE(decodeToUtf8("\xF4\x90\x80\x80", kUtf8).has_value());
}

TEST_CASE("a surrogate half does not decode as UTF-8", "[format][encoding]") {
    CHECK_FALSE(decodeToUtf8("\xED\xA0\x80", kUtf8).has_value());
}

TEST_CASE("the same bytes are two different texts in two encodings", "[format][encoding]") {
    // The reason an encoding cannot be guessed from the bytes alone, in one
    // line: 0xC9 0x74 0xE9 reads "Été" in Latin-1 and "иtИ" in KOI8-R, and
    // nothing in the bytes says which of the two was meant.
    const std::string bytes = "\xC9t\xE9";

    CHECK(decodeToUtf8(bytes, named("iso-8859-1")) == "Été");
    CHECK(decodeToUtf8(bytes, named("koi8-r")) == "иtИ");
}

TEST_CASE("bytes that decode nowhere are refused rather than patched up", "[format][encoding]") {
    // ICU substitutes U+FFFD for what it cannot map unless it is told to stop,
    // and a file full of replacement characters is a file nobody refuses.
    CHECK_FALSE(decodeToUtf8("\xFF\xFE\x00", named("utf-8")).has_value());
}

TEST_CASE("text goes back to the bytes it came from", "[format][encoding]") {
    // The round trip, at the level where it is decided: the same text, in the
    // encoding it was read in, is the same bytes.
    const std::string bytes = "\xE9t\xE9 \xE0 No\xEBl\n";
    const Encoding latin = named("iso-8859-1");

    const std::optional<std::string> text = decodeToUtf8(bytes, latin);

    CHECK(encodeFromUtf8(text.value_or(std::string{}), latin) == bytes);
}

TEST_CASE("a character the encoding cannot write is named, not replaced", "[format][encoding]") {
    // ICU writes `?` for what it cannot map unless it is told to stop. A `?` is
    // a character the user did not type, written over the file they had.
    const std::expected<std::string, UnwritableCharacter> written =
        encodeFromUtf8("Przyszedł późno", named("iso-8859-1"));

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error() == UnwritableCharacter{.character = "ł"});
}

TEST_CASE("an encoding that can write everything writes everything", "[format][encoding]") {
    CHECK(encodeFromUtf8("日本語", kUtf8) == "日本語");
}

TEST_CASE("a mark settles the encoding, and nothing else is weighed", "[format][encoding]") {
    // The only thing a subtitle file declares about its own encoding. What
    // follows it is not weighed at all — the question is answered.
    const DetectedEncoding utf8 = proposedFor(std::string{kBom} + "WEBVTT\n");
    const DetectedEncoding little = proposedFor("\xFF\xFEW\0E\0");

    CHECK(utf8.encoding == Encoding::utf8(ByteOrderMark::Present));
    CHECK(utf8.choice == EncodingChoice::ByteOrderMark);
    CHECK(little.encoding == Encoding::utf16Le(ByteOrderMark::Present));
    CHECK(little.choice == EncodingChoice::ByteOrderMark);
}

TEST_CASE("bytes that decode as UTF-8 are UTF-8", "[format][encoding]") {
    // Two files nothing declares: one of plain ASCII, where every encoding
    // agrees and UTF-8 is the portable answer; one with multi-byte sequences,
    // where UTF-8 is structurally verifiable. ICU's detector ranks ISO-8859-1
    // above UTF-8 on the second, by one point of letter frequencies.
    for (const std::string_view content : {"Nothing but ASCII here.\n", "Été à Noël\n"}) {
        INFO("contenu : " << content);
        const DetectedEncoding detected = proposedFor(content);

        CHECK(detected.encoding == kUtf8);
        CHECK(detected.choice == EncodingChoice::Detected);
    }
}

TEST_CASE("bytes nothing declares are weighed, and the answer is a proposal",
          "[format][encoding]") {
    const DetectedEncoding detected = proposedFor("\xE9t\xE9 \xE0 No\xEBl\n");

    CHECK(detected.encoding == named("iso-8859-1"));
    CHECK(detected.choice == EncodingChoice::Detected);
}

TEST_CASE("a script buried under ASCII is still weighed", "[format][encoding]") {
    // **The file that cost the most, and the whole of issue #310.** Twenty
    // English subtitles and one Cyrillic card, written in KOI8-R: nineteen
    // bytes out of thirteen hundred carry anything at all. Weighing the file
    // whole, ICU ranked ISO-8859-1 at 66 — labelled "en" — and KOI8-R at 2, so
    // the card came back as accented Latin. `uchardet` answered KOI8-R on the
    // same bytes, which is what says this was never a want of information.
    //
    // Lines of plain ASCII read the same under every candidate, so they cannot
    // tell the candidates apart; what they can do, and did, is outweigh the
    // lines that do.
    std::string content;
    for (int index = 1; index <= 20; ++index)
        content += std::to_string(index) + "\n00:00:0" + std::to_string(index % 10) +
                   ",000 --> 00:00:09,000\nNobody answered the telephone.\n\n";
    content += "21\n00:00:42,000 --> 00:00:43,000\n\xF0\xD4\xCF \xC4\xCC\xC9\xCC\xCF"
               "\xD3\xD8 \xCE\xC5\xC4\xCF\xCC\xC7\xCF.\n\n";

    const DetectedEncoding detected = proposedFor(content);

    CHECK(detected.encoding == named("koi8-r"));
    CHECK(detected.choice == EncodingChoice::Detected);
}

TEST_CASE("UTF-16 without a mark is weighed whole", "[format][encoding]") {
    // **The one shape the line filter must not touch.** A NUL says the bytes
    // are a wide encoding, where `0x0a` is half a code unit rather than a line
    // ending, and where a line of plain ASCII is a run of `X\0X\0` that carries
    // no high byte — so dropping ASCII lines would drop the file. Both sides
    // are checked: the filter would take the two apart, not merely blur them.
    for (const std::string_view charset : {"utf-16le", "utf-16be"}) {
        INFO("encodage : " << charset);
        const Encoding wide = named(charset);
        const std::expected<std::string, UnwritableCharacter> bytes =
            encodeFromUtf8("1\n00:00:02,000 --> 00:00:03,000\nOù êtes-vous passé ?\n\n", wide);
        REQUIRE(bytes.has_value());

        CHECK(proposedFor(*bytes).encoding == wide);
    }
}

TEST_CASE("an encoding under which the file does not decode is not proposed",
          "[format][encoding]") {
    // ICU's best answer is a ranking, not a verdict: it puts UTF-32BE at the
    // top of these bytes, which a length that is not a multiple of four then
    // refuses. Proposing an encoding that does not decode is proposing nothing.
    CHECK_FALSE(detectEncoding(std::string{"\x00\x01\x02\xFF\xFE\x80\x81", 7}).has_value());
}

TEST_CASE("bytes that propose nothing propose nothing", "[format][encoding]") {
    // One high byte, and no encoding to hang a proposal on. Reading falls back
    // to UTF-8 from here, and fails — which is the honest answer.
    CHECK_FALSE(detectEncoding("\xFF").has_value());
}

TEST_CASE("a byte order mark is seen and taken off", "[format][encoding]") {
    const std::string content = std::string{kBom} + "WEBVTT\n";

    CHECK(startsWithByteOrderMark(content, kUtf8));
    CHECK(withoutByteOrderMark(content, kUtf8) == "WEBVTT\n");
}

TEST_CASE("content without a mark is left alone", "[format][encoding]") {
    CHECK_FALSE(startsWithByteOrderMark("WEBVTT\n", kUtf8));
    CHECK(withoutByteOrderMark("WEBVTT\n", kUtf8) == "WEBVTT\n");
    CHECK_FALSE(startsWithByteOrderMark("", kUtf8));
}

TEST_CASE("each encoding looks for its own mark", "[format][encoding]") {
    // The two bytes of a UTF-16 mark are the byte order itself, and one file's
    // mark is another file's first character: 0xFF 0xFE opens a UTF-16LE file
    // and is not a mark in UTF-8 at all.
    const std::string little = "\xFF\xFE";

    CHECK(startsWithByteOrderMark(little, Encoding::utf16Le(ByteOrderMark::Absent)));
    CHECK_FALSE(startsWithByteOrderMark(little, Encoding::utf16Be(ByteOrderMark::Absent)));
    CHECK_FALSE(startsWithByteOrderMark(little, kUtf8));
}

TEST_CASE("an encoding that carries no mark never finds one", "[format][encoding]") {
    const std::string content = std::string{kBom} + "WEBVTT\n";

    CHECK_FALSE(startsWithByteOrderMark(content, named("windows-1252")));
    CHECK(withoutByteOrderMark(content, named("windows-1252")) == content);
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
