#pragma once

namespace subedit::core {

/// Which reading of disorder `Project::outOfOrder()` reports.
///
/// The two always agree on **whether** a project is out of order: a sequence in
/// which each element follows the one before it is increasing, so each element
/// also follows every earlier one. They differ only on **which lines to name**.
///
/// On the starts `0, 4000, 2000, 3000`, `Breaks` names `{2}` and `Late` names
/// `{2, 3}`.
///
/// Which one an interface should show is not settled. The phase-3 inspection
/// offers both under an option, so that the choice is made on real files rather
/// than argued without data; phase 5 inherits it and the option goes. That is
/// why this is a parameter and not a rule frozen into the query — see
/// docs/specs/03-cli.md.
enum class OrderReport {
    /// The lines that start before the one immediately before them.
    ///
    /// Names where the sequence breaks, as close to the break as possible. The
    /// reading phase 2 shipped, and the default: every caller written before
    /// the second one existed still gets it.
    Breaks,

    /// The lines that start before the largest start already seen.
    ///
    /// Names the whole extent of the disorder rather than its breaking points.
    /// A line that follows its predecessor but still lands before something
    /// earlier is late under this reading and breaks nothing under the other.
    Late,
};

} // namespace subedit::core
