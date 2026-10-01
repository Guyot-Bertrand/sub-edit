#include <subedit/core/wording/formats.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace subedit::core {

std::string_view nameOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return "SubRip";
    case SubtitleFormat::WebVtt:
        return "WebVTT";
    case SubtitleFormat::SubViewer2:
        return "SubViewer 2";
    case SubtitleFormat::SubStationAlpha:
        return "Sub Station Alpha";
    case SubtitleFormat::AdvancedSubStationAlpha:
        return "Advanced SSA";
    case SubtitleFormat::MicroDvd:
        return "MicroDVD";
    case SubtitleFormat::Mpl2:
        return "MPL2";
    case SubtitleFormat::TMPlayer:
        return "TMPlayer";
    case SubtitleFormat::Lrc:
        return "LRC";
    }
    std::unreachable();
}

std::string_view optionNameOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return "srt";
    case SubtitleFormat::WebVtt:
        return "vtt";
    case SubtitleFormat::SubViewer2:
        return "subviewer2";
    case SubtitleFormat::SubStationAlpha:
        return "ssa";
    case SubtitleFormat::AdvancedSubStationAlpha:
        return "ass";
    case SubtitleFormat::MicroDvd:
        return "microdvd";
    case SubtitleFormat::Mpl2:
        return "mpl2";
    case SubtitleFormat::TMPlayer:
        return "tmplayer";
    case SubtitleFormat::Lrc:
        return "lrc";
    }
    std::unreachable();
}

std::optional<SubtitleFormat> formatNamed(std::string_view option) {
    // Walked rather than tabulated a second time: `optionNameOf` is the one
    // place the names are written, and a table facing it would be a place to
    // forget one.
    for (const SubtitleFormat format : kSubtitleFormats) {
        if (optionNameOf(format) == option)
            return format;
    }
    return std::nullopt;
}

std::string_view extensionOf(SubtitleFormat format) {
    // **Two extensions name two formats each, and this function does not mind.**
    // `.sub` is MicroDVD and SubViewer 2, `.txt` is MPL2 and TMPlayer. Going
    // this way there is one answer per format — the one to suggest when saving.
    // The other way round is not a function at all, which is why nothing here
    // ever maps an extension back to a format: only the content decides.
    switch (format) {
    case SubtitleFormat::SubRip:
        return ".srt";
    case SubtitleFormat::WebVtt:
        return ".vtt";
    // The two shared extensions are grouped rather than listed twice, which is
    // also what clang-tidy asks for: two consecutive branches returning the
    // same thing are one branch. It leaves the enumeration order, and says the
    // sharing where a reader meets it.
    case SubtitleFormat::SubViewer2:
    case SubtitleFormat::MicroDvd:
        return ".sub";
    case SubtitleFormat::SubStationAlpha:
        return ".ssa";
    case SubtitleFormat::AdvancedSubStationAlpha:
        return ".ass";
    case SubtitleFormat::Mpl2:
    case SubtitleFormat::TMPlayer:
        return ".txt";
    case SubtitleFormat::Lrc:
        return ".lrc";
    }
    std::unreachable();
}

DecimalMark decimalMarkOf(SubtitleFormat format) {
    return format == SubtitleFormat::SubRip ? DecimalMark::Comma : DecimalMark::Period;
}

std::string_view nameOf(Newline newline) {
    switch (newline) {
    case Newline::Lf:
        return "LF";
    case Newline::CrLf:
        return "CRLF";
    case Newline::Cr:
        return "CR";
    }
    std::unreachable();
}

std::string nameOf(const Encoding& encoding) {
    return std::string{encoding.charset()} +
           (encoding.byteOrderMark() == ByteOrderMark::Present ? ", BOM" : ", no BOM");
}

std::string encodingStatusOf(const Encoding& encoding) {
    return "Encoding: " + nameOf(encoding);
}

std::string refusalOf(EncodingRefusal refusal, std::string_view name) {
    switch (refusal) {
    case EncodingRefusal::Unknown:
        return "no encoding is named \"" + std::string{name} + "\"";
    case EncodingRefusal::WritesItsOwnMark:
        return "\"" + std::string{name} +
               "\" writes a byte order mark of its own; name the byte order, "
               "as UTF-16LE and UTF-16BE do";
    }
    std::unreachable();
}

std::string_view reasonOf(ReadErrorKind kind) {
    switch (kind) {
    case ReadErrorKind::Undecodable:
        return "cannot be decoded in the chosen encoding";
    case ReadErrorKind::NoSubtitleFound:
        return "holds nothing recognisable as a subtitle";
    case ReadErrorKind::UnknownFormat:
        return "is in no format this tool knows";
    }
    std::unreachable();
}

std::string_view reasonOf(FileErrorKind kind) {
    switch (kind) {
    case FileErrorKind::NotFound:
        return "does not exist";
    case FileErrorKind::PermissionDenied:
        return "cannot be opened: permission denied";
    case FileErrorKind::Io:
        return "cannot be read";
    }
    std::unreachable();
}

std::string_view reasonOfWriting(FileErrorKind kind) {
    switch (kind) {
    case FileErrorKind::PermissionDenied:
        return "cannot be written: permission denied";
    case FileErrorKind::NotFound:
    case FileErrorKind::Io:
        return "cannot be written";
    }
    std::unreachable();
}

std::string_view reasonOfCreating(FileErrorKind kind) {
    switch (kind) {
    case FileErrorKind::PermissionDenied:
        return "cannot be created: permission denied";
    case FileErrorKind::NotFound:
    case FileErrorKind::Io:
        return "cannot be created";
    }
    std::unreachable();
}

std::string_view reasonOf(WriteErrorKind kind) {
    switch (kind) {
    case WriteErrorKind::Unencodable:
        return "holds a character the chosen encoding cannot write";
    }
    std::unreachable();
}

std::string_view reasonOf(const OpenError& error) {
    return std::visit([](const auto& one) { return reasonOf(one.kind); }, error);
}

std::string_view reasonOf(const SaveError& error) {
    // The second step of a save is a write: its refusal is worded as one.
    if (const auto* refused = std::get_if<FileError>(&error))
        return reasonOfWriting(refused->kind);
    return reasonOf(std::get<WriteError>(error).kind);
}

} // namespace subedit::core
