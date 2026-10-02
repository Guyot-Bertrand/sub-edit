#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording/counts.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

/// What the removal does to the texts, subtitle by subtitle.
///
/// Read from the command rather than found by comparing: `describe()` names the
/// subtitles rewritten and the ones taken away, and the texts before were set
/// aside by the caller. **The indices are those of the project before the
/// command ran** — the rewrites move none, and the removal names the ones it
/// takes — so a text that stays is read back after as many places up as
/// subtitles went before it.
std::vector<TextChange> changesOfRemoval(const core::Project& after,
                                         const std::vector<std::string>& before,
                                         const std::vector<core::Change>& described) {
    std::set<std::size_t> removed;
    std::set<std::size_t> rewritten;
    for (const core::Change& change : described) {
        std::set<std::size_t>& into =
            change.kind == core::ChangeKind::Removal ? removed : rewritten;
        for (const core::SubtitleIndex index : change.subtitles.indices())
            into.insert(index.value());
    }

    std::vector<TextChange> changes;
    changes.reserve(removed.size() + rewritten.size());
    // Ascending, whichever of the two a subtitle is: it is the order of the file.
    for (std::size_t at = 0; at < before.size(); ++at) {
        const bool taken = removed.contains(at);
        if (!taken && !rewritten.contains(at))
            continue;

        TextChange change{
            .subtitle = at + 1, .document = core::Document::Main, .before = before[at]};
        if (!taken) {
            const std::size_t now =
                at -
                static_cast<std::size_t>(std::distance(removed.begin(), removed.lower_bound(at)));
            change.after =
                after.subtitleAt(core::SubtitleIndex::fromValue(now)).text(core::Document::Main);
        }
        changes.push_back(std::move(change));
    }
    return changes;
}

} // namespace

ExitCode removeHearingImpairedIn(core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<core::Encoding>& reading,
                                 const Destination& destination,
                                 const Reporter& reporter) {
    const ChangingOperation clean = [](core::Session& session, Wants wants) -> OperationOutcome {
        // The texts as they are, set aside only when someone reads the list.
        std::vector<std::string> before;
        if (wants.changes) {
            before.reserve(session.project().count());
            for (const core::Subtitle& subtitle : session.project().subtitles())
                before.push_back(subtitle.text(core::Document::Main));
        }

        // The whole file: the selection reached the core with the window, and
        // a command line has none.
        std::unique_ptr<core::Command> command = core::removeHearingImpaired(
            session.project(), core::Selection::all(session.project()), core::Document::Main);
        if (!command)
            return OperationResult{
                .sentence = core::noMentionToRemove(),
                .counts = {{"cleaned", 0}, {"removed", 0}},
                .changes = wants.changes ? std::optional{std::vector<TextChange>{}} : std::nullopt};

        const core::HearingImpairedTally tally = core::tallyOf(*command);
        const std::vector<core::Change> described = command->describe();
        session.apply(std::move(command));

        return OperationResult{
            .sentence = core::noticeOfMentionsRemoved(tally.cleaned, tally.removed),
            .counts = {{"cleaned", static_cast<std::int64_t>(tally.cleaned)},
                       {"removed", static_cast<std::int64_t>(tally.removed)}},
            .changes = wants.changes
                           ? std::optional{changesOfRemoval(session.project(), before, described)}
                           : std::nullopt};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "cleaned", clean);
}

} // namespace subedit::cli
