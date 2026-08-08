#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>

#include <vector>

namespace subedit::core {

class Project;

/// One reversible operation on a project.
///
/// A command carries what it needs to undo itself, **captured at construction
/// and limited to the strict minimum** — removing subtitles remembers « insert
/// these subtitles at these positions », not a copy of the whole project. That
/// is what keeps the history from duplicating a file of several thousand
/// subtitles at every operation.
///
/// Every operation of the core is written as a command, from phase 2 onwards.
/// A function that modifies a project without going through one is a defect.
class Command {

public:
    virtual ~Command() = default;

    /// Carries the operation out.
    virtual void apply(Project& project) = 0;

    /// Puts back exactly the state that preceded `apply`.
    ///
    /// Exactly: an operation that cannot undo itself faithfully has to empty
    /// the stack rather than lie about being reversible.
    virtual void revert(Project& project) = 0;

    /// Returns what kind of operation this is.
    ///
    /// What an interface names when it offers to undo it, and what an order
    /// policy reads to know whether the operation could have broken the order.
    /// Neither has to know the concrete class.
    [[nodiscard]] virtual CommandKind kind() const = 0;

    /// Reports what the command touched, and where.
    ///
    /// The core knows no signal mechanism: it hands the information back and
    /// the caller decides what to do with it. The interface uses it to refresh
    /// the rows that moved instead of rebuilding the whole table; the history
    /// uses it to know which documents differ from the file on disk.
    [[nodiscard]] virtual std::vector<Change> describe() const = 0;

protected:
    Command() = default;
    Command(const Command&) = default;
    Command(Command&&) = default;
    Command& operator=(const Command&) = default;
    Command& operator=(Command&&) = default;
};

} // namespace subedit::core
