#include <subedit/core/wording/analysis.hpp>

#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

constexpr std::int64_t kMillisecondsPerSecond = 1000;

} // namespace

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

std::string statementOf(const Anomaly& anomaly) {
    return "subtitle " + std::to_string(anomaly.index.number()) + " " +
           std::string{nameOf(anomaly.kind)};
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
    case DiagnosticKind::UnknownEventField:
        // The column is the detail, so the sentence does not name it twice.
        return "declares an event column this tool cannot fill";
    case DiagnosticKind::AssumedFrameRate:
        // The rate is the detail. What the sentence has to carry is that the
        // file did not say it — every position on screen rests on the answer.
        return "counts in frames and states no rate; it was read at";
    case DiagnosticKind::DeducedEnds:
        // No detail: what the sentence has to carry is that not one end in the
        // table came from the file, and naming a line would say the opposite.
        return "carries no end times; each one was taken from the next start";
    case DiagnosticKind::MixedNewlines:
        return "more than one kind of line ending";
    case DiagnosticKind::GuessedEncoding:
        return "an encoding nothing declared";
    case DiagnosticKind::MarkOverridesEncoding:
        return "a byte order mark that contradicts the encoding asked for";
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
    case CommandKind::SetDuration:
        return "editing a duration";
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
    case CommandKind::Italicise:
        return "putting in italics";
    case CommandKind::Unitalicise:
        return "taking italics out";
    case CommandKind::ChangeCase:
        return "changing the case";
    case CommandKind::AddDialogueDashes:
        return "adding dialogue dashes";
    case CommandKind::RemoveDialogueDashes:
        return "removing dialogue dashes";
    case CommandKind::Merge:
        return "merging";
    case CommandKind::Split:
        return "splitting";
    case CommandKind::Cut:
        return "cutting texts";
    case CommandKind::Paste:
        return "pasting texts";
    case CommandKind::AttachTranslation:
        return "opening a translation";
    case CommandKind::AdjustDurations:
        return "adjusting durations";
    case CommandKind::Replace:
        return "replacing";
    case CommandKind::ReplaceAll:
        return "replacing all";
    case CommandKind::Append:
        return "appending a file";
    case CommandKind::SplitProject:
        return "splitting the project";
    case CommandKind::CorrectTexts:
        return "correcting texts";
    }

    // Every one of `CommandKind` is handled and the compiler checks it. A
    // `default` here would take an enumerator added without a name in
    // silence, and the action would announce it as an empty string.
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

std::string gridStatusOf(GridVerdict verdict, std::optional<FrameRate> rate) {
    if (!rate.has_value())
        return "No grid";

    std::string text = "Grid: " + nameOf(*rate) + " fps";
    if (verdict == GridVerdict::Partial)
        text += " (" + std::string{nameOf(verdict)} + ")";
    return text;
}

std::string framesStatusOf(FrameRate rate) {
    return "Frames: " + nameOf(rate) + " fps";
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

} // namespace subedit::core
