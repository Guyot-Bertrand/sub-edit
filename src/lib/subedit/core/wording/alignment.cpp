#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/wording/alignment.hpp>
#include <subedit/core/wording/analysis.hpp>

#include <cstddef>
#include <string>
#include <utility>

namespace subedit::core {

std::string noSuchAlignmentEngine(std::string_view name) {
    return substitute(translate("there is no alignment engine called %1"), {name});
}

std::string needsOneAudioSource() {
    return std::string{
        translate("sync needs the audio to listen to: give --video or --audio, and not both")};
}

std::string noLineFoundInTheAudio(std::size_t lines) {
    return substitute(
        translatePlural("the %1 subtitle was looked for in the audio and was not found: is it the "
                        "right audio, and the right language?",
                        "none of the %1 subtitles was found in the audio: is it the right audio, "
                        "and the right language?",
                        lines),
        {std::to_string(lines)});
}

std::string noticeOfAlignment(const AlignmentReport& report) {
    const std::size_t total = report.lines.size();
    const std::size_t toCheck = report.partial + report.outOfLine + report.notFound;
    const std::string placed = substitute(
        translatePlural("%1 of %2 subtitle placed on the audio, %3 to check",
                        "%1 of %2 subtitles placed on the audio, %3 to check",
                        total),
        {std::to_string(report.positioned()), std::to_string(total), std::to_string(toCheck)});
    return substitute(
        // TRANSLATORS: %1 is a sentence, such as « 40 of 42 subtitles placed on the audio, 2 to
        // check »; %2 and %3 are lengths in seconds, such as « -2.400 s ».
        translate("%1; median shift %2, drift %3"),
        {placed,
         secondsOf(Duration::fromMilliseconds(report.medianShiftMilliseconds)),
         secondsOf(Duration::fromMilliseconds(report.driftMilliseconds))});
}

std::string doubtText(LineDoubt doubt) {
    switch (doubt) {
    case LineDoubt::None:
        break;
    case LineDoubt::NotFound:
        return std::string{translate("not found in the audio, left as it was")};
    case LineDoubt::Partial:
        return std::string{translate("only partly heard")};
    case LineDoubt::OutOfLine:
        return std::string{translate("moved differently from its neighbours")};
    }
    return {};
}

std::string lineCutBackText() {
    return std::string{translate("cut back to the start of the next subtitle")};
}

std::string lineOverlapsText() {
    return std::string{translate("still overlaps the next subtitle")};
}

} // namespace subedit::core
