#include <subedit/core/edit/translation.hpp>
#include <subedit/core/edit/translation_drift.hpp>
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
        return "translation: the file holds no line";

    // Each clause only when it is not zero, in the order a reader meets the
    // things in: what worked, then what did not.
    std::vector<std::string> clauses;
    clauses.reserve(4);
    if (outcome.attached > 0)
        clauses.push_back(countOf(outcome.attached, "line") + " attached");
    if (outcome.born > 0)
        clauses.push_back(countOf(outcome.born, "subtitle") + " born of a line");
    if (outcome.untranslated > 0)
        clauses.push_back(countOf(outcome.untranslated, "subtitle") +
                          " left without a translation");
    if (outcome.outOfOrder > 0)
        clauses.push_back(countOf(outcome.outOfOrder, "line") + " out of order");

    std::string notice = "translation: ";
    for (std::size_t rank = 0; rank < clauses.size(); ++rank)
        notice += (rank == 0 ? "" : "; ") + clauses[rank];
    return notice;
}

std::string shiftNoticeOf(const ConstantShift& shift) {
    const bool late = shift.lateBy.milliseconds() > 0;
    const Duration length =
        late ? shift.lateBy : Duration::fromMilliseconds(-shift.lateBy.milliseconds());
    return "the lines sit " + secondsOf(length) + (late ? " later" : " earlier") +
           " than the subtitles; moved " + (late ? "back" : "forward") + ", " +
           countOf(shift.ifShifted.attached, "line") + " would attach instead of " +
           std::to_string(shift.asOpened.attached);
}

std::string_view reasonOf(const TranslationError& error) {
    return std::visit(
        [](const auto& one) -> std::string_view {
            if constexpr (std::is_same_v<std::decay_t<decltype(one)>, SameFileAsMain>)
                return "the file is already open as the main document";
            else
                return reasonOf(one.kind);
        },
        error);
}

} // namespace subedit::core
