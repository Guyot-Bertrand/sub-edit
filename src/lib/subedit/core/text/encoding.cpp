#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/text/encoding.hpp>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

/// Tells whether an ICU call failed.
///
/// `U_FAILURE` answers a `UBool`, which is a `signed char`; reading it as a
/// condition is a conversion clang-tidy refuses, and rightly — it is the same
/// kind of implicit narrowing that hides a bug elsewhere. Written once here
/// rather than at every call.
[[nodiscard]] bool failed(UErrorCode status) {
    return U_FAILURE(status) != 0;
}

/// How much of the conversion ICU carries between the two converters at a time.
///
/// A converter turns bytes into UTF-16 and another turns UTF-16 into UTF-8;
/// `ucnv_convertEx` runs both against a buffer of its caller's, in slices. Four
/// thousand units is a few kilobytes on the stack, and nothing here is
/// proportional to the size of the file.
constexpr int32_t kPivotUnits = 4096;

/// The room the first attempt gives the text, beyond the size of the bytes.
///
/// A guess and not a bound: a byte of Latin-1 becomes two of UTF-8, and the
/// loop below grows the buffer for as long as ICU says it is short. Guessing
/// the size of the file is right for UTF-8, which is what most files are.
constexpr std::size_t kFirstGuessSlack = 16;

/// The most bytes one character takes in UTF-8.
constexpr std::size_t kLongestUtf8Character = 4;

/// How many UTF-16 code units of the offending character ICU is asked for.
constexpr std::size_t kInvalidUnits = 8;

/// The one charset both conversions have on their other side: text is UTF-8
/// everywhere above this file.
constexpr const char* kUtf8Charset = "UTF-8";

/// ICU's own boolean, whose `TRUE` and `FALSE` macros are gone since ICU 68.
constexpr UBool kIcuTrue = 1;
constexpr UBool kIcuFalse = 0;

/// A converter, closed wherever the conversion leaves off.
///
/// `ucnv_open` allocates and `ucnv_close` releases; naming that pair a
/// `unique_ptr` is what keeps the ways out of the loop below from each having
/// to remember two closes.
struct ConverterCloser {
    void operator()(UConverter* converter) const { ucnv_close(converter); }
};

using ConverterHandle = std::unique_ptr<UConverter, ConverterCloser>;

/// The charset detector, closed the same way.
struct DetectorCloser {
    void operator()(UCharsetDetector* detector) const { ucsdet_close(detector); }
};

using DetectorHandle = std::unique_ptr<UCharsetDetector, DetectorCloser>;

} // namespace

namespace {

/// Runs `bytes` through the two converters, and hands back what came out.
///
/// **One pass, and it was measured.** Converting into UTF-16 and rendering that
/// took 1.85 ms on a four-thousand-subtitle file where this takes 0.20 ms:
/// `ucnv_convertEx` chains the two converters itself, over a pivot buffer that
/// does not grow with the file.
///
/// Nothing is substituted — the callers set the callback that says so before
/// handing their converters over, and an empty answer means the conversion
/// stopped on something it could not carry.
[[nodiscard]] std::optional<std::string>
convertWith(UConverter* from, UConverter* to, std::string_view bytes) {
    std::string out(bytes.size() + kFirstGuessSlack, '\0');
    const char* source = bytes.data();
    const char* sourceEnd = source + bytes.size();
    char* target = out.data();

    std::array<UChar, kPivotUnits> pivot{};
    UChar* pivotSource = pivot.data();
    UChar* pivotTarget = pivot.data();
    UBool resetting = kIcuTrue;

    while (true) {
        UErrorCode converting = U_ZERO_ERROR;
        ucnv_convertEx(to,
                       from,
                       &target,
                       out.data() + out.size(),
                       &source,
                       sourceEnd,
                       pivot.data(),
                       &pivotSource,
                       &pivotTarget,
                       pivot.data() + pivot.size(),
                       resetting,
                       kIcuTrue,
                       &converting);
        resetting = kIcuFalse;

        if (converting == U_BUFFER_OVERFLOW_ERROR) {
            // What is written so far, kept as an offset: growing the string
            // moves the buffer, and the pointer into it with it.
            const auto written = static_cast<std::size_t>(target - out.data());
            out.resize(out.size() * 2);
            target = out.data() + written;
            continue;
        }

        if (failed(converting))
            return std::nullopt;
        break;
    }

    out.resize(static_cast<std::size_t>(target - out.data()));
    return out;
}

/// What `converter` choked on, as UTF-8, for a message that can name it.
///
/// ICU keeps the offending code units in the converter itself — that is what
/// `ucnv_getInvalidUChars` is for — so the character does not have to be hunted
/// down a second time by re-encoding the text one character at a time.
[[nodiscard]] std::string invalidCharacterOf(UConverter* converter) {
    // A handful of code units: what stops a conversion is one character, and a
    // surrogate pair is the longest it can be.
    std::array<UChar, kInvalidUnits> units{};
    auto length = static_cast<int8_t>(units.size());

    // One status for both calls, as everywhere else here: `u_strToUTF8` returns
    // at once on a status that already failed, leaving `written` at zero — and
    // an empty answer is what either failure comes to anyway.
    UErrorCode status = U_ZERO_ERROR;
    ucnv_getInvalidUChars(converter, units.data(), &length, &status);

    std::string text(units.size() * kLongestUtf8Character, '\0');
    int32_t written = 0;
    u_strToUTF8(
        text.data(), static_cast<int32_t>(text.size()), &written, units.data(), length, &status);

    text.resize(failed(status) ? 0 : static_cast<std::size_t>(written));
    return text;
}

} // namespace

std::optional<std::string> decodeToUtf8(std::string_view bytes, const Encoding& encoding) {
    const std::string charset{encoding.charset()};

    // **Three calls, one status, and no check between them.** That is ICU's own
    // convention — every one of its functions returns at once on a status that
    // already failed — and it is what keeps this function free of a branch no
    // test could reach: an `Encoding` exists only because ICU accepted its
    // name, so opening its converter cannot fail. Were it to anyway, the null
    // converter would reach `ucnv_convertEx`, which refuses it with
    // `U_ILLEGAL_ARGUMENT_ERROR` — the failure below, which tests do reach.
    UErrorCode status = U_ZERO_ERROR;
    const ConverterHandle from{ucnv_open(charset.c_str(), &status)};
    const ConverterHandle to{ucnv_open(kUtf8Charset, &status)};

    // Stop rather than substitute. This one call is the difference between
    // refusing a file read in the wrong encoding and handing back its text with
    // every accent turned into a replacement character.
    ucnv_setToUCallBack(from.get(), UCNV_TO_U_CALLBACK_STOP, nullptr, nullptr, nullptr, &status);

    return convertWith(from.get(), to.get(), bytes);
}

std::expected<std::string, UnwritableCharacter> encodeFromUtf8(std::string_view text,
                                                               const Encoding& encoding) {
    const std::string charset{encoding.charset()};

    UErrorCode status = U_ZERO_ERROR;
    const ConverterHandle from{ucnv_open(kUtf8Charset, &status)};
    const ConverterHandle to{ucnv_open(charset.c_str(), &status)};

    // The same refusal, the other way round: ICU writes `?` for a character the
    // encoding has no room for, and a `?` written to disk over the file it came
    // from is text lost in silence.
    ucnv_setFromUCallBack(to.get(), UCNV_FROM_U_CALLBACK_STOP, nullptr, nullptr, nullptr, &status);

    const std::optional<std::string> bytes = convertWith(from.get(), to.get(), text);
    if (!bytes.has_value())
        return std::unexpected(UnwritableCharacter{.character = invalidCharacterOf(to.get())});

    return *bytes;
}

std::optional<Encoding> byteOrderMarkAt(std::string_view bytes) {
    // The three the phase reads. UTF-32 is left out on purpose — Gaupol detects
    // it and offers it nowhere, and no subtitle file carries it.
    for (const Encoding& marked : {Encoding::utf8(ByteOrderMark::Present),
                                   Encoding::utf16Le(ByteOrderMark::Present),
                                   Encoding::utf16Be(ByteOrderMark::Present)})
        if (startsWithByteOrderMark(bytes, marked))
            return marked;

    return std::nullopt;
}

std::optional<DetectedEncoding> detectEncoding(std::string_view bytes) {
    // The mark first, always.
    if (const std::optional<Encoding> declared = byteOrderMarkAt(bytes))
        return DetectedEncoding{.encoding = *declared, .choice = EncodingChoice::ByteOrderMark};

    // **Bytes that decode as UTF-8 are UTF-8, and ICU is not asked.** Two
    // arguments meet here. On a file of plain ASCII, every encoding on offer
    // reads the same characters, so the question has no stake and UTF-8 is the
    // answer that keeps the file portable. On a file with multi-byte sequences,
    // UTF-8 is *structurally* verifiable — a text in another encoding forms
    // valid sequences by accident about as often as a coin lands on its edge —
    // where every other answer rests on letter frequencies.
    //
    // Measured, and the margin is why this is not left to ICU: on a two-accent
    // file of the corpus, its detector ranks ISO-8859-1 at 81 and UTF-8 at 80.
    // One point of frequency statistics against a proof — and the file would
    // have come back with `Ã©` where its accents were.
    const Encoding utf8 = Encoding::utf8(ByteOrderMark::Absent);
    if (decodeToUtf8(bytes, utf8).has_value())
        return DetectedEncoding{.encoding = utf8, .choice = EncodingChoice::Detected};

    UErrorCode status = U_ZERO_ERROR;
    const DetectorHandle detector{ucsdet_open(&status)};
    ucsdet_setText(detector.get(), bytes.data(), static_cast<int32_t>(bytes.size()), &status);

    // **Every match ICU weighed, in its order, and the first that actually
    // decodes.** Its best answer is a ranking and not a verdict, and the top of
    // that ranking can be an encoding under which the file does not decode at
    // all — a short Cyrillic text is read as UTF-16BE, which an odd number of
    // bytes then refuses. Proposing an encoding that does not decode is
    // proposing nothing; the ranking is kept, the impossible answers are
    // dropped.
    int32_t count = 0;
    const UCharsetMatch** matches = ucsdet_detectAll(detector.get(), &count, &status);

    for (int32_t rank = 0; matches != nullptr && rank < count; ++rank) {
        // The name is taken with a status of its own, and a match that will not
        // give one leaves `name` null rather than a reason nobody would read.
        UErrorCode naming = U_ZERO_ERROR;
        const char* name = ucsdet_getName(matches[rank], &naming);

        const std::optional<Encoding> proposed =
            name == nullptr ? std::nullopt : Encoding::create(name, ByteOrderMark::Absent);
        if (proposed.has_value() && decodeToUtf8(bytes, *proposed).has_value())
            return DetectedEncoding{.encoding = *proposed, .choice = EncodingChoice::Detected};
    }

    return std::nullopt;
}

bool startsWithByteOrderMark(std::string_view bytes, const Encoding& encoding) {
    const std::string_view mark = encoding.byteOrderMarkBytes();
    return !mark.empty() && bytes.starts_with(mark);
}

std::string_view withoutByteOrderMark(std::string_view bytes, const Encoding& encoding) {
    if (!startsWithByteOrderMark(bytes, encoding))
        return bytes;
    return bytes.substr(encoding.byteOrderMarkBytes().size());
}

NewlineScan scanNewlines(std::string_view content) {
    int carriageReturnLineFeed = 0;
    int lineFeed = 0;
    int carriageReturn = 0;

    NewlineScan scan;
    Newline first = Newline::Lf;
    bool seenAny = false;
    int line = 0;

    for (std::size_t index = 0; index < content.size(); ++index) {
        Newline kind = Newline::Lf;
        if (content[index] == '\r') {
            const bool followed = index + 1 < content.size() && content[index + 1] == '\n';
            kind = followed ? Newline::CrLf : Newline::Cr;
            if (followed)
                ++index;
        } else if (content[index] != '\n') {
            continue;
        }

        ++line;
        switch (kind) {
        case Newline::CrLf:
            ++carriageReturnLineFeed;
            break;
        case Newline::Lf:
            ++lineFeed;
            break;
        case Newline::Cr:
            ++carriageReturn;
            break;
        }

        if (!seenAny) {
            first = kind;
            seenAny = true;
        } else if (kind != first && !scan.mixed) {
            scan.mixed = true;
            scan.mixedAtLine = line;
        }
    }

    if (!seenAny)
        return scan;

    scan.newline = Newline::Lf;
    if (carriageReturnLineFeed >= lineFeed && carriageReturnLineFeed >= carriageReturn)
        scan.newline = Newline::CrLf;
    else if (carriageReturn > lineFeed)
        scan.newline = Newline::Cr;

    return scan;
}

} // namespace subedit::core
