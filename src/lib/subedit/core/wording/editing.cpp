#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/search.hpp>
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
    return std::string{nameOf(kind)} + " leaves " + countOf(beyond.count, "subtitle") +
           " past the end of the video, by " + secondsOf(beyond.overshoot) + " at most";
}

std::string noticeOf(PartialAlignment partial) {
    // **« of 176 » and not « out of 176 »**, because the number that matters is
    // the second one: a reader who sees five and a hundred and seventy-six side
    // by side has the whole of it, and the shorter form puts them side by side.
    return std::to_string(partial.aligned) + " of " + countOf(partial.total, "subtitle") +
           " aligned to " + nameOf(partial.onto) + " fps; the document is still read on a " +
           nameOf(partial.retained) + " fps grid";
}

std::string reasonOf(const PatternError& error) {
    switch (error.kind) {
    case PatternError::Kind::Empty:
        return "nothing to look for";
    case PatternError::Kind::InvalidExpression:
        return "not a regular expression (" + error.reason + ")";
    }
    std::unreachable();
}

std::string noticeOfAdjustment(std::size_t adjusted, const SacrificedConstraints& sacrificed) {
    std::string notice = adjusted == 0
                             ? std::string{"no duration to adjust"}
                             : "adjusted the durations of " + countOf(adjusted, "subtitle");

    // In the order the constraints are applied, which is the order a reader of
    // the dialog meets them in.
    std::vector<std::string> posts;
    if (sacrificed.speed > 0)
        posts.emplace_back("the reading speed in " + countOf(sacrificed.speed, "subtitle"));
    if (sacrificed.minimum > 0)
        posts.emplace_back("the minimum duration in " + countOf(sacrificed.minimum, "subtitle"));
    if (sacrificed.gap > 0)
        posts.emplace_back("the gap in " + countOf(sacrificed.gap, "subtitle"));

    for (std::size_t rank = 0; rank < posts.size(); ++rank)
        notice += (rank == 0 ? "; could not satisfy " : ", ") + posts[rank];
    return notice;
}

} // namespace subedit::core
