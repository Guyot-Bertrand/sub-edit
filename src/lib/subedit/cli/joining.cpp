#include <subedit/cli/disjoining.hpp>
#include <subedit/cli/joining.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/sorting.hpp>
#include <subedit/core/edit/join_repeats.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/editing.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace subedit::cli {

namespace {

/// Joins the repeats of the project of `session`, through the history.
[[nodiscard]] core::JoinReport joinedIn(core::Session& session) {
    core::JoinedRepeats joined = core::joinRepeats(session.project());
    if (joined.command != nullptr) {
        (void)session.apply(std::move(joined.command));
    }
    return joined.report;
}

[[nodiscard]] std::int64_t count(std::size_t number) {
    return static_cast<std::int64_t>(number);
}

} // namespace

ExitCode joinAll(core::FileSystem& files,
                 const std::vector<std::string>& paths,
                 const std::optional<core::Encoding>& reading,
                 const Destination& destination,
                 const Reporter& reporter) {
    const Operation join = [](core::Session& session) -> OperationOutcome {
        const core::JoinReport report = joinedIn(session);
        return OperationResult{.sentence = core::noticeOfJoin(report),
                               .counts = {{"subtitles", count(session.project().count())},
                                          {"joined", count(report.joined)},
                                          {"chains", count(report.chains)}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "joined", join);
}

ExitCode repairAll(core::FileSystem& files,
                   const std::vector<std::string>& paths,
                   const std::optional<core::Encoding>& reading,
                   const core::DisjointSettings& settings,
                   const Destination& destination,
                   const Reporter& reporter) {
    const Operation repair = [settings](core::Session& session) -> OperationOutcome {
        const core::JoinReport joined = joinedIn(session);
        const std::size_t moved = sortedIn(session);

        // In order now, so the cut cannot be refused.
        core::DisjointRanges made = core::makeDisjoint(session.project(), settings).value();
        const core::DisjointReport cut = made.report;
        if (made.command != nullptr) {
            (void)session.apply(std::move(made.command));
        }

        std::vector<Count> counts{{"subtitles", count(session.project().count())},
                                  {"joined", count(joined.joined)},
                                  {"moved", count(moved)}};
        for (Count& each : fullCountsOfDisjoint(cut)) {
            counts.push_back(std::move(each));
        }
        return OperationResult{.sentence = core::noticeOfJoin(joined) + "; " +
                                           core::noticeOfSort(moved) + "; " +
                                           core::noticeOfDisjoint(cut),
                               .counts = std::move(counts)};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "repaired", repair);
}

} // namespace subedit::cli
