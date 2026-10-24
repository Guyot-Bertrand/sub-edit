#include <subedit/core/i18n/messages.hpp>
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
        return translate("ends before it starts");
    case AnomalyKind::OverlappingSubtitles:
        return translate("starts before the previous one ends");
    case AnomalyKind::OutOfOrder:
        return translate("starts before the previous one starts");
    }
    std::unreachable();
}

std::string statementOf(const Anomaly& anomaly) {
    // TRANSLATORS: %1 is a subtitle number, %2 what is wrong with it, such as
    // « starts before the previous one ends ».
    return substitute(translate("subtitle %1 %2"),
                      {std::to_string(anomaly.index.number()), nameOf(anomaly.kind)});
}

std::string summaryOf(AnomalyKind kind, std::size_t count) {
    // TRANSLATORS: %1 is what is wrong with a subtitle, such as « starts before
    // the previous one ends », and %2 how many subtitles it is wrong with.
    return substitute(translate("subtitle %1: %2"), {nameOf(kind), std::to_string(count)});
}

std::string_view nameOf(DiagnosticKind kind) {
    switch (kind) {
    case DiagnosticKind::IgnoredLine:
        return translate("a line that fits nowhere");
    case DiagnosticKind::MalformedTimestamp:
        return translate("a timing line that could not be read");
    case DiagnosticKind::MissingNumbering:
        return translate("a SubRip block without its number");
    case DiagnosticKind::InconsistentNumbering:
        return translate("SubRip numbers that do not follow");
    case DiagnosticKind::TextBeforeAnyTimestamp:
        return translate("text before the first timing line");
    case DiagnosticKind::UnknownBlock:
        return translate("a WebVTT block of an unknown kind");
    case DiagnosticKind::UnknownEventField:
        // The column is the detail, so the sentence does not name it twice.
        return translate("declares an event column this tool cannot fill");
    case DiagnosticKind::AssumedFrameRate:
        // The rate is the detail. What the sentence has to carry is that the
        // file did not say it — every position on screen rests on the answer.
        return translate("counts in frames and states no rate; it was read at");
    case DiagnosticKind::DeducedEnds:
        // No detail: what the sentence has to carry is that not one end in the
        // table came from the file, and naming a line would say the opposite.
        return translate("carries no end times; each one was taken from the next start");
    case DiagnosticKind::MixedNewlines:
        return translate("more than one kind of line ending");
    case DiagnosticKind::GuessedEncoding:
        return translate("an encoding nothing declared");
    case DiagnosticKind::MarkOverridesEncoding:
        return translate("a byte order mark that contradicts the encoding asked for");
    }
    std::unreachable();
}

std::string_view nameOf(core::Severity severity) {
    switch (severity) {
    case Severity::Warning:
        return translate("left as it stands");
    case Severity::Recovered:
        return translate("settled by the reader");
    }
    std::unreachable();
}

std::string_view nameOf(CommandKind kind) {
    switch (kind) {
    case CommandKind::SetText:
        return translate("editing a text");
    case CommandKind::SetStart:
        return translate("editing a start");
    case CommandKind::SetEnd:
        return translate("editing an end");
    case CommandKind::SetDuration:
        return translate("editing a duration");
    case CommandKind::Insert:
        return translate("inserting");
    case CommandKind::Remove:
        return translate("removing");
    case CommandKind::Shift:
        return translate("shifting");
    case CommandKind::Transform:
        return translate("transforming");
    case CommandKind::ConvertFrameRate:
        return translate("converting the frame rate");
    case CommandKind::Snap:
        return translate("aligning on the frame rate");
    case CommandKind::Sort:
        return translate("sorting");
    case CommandKind::RemoveHearingImpaired:
        return translate("removing hearing-impaired mentions");
    case CommandKind::Italicise:
        return translate("putting in italics");
    case CommandKind::Unitalicise:
        return translate("taking italics out");
    case CommandKind::ChangeCase:
        return translate("changing the case");
    case CommandKind::AddDialogueDashes:
        return translate("adding dialogue dashes");
    case CommandKind::RemoveDialogueDashes:
        return translate("removing dialogue dashes");
    case CommandKind::Merge:
        return translate("merging");
    case CommandKind::Split:
        return translate("splitting");
    case CommandKind::Cut:
        return translate("cutting texts");
    case CommandKind::Paste:
        return translate("pasting texts");
    case CommandKind::AttachTranslation:
        return translate("opening a translation");
    case CommandKind::AdjustDurations:
        return translate("adjusting durations");
    case CommandKind::Replace:
        return translate("replacing");
    case CommandKind::ReplaceAll:
        return translate("replacing all");
    case CommandKind::Append:
        return translate("appending a file");
    case CommandKind::SplitProject:
        return translate("splitting the project");
    case CommandKind::CorrectTexts:
        return translate("correcting texts");
    case CommandKind::MakeDisjoint:
        return translate("making the ranges disjoint");
    case CommandKind::JoinRepeats:
        return translate("joining repeated subtitles");
    case CommandKind::Transcribe:
        return translate("transcribing the audio");
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
        // TRANSLATORS: how well the positions fall on a frame-rate grid: clean,
        // partial, or none at all.
        return translate("clean");
    case GridVerdict::Partial:
        return translate("partial");
    case GridVerdict::Silent:
        return translate("none");
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
        return std::string{translate("No grid")};

    if (verdict == GridVerdict::Partial)
        // TRANSLATORS: %1 is a frame rate, %2 the word for a partial grid.
        return substitute(translate("Grid: %1 fps (%2)"), {nameOf(*rate), nameOf(verdict)});
    return substitute(translate("Grid: %1 fps"), {nameOf(*rate)});
}

std::string framesStatusOf(FrameRate rate) {
    return substitute(translate("Frames: %1 fps"), {nameOf(rate)});
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
