#include <subedit/core/wording.hpp>

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

namespace subedit::core {

namespace {

constexpr std::int64_t kMillisecondsPerSecond = 1000;

} // namespace

std::string_view nameOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return "SubRip";
    case SubtitleFormat::WebVtt:
        return "WebVTT";
    }
    std::unreachable();
}

std::string_view extensionOf(SubtitleFormat format) {
    switch (format) {
    case SubtitleFormat::SubRip:
        return ".srt";
    case SubtitleFormat::WebVtt:
        return ".vtt";
    }
    std::unreachable();
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

std::string_view reasonOf(ReadErrorKind kind) {
    switch (kind) {
    case ReadErrorKind::InvalidUtf8:
        return "is not valid UTF-8";
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

std::string_view nameOf(AnomalyKind kind) {
    switch (kind) {
    case AnomalyKind::EndBeforeStart:
        return "ends before it starts";
    case AnomalyKind::OverlappingSubtitles:
        return "starts before the previous one ends";
    case AnomalyKind::OutOfOrder:
        return "starts before the previous one starts";
    }
    std::unreachable();
}

std::string_view nameOf(DiagnosticKind kind) {
    switch (kind) {
    case DiagnosticKind::IgnoredLine:
        return "a line that fits nowhere";
    case DiagnosticKind::MalformedTimestamp:
        return "a timing line that could not be read";
    case DiagnosticKind::MissingNumbering:
        return "a SubRip block without its number";
    case DiagnosticKind::InconsistentNumbering:
        return "SubRip numbers that do not follow";
    case DiagnosticKind::TextBeforeAnyTimestamp:
        return "text before the first timing line";
    case DiagnosticKind::UnknownBlock:
        return "a WebVTT block of an unknown kind";
    case DiagnosticKind::MixedNewlines:
        return "more than one kind of line ending";
    }
    std::unreachable();
}

std::string_view nameOf(core::Severity severity) {
    switch (severity) {
    case Severity::Warning:
        return "left as it stands";
    case Severity::Recovered:
        return "settled by the reader";
    }
    std::unreachable();
}

std::string_view nameOf(CommandKind kind) {
    switch (kind) {
    case CommandKind::SetText:
        return "editing a text";
    case CommandKind::SetStart:
        return "editing a start";
    case CommandKind::SetEnd:
        return "editing an end";
    case CommandKind::Insert:
        return "inserting";
    case CommandKind::Remove:
        return "removing";
    case CommandKind::Shift:
        return "shifting";
    case CommandKind::Transform:
        return "transforming";
    case CommandKind::ConvertFrameRate:
        return "converting the frame rate";
    case CommandKind::Snap:
        return "aligning on the frame rate";
    case CommandKind::Sort:
        return "sorting";
    case CommandKind::RemoveHearingImpaired:
        return "removing hearing-impaired mentions";
    }

    // The eleven are handled and the compiler checks it. A `default` here would
    // take an enumerator added without a name in silence, and the action would
    // announce it as an empty string.
    std::unreachable();
}

std::string nameOf(core::FrameRate rate) {
    constexpr std::int64_t kThousand = 1000;
    if (kThousand % rate.denominator() != 0) {
        return std::to_string(rate.numerator()) + "/" + std::to_string(rate.denominator());
    }

    // Both terms are bounded by a billion, so a thousandth of the rate holds in
    // an int64 with three orders of magnitude to spare.
    const std::int64_t thousandths = rate.numerator() * (kThousand / rate.denominator());
    std::string whole = std::to_string(thousandths / kThousand);
    const std::int64_t rest = thousandths % kThousand;
    if (rest == 0) {
        return whole;
    }

    std::string decimals = std::to_string(rest);
    decimals.insert(0, 3 - decimals.size(), '0');
    while (decimals.back() == '0') {
        decimals.pop_back();
    }
    return whole + "." + decimals;
}

std::string_view nameOf(GridVerdict verdict) {
    switch (verdict) {
    case GridVerdict::Clean:
        return "clean";
    case GridVerdict::Partial:
        return "partial";
    case GridVerdict::Silent:
        return "none";
    }

    // The three are handled and the compiler checks it.
    std::unreachable();
}

std::string percentOf(double value) {
    constexpr std::int64_t kTenths = 10;
    const std::int64_t tenths = std::llround(value * static_cast<double>(kTenths));
    return std::to_string(tenths / kTenths) + "." + std::to_string(tenths % kTenths) + "%";
}

std::string videoStatusOf(const std::optional<std::filesystem::path>& video) {
    if (!video.has_value())
        return "No video";
    return "Video: " + video->filename().string();
}

std::string secondsOf(Duration length) {
    const std::int64_t total = length.milliseconds();
    const std::int64_t whole = total / kMillisecondsPerSecond;
    std::int64_t rest = total % kMillisecondsPerSecond;
    if (rest < 0)
        rest = -rest;

    std::string text = std::to_string(whole);
    // A length between −1 s and 0 has a whole part of zero, which carries no
    // sign of its own: "-0.500 s" would otherwise be written "0.500 s".
    if (total < 0 && whole == 0)
        text = "-" + text;

    std::string decimals = std::to_string(rest);
    decimals.insert(0, 3 - decimals.size(), '0');
    return text + "." + decimals + " s";
}

std::string noticeOf(CommandKind kind, BeyondEnd beyond) {
    // « by ... at most » rather than « the furthest by ... », which reads
    // wrong when there is exactly one of them — and one is the common case.
    return std::string{nameOf(kind)} + " leaves " + countOf(beyond.count, "subtitle") +
           " past the end of the video, by " + secondsOf(beyond.overshoot) + " at most";
}

std::string countOf(std::size_t count, std::string_view noun) {
    std::string text = std::to_string(count) + " " + std::string{noun};
    if (count != 1) {
        text += "s";
    }
    return text;
}

} // namespace subedit::core
