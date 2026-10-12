#include <subedit/core/edit/translation.hpp>
#include <subedit/core/edit/translation_drift.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/core/wording/translation.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace subedit::core {

std::string noticeOf(const TranslationOutcome& outcome) {
    // Not « 176 subtitles left without a translation »: that would blame the
    // subtitles for a file that holds nothing.
    if (outcome.attached + outcome.born == 0)
        return std::string{translate("translation: the file holds no line")};

    // Each clause only when it is not zero, in the order a reader meets the
    // things in: what worked, then what did not.
    std::vector<std::string> clauses;
    clauses.reserve(4);
    if (outcome.attached > 0)
        clauses.push_back(
            substitute(translatePlural("%1 line attached", "%1 lines attached", outcome.attached),
                       {std::to_string(outcome.attached)}));
    if (outcome.born > 0)
        clauses.push_back(substitute(translatePlural("%1 subtitle born of a line",
                                                     "%1 subtitles born of a line",
                                                     outcome.born),
                                     {std::to_string(outcome.born)}));
    if (outcome.untranslated > 0)
        clauses.push_back(substitute(translatePlural("%1 subtitle left without a translation",
                                                     "%1 subtitles left without a translation",
                                                     outcome.untranslated),
                                     {std::to_string(outcome.untranslated)}));
    if (outcome.outOfOrder > 0)
        clauses.push_back(substitute(
            translatePlural("%1 line out of order", "%1 lines out of order", outcome.outOfOrder),
            {std::to_string(outcome.outOfOrder)}));

    std::string said;
    for (std::size_t rank = 0; rank < clauses.size(); ++rank)
        said += (rank == 0 ? "" : "; ") + clauses[rank];
    // TRANSLATORS: %1 is what became of the lines of a translation, such as
    // « 4 lines attached; 1 line out of order ».
    return substitute(translate("translation: %1"), {said});
}

std::string shiftNoticeOf(const ConstantShift& shift) {
    const bool late = shift.lateBy.milliseconds() > 0;
    const Duration length =
        late ? shift.lateBy : Duration::fromMilliseconds(-shift.lateBy.milliseconds());
    const std::size_t attached = shift.ifShifted.attached;
    const std::string_view sentence =
        // TRANSLATORS: %1 is a length in seconds, %2 and %3 counts of lines.
        late ? translatePlural("the lines sit %1 later than the subtitles; moved back, %2 line "
                               "would attach instead of %3",
                               "the lines sit %1 later than the subtitles; moved back, %2 lines "
                               "would attach instead of %3",
                               attached)
             : translatePlural("the lines sit %1 earlier than the subtitles; moved forward, %2 "
                               "line would attach instead of %3",
                               "the lines sit %1 earlier than the subtitles; moved forward, %2 "
                               "lines would attach instead of %3",
                               attached);
    return substitute(
        sentence,
        {secondsOf(length), std::to_string(attached), std::to_string(shift.asOpened.attached)});
}

std::string_view reasonOf(const TranslationError& error) {
    return std::visit(
        [](const auto& one) -> std::string_view {
            if constexpr (std::is_same_v<std::decay_t<decltype(one)>, SameFileAsMain>)
                return translate("the file is already open as the main document");
            else
                return reasonOf(one.kind);
        },
        error);
}

} // namespace subedit::core
