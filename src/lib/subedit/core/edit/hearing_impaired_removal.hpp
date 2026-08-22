#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>

#include <cstddef>
#include <memory>

namespace subedit::core {

class Project;
class Selection;

/// Builds the command that removes the hearing-impaired mentions of `document`
/// from the subtitles of `selection`.
///
/// **The selection arrives with phase 5**, which is where phase 4 said it
/// would: the command line passes the whole file, the window passes what is
/// selected, and neither has to know what the other does.
///
/// Returns **nothing when no text bites**. An empty group would apply without
/// doing anything and still push an entry the user would meet in « undo »
/// without understanding it.
[[nodiscard]] std::unique_ptr<Command>
removeHearingImpaired(const Project& project, const Selection& selection, Document document);

/// What a removal did: how many subtitles it rewrote, how many it took away.
struct HearingImpairedTally {
    std::size_t cleaned = 0;
    std::size_t removed = 0;

    friend bool operator==(const HearingImpairedTally&, const HearingImpairedTally&) = default;
};

/// Reads what `command` will do, from the command rather than by counting
/// again.
///
/// `describe()` already says it: one change of main text per subtitle
/// rewritten, one removal naming every subtitle taken away. Counting a second
/// time — by comparing the texts before and after, say — would give two answers
/// to one question, and the one shown would be the one no test compares.
///
/// **Shared rather than repeated.** The command line words it and the window
/// words it; two copies of one count drift, which this project has already
/// paid for once.
[[nodiscard]] HearingImpairedTally tallyOf(const Command& command);

} // namespace subedit::core
