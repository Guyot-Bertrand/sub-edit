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
#include <subedit/core/format/write_error.hpp>
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

/// Whether the reading has to say which encoding it settled on.
///
/// **Not every guess is worth a line.** A diagnostic says what happened that
/// the user cannot see, and reading a UTF-8 file as UTF-8 is not an event: it
/// is what every file was read as until this phase, and saying it on every
/// opening would put a panel under the table of every ordinary document — the
/// panel exists to be available, not to stand between the user and their
/// subtitles.
///
/// What is worth a line is the file that would have been refused, or read as
/// nonsense, before: one whose encoding was guessed to be something else. A
/// mark is not a guess at all, and says nothing anyone can act on.
[[nodiscard]] bool saysSomethingNew(const std::optional<DetectedEncoding>& detected) {
    return detected.has_value() && detected->choice == EncodingChoice::Detected &&
           detected->encoding != Encoding::utf8(ByteOrderMark::Absent);
}

[[nodiscard]] std::string textOf(SubtitleFormat format, const WriteRequest& request) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return SubRipWriter{}.write(request);
    case SubtitleFormat::WebVtt:
        return WebVttWriter{}.write(request);
    }
    std::unreachable();
}

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
    // **The mark wins, even against the encoding it was given.** It is the only
    // thing a subtitle file declares about itself, and reading it otherwise
    // than it declares would be obeying a caller against the file's own word.
    // Gaupol settles it the same way — `open_main` starts the reading again
    // with the encoding of the mark — and says nothing; here the gap between
    // what was asked and what was done is a diagnostic, below.
    //
    // The mark also comes off before anything else: a converter would not take
    // it off, since to it, it is a zero-width space at the head of the first
    // subtitle.
    const std::optional<Encoding> declared = byteOrderMarkAt(content);
    const Encoding read = declared.value_or(encoding.withByteOrderMark(ByteOrderMark::Absent));

    // The encoding comes next: what follows reads text, and bytes that do not
    // decode are not text.
    const std::optional<std::string> decoded =
        decodeToUtf8(withoutByteOrderMark(content, read), read);
    if (!decoded.has_value())
        return std::unexpected(ReadError{
            .kind = ReadErrorKind::Undecodable,
            .detail = std::string{read.charset()},
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

    if (declared.has_value() && declared->charset() != encoding.charset())
        // First, because it happened first: the mark was read before a single
        // line was.
        result->diagnostics.insert(result->diagnostics.begin(),
                                   Diagnostic{
                                       .severity = Severity::Recovered,
                                       .line = kWholeFile,
                                       .kind = DiagnosticKind::MarkOverridesEncoding,
                                       // The charset alone: that a mark is
                                       // there is the whole subject of the
                                       // sentence, so `-sig` would say it
                                       // twice.
                                       .detail = std::string{read.charset()},
                                   });

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
    const std::optional<DetectedEncoding> detected = detectEncoding(content);

    // UTF-8 when the bytes propose nothing — an empty file, or one ICU weighed
    // without finding anything to weigh. It is the encoding this tool wrote
    // every file in until phase 8, and the one a file that says nothing is most
    // likely to be.
    const Encoding encoding = detected ? detected->encoding : Encoding::utf8(ByteOrderMark::Absent);

    std::expected<ReadResult, ReadError> result = readSubtitles(content, encoding);
    if (!result.has_value())
        return result;

    if (saysSomethingNew(detected))
        // First, because it happened first: the encoding was settled before a
        // single line was read.
        result->diagnostics.insert(result->diagnostics.begin(),
                                   Diagnostic{
                                       .severity = Severity::Recovered,
                                       .line = kWholeFile,
                                       // The charset and not a worded name: a
                                       // diagnostic detail is the datum the
                                       // sentence is about, and a guessed
                                       // encoding never carries a mark — a mark
                                       // is a declaration, so nothing was
                                       // guessed.
                                       .kind = DiagnosticKind::GuessedEncoding,
                                       .detail = std::string{result->encoding.charset()},
                                   });

    return result;
}

std::expected<std::string, WriteError> writeSubtitles(SubtitleFormat format,
                                                      const WriteRequest& request) {
    const std::expected<std::string, UnwritableCharacter> body =
        encodeFromUtf8(textOf(format, request), request.encoding);
    if (!body.has_value())
        return std::unexpected(WriteError{
            .kind = WriteErrorKind::Unencodable,
            .detail = body.error().character,
        });

    // The mark is bytes and not text: it goes on after the conversion, as it
    // came off before it.
    std::string out;
    if (request.encoding.byteOrderMark() == ByteOrderMark::Present)
        out += request.encoding.byteOrderMarkBytes();

    return out + *body;
}

} // namespace subedit::core
