#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/format_detection.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/sub_rip_reader.hpp>
#include <subedit/core/format/sub_rip_writer.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/web_vtt_reader.hpp>
#include <subedit/core/format/web_vtt_writer.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/encoding.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

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

std::expected<ReadResult, ReadError> readSubtitles(std::string_view content,
                                                   const Encoding& encoding) {
    // The mark comes off first, and the bytes are what decide it was there: a
    // converter would not take it off — to it, it is a zero-width space at the
    // head of the first subtitle.
    const Encoding read = encoding.withByteOrderMark(startsWithByteOrderMark(content, encoding)
                                                         ? ByteOrderMark::Present
                                                         : ByteOrderMark::Absent);

    // The encoding comes next: what follows reads text, and bytes that do not
    // decode are not text.
    const std::optional<std::string> decoded =
        decodeToUtf8(withoutByteOrderMark(content, read), read);
    if (!decoded.has_value())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::Undecodable,
            .detail = read.name(),
        });

    const std::string_view text = *decoded;

    const std::optional<SubtitleFormat> format = detectFormat(text);
    if (!format.has_value())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::UnknownFormat,
            .detail = "aucun format reconnu",
        });

    std::expected<ReadResult, ReadError> result = readAs(*format, text);
    if (!result.has_value())
        return result;

    result->encoding = read;

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

std::expected<ReadResult, ReadError> readSubtitles(std::string_view content) {
    return readSubtitles(content, Encoding::utf8(ByteOrderMark::Absent));
}

std::string writeSubtitles(SubtitleFormat format, const WriteRequest& request) {
    std::string out;
    if (request.encoding.byteOrderMark() == ByteOrderMark::Present)
        out += request.encoding.byteOrderMarkBytes();

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
