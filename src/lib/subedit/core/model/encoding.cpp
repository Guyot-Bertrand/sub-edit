#include <subedit/core/model/encoding.hpp>

#include <unicode/ucnv.h>
#include <unicode/utypes.h>

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

/// The marks a file can carry, and the only ones.
///
/// A byte order mark exists for the Unicode encodings and for no other; Gaupol
/// looks for the same list in `detect_bom`, minus the UTF-32 pair this phase
/// does not offer. This is knowledge about what a mark *is*, and not the table
/// of encodings the phase refuses to write — that one is ICU's, and it holds
/// ninety-seven names this file never sees.
[[nodiscard]] std::string_view markBytesOf(std::string_view charset) {
    if (charset == "UTF-8")
        return "\xEF\xBB\xBF";
    if (charset == "UTF-16LE")
        return "\xFF\xFE";
    if (charset == "UTF-16BE")
        return "\xFE\xFF";
    return {};
}

/// The name ICU settles on for `name`, or nothing if it converts nothing.
///
/// Two names for one converter — `cp1252` and `windows-1252` — have to become
/// one value, or equality would answer "different" about the same encoding.
///
/// **Neither of the two spellings ICU offers is the right one on its own**, and
/// the order below is what a user ends up reading. `ucnv_getName` answers with
/// ICU's internal name — `windows-1252` is `ibm-5348_P100-1997` there, correct
/// and unrecognisable. The IANA registry names Latin-1 `ISO_8859-1:1987`, which
/// is its official name and nobody's. The MIME name is the one seen in the
/// wild — `ISO-8859-1`, `KOI8-R`, `Shift_JIS` — and it is preferred; IANA
/// covers what has no MIME name, `windows-1252` among them.
[[nodiscard]] std::optional<std::string_view> canonicalNameOf(const char* name) {
    // One status for both calls, as ICU's own convention has it: `ucnv_getName`
    // returns at once on a status that already failed, and `ucnv_close` takes a
    // null pointer. A single check then answers for the two, and there is no
    // second branch for a test to leave unvisited.
    UErrorCode status = U_ZERO_ERROR;
    UConverter* converter = ucnv_open(name, &status);
    const char* internalName = ucnv_getName(converter, &status);
    ucnv_close(converter);
    if (failed(status))
        return std::nullopt;

    for (const char* standard : {"MIME", "IANA"}) {
        UErrorCode standardStatus = U_ZERO_ERROR;
        if (const char* named = ucnv_getStandardName(internalName, standard, &standardStatus);
            !failed(standardStatus) && named != nullptr)
            return std::string_view{named};
    }

    return std::string_view{internalName};
}

} // namespace

Encoding::Encoding(std::string_view charset, ByteOrderMark mark)
    : m_charset(charset), m_markBytes(markBytesOf(charset)), m_mark(mark) {}

std::optional<Encoding> Encoding::create(std::string_view name, ByteOrderMark mark) {
    // An empty name is ICU's way of asking for the platform's default
    // converter, which is exactly the "encoding of the locale, silently" that
    // the phase refuses — see the analysis of Gaupol in the phase 8 spec.
    if (name.empty())
        return std::nullopt;

    // A terminated copy: ICU takes a C string, and a `string_view` promises no
    // terminator — the caller's name may well be a slice of a command line.
    const std::string terminated{name};

    const std::optional<std::string_view> canonical = canonicalNameOf(terminated.c_str());
    if (!canonical.has_value())
        return std::nullopt;

    return Encoding{*canonical, mark};
}

} // namespace subedit::core
