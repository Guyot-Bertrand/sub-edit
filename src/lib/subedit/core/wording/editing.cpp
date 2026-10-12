#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/editing.hpp>

#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

std::string noticeOf(CommandKind kind, BeyondEnd beyond) {
    // « by ... at most » rather than « the furthest by ... », which reads
    // wrong when there is exactly one of them — and one is the common case.
    const std::string_view sentence =
        // TRANSLATORS: %1 is an operation, such as « shifting », %2 a count, %3
        // a length in seconds.
        translatePlural("%1 leaves %2 subtitle past the end of the video, by %3 at most",
                        "%1 leaves %2 subtitles past the end of the video, by %3 at most",
                        beyond.count);
    return substitute(sentence,
                      {nameOf(kind), std::to_string(beyond.count), secondsOf(beyond.overshoot)});
}

std::string noticeOf(PartialAlignment partial) {
    // **« of 176 » and not « out of 176 »**, because the number that matters is
    // the second one: a reader who sees five and a hundred and seventy-six side
    // by side has the whole of it, and the shorter form puts them side by side.
    // TRANSLATORS: %1 of %2 subtitles were aligned; %3 and %4 are frame rates.
    return substitute(translatePlural("%1 of %2 subtitle aligned to %3 fps; the document is "
                                      "still read on a %4 fps grid",
                                      "%1 of %2 subtitles aligned to %3 fps; the document is "
                                      "still read on a %4 fps grid",
                                      partial.total),
                      {std::to_string(partial.aligned),
                       std::to_string(partial.total),
                       nameOf(partial.onto),
                       nameOf(partial.retained)});
}

std::string reasonOf(const PatternError& error) {
    switch (error.kind) {
    case PatternError::Kind::Empty:
        return std::string{translate("nothing to look for")};
    case PatternError::Kind::InvalidExpression:
        // TRANSLATORS: %1 is the regular expression engine's own reason, in English.
        return substitute(translate("not a regular expression (%1)"), {error.reason});
    }
    std::unreachable();
}

std::string noticeOfAdjustment(std::size_t adjusted, const SacrificedConstraints& sacrificed) {
    std::string notice = adjusted == 0
                             ? std::string{translate("no duration to adjust")}
                             : substitute(translatePlural("adjusted the durations of %1 subtitle",
                                                          "adjusted the durations of %1 subtitles",
                                                          adjusted),
                                          {std::to_string(adjusted)});

    // In the order the constraints are applied, which is the order a reader of
    // the dialog meets them in.
    std::vector<std::string> posts;
    if (sacrificed.speed > 0)
        posts.push_back(substitute(translatePlural("the reading speed in %1 subtitle",
                                                   "the reading speed in %1 subtitles",
                                                   sacrificed.speed),
                                   {std::to_string(sacrificed.speed)}));
    if (sacrificed.minimum > 0)
        posts.push_back(substitute(translatePlural("the minimum duration in %1 subtitle",
                                                   "the minimum duration in %1 subtitles",
                                                   sacrificed.minimum),
                                   {std::to_string(sacrificed.minimum)}));
    if (sacrificed.gap > 0)
        posts.push_back(substitute(
            translatePlural("the gap in %1 subtitle", "the gap in %1 subtitles", sacrificed.gap),
            {std::to_string(sacrificed.gap)}));
    if (posts.empty())
        return notice;

    std::string unsatisfied;
    for (std::size_t rank = 0; rank < posts.size(); ++rank)
        unsatisfied += (rank == 0 ? "" : ", ") + posts[rank];
    // TRANSLATORS: %1 is what the adjustment did, %2 the constraints it could
    // not satisfy, such as « the gap in 2 subtitles ».
    return substitute(translate("%1; could not satisfy %2"), {notice, unsatisfied});
}

std::string noticeOfDisjoint(const DisjointReport& report) {
    if (report.groups == 0 && report.endingBeforeStart == 0)
        return std::string{translate("already disjoint")};

    std::string notice = substitute(translatePlural("%1 subtitle split, %2 merged",
                                                    "%1 subtitles split, %2 merged",
                                                    report.split),
                                    {std::to_string(report.split), std::to_string(report.merged)});

    // What was not done in full, after what was. English says the same words
    // for one and for several where no noun follows the count; a language
    // that agrees the participle with it has both forms to fill.
    std::vector<std::string> clauses;
    if (report.windowed > 0)
        clauses.push_back(
            substitute(translatePlural("%1 shown in windows for holding too many lines",
                                       "%1 shown in windows for holding too many lines",
                                       report.windowed),
                       {std::to_string(report.windowed)}));
    if (report.absorbed > 0)
        clauses.push_back(substitute(translatePlural("%1 very short taken over by a neighbour",
                                                     "%1 very short taken over by a neighbour",
                                                     report.absorbed),
                                     {std::to_string(report.absorbed)}));
    if (report.tooShort > 0)
        clauses.push_back(
            substitute(translatePlural("%1 subtitle shorter than the limit left whole",
                                       "%1 subtitles shorter than the limit left whole",
                                       report.tooShort),
                       {std::to_string(report.tooShort)}));
    if (report.extrasDropped > 0)
        clauses.push_back(substitute(
            translatePlural("%1 merged with styles or positions that differ, the first kept",
                            "%1 merged with styles or positions that differ, the first kept",
                            report.extrasDropped),
            {std::to_string(report.extrasDropped)}));
    if (report.endingBeforeStart > 0)
        clauses.push_back(
            substitute(translatePlural("%1 subtitle ending before the start left alone",
                                       "%1 subtitles ending before the start left alone",
                                       report.endingBeforeStart),
                       {std::to_string(report.endingBeforeStart)}));

    for (std::size_t rank = 0; rank < clauses.size(); ++rank)
        notice += (rank == 0 ? "; " : ", ") + clauses[rank];
    return notice;
}

std::string noticeOfJoin(const JoinReport& report) {
    if (report.joined == 0)
        return std::string{translate("no repeated subtitles")};
    return substitute(translatePlural("%1 repeated subtitle joined",
                                      "%1 repeated subtitles joined",
                                      report.joined),
                      {std::to_string(report.joined)});
}

std::string refusalOfDisjoint(DisjointRefusal refusal) {
    switch (refusal) {
    case DisjointRefusal::NotInOrder:
        return std::string{
            translate("The subtitles are not in the order of their start: sort them first.")};
    }
    std::unreachable();
}

} // namespace subedit::core
