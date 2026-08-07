#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/encoding.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_rip_reader.hpp>
#include <subedit/core/format/sub_rip_writer.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_format.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/web_vtt_reader.hpp>
#include <subedit/core/format/web_vtt_writer.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

constexpr std::string_view kUtf8Bom = "\xEF\xBB\xBF";

[[nodiscard]] std::expected<ReadResult, ReadError> readAs(SubtitleFormat format,
                                                          std::string_view content) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return SubRipReader{}.read(content);
    case SubtitleFormat::WebVtt:
        return WebVttReader{}.read(content);
    }
    std::unreachable();
}

} // namespace

std::expected<ReadResult, ReadError> readSubtitles(std::string_view content) {
    // The encoding comes first: what follows reads text, and text that is not
    // valid UTF-8 is not text.
    if (!isValidUtf8(content))
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::InvalidUtf8,
            .detail = "octets invalides en UTF-8",
        });

    const bool hadBom = hasUtf8Bom(content);
    const std::string_view text = withoutUtf8Bom(content);

    const std::optional<SubtitleFormat> format = detectFormat(text);
    if (!format.has_value())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::UnknownFormat,
            .detail = "aucun format reconnu",
        });

    std::expected<ReadResult, ReadError> result = readAs(*format, text);
    if (!result.has_value())
        return result;

    result->hadUtf8Bom = hadBom;

    const NewlineScan scan = scanNewlines(text);
    result->newline = scan.newline;
    if (scan.mixed)
        result->diagnostics.push_back(Diagnostic{
            .severity = Severity::Recovered,
            .line = scan.mixedAtLine,
            .kind = DiagnosticKind::MixedNewlines,
            .detail = {},
        });

    return result;
}

std::string writeSubtitles(SubtitleFormat format, const WriteRequest& request, Utf8Bom bom) {
    std::string out;
    if (bom == Utf8Bom::Present)
        out += kUtf8Bom;

    switch (format) {
    case SubtitleFormat::SubRip:
        out += SubRipWriter{}.write(request);
        return out;
    case SubtitleFormat::WebVtt:
        out += WebVttWriter{}.write(request);
        return out;
    }
    std::unreachable();
}

} // namespace subedit::core
