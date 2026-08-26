#pragma once

#include <subedit/core/command/command_kind.hpp>

#include <utility>

namespace subedit::core {

/// What a session does about an operation that leaves the subtitles out of
/// order.
enum class OrderPolicy {
    /// Nothing. The file keeps the order it came with, and no edit moves a
    /// line. `scanAnomalies` says where the disorder is — it took the place of
    /// `Project::outOfOrder()` at issue #127 — and the interface shows it: the
    /// table tints the subtitles it names.
    ///
    /// The default, and the reading of ADR 0008 that the core applies
    /// everywhere: report, do not repair.
    Lenient,

    /// Follow the operation with a sort, inside the same history entry.
    ///
    /// A policy of composition, not an invariant of the model — see ADR 0012.
    /// The order is restored by a command that can be undone, rather than by a
    /// rule every operation would have to carry.
    Strict,
};

/// Tells whether an operation of that kind could leave the subtitles out of
/// order.
///
/// Read by the strict policy to know whether a sort has to follow. It answers
/// on the kind and not on the result: a shift that happened to keep the order
/// still gets a sort, which then moves nothing and reports nothing.
///
/// Only what moves a **start** can break the order. Changing a text or an end
/// cannot, and neither can a removal — taking lines out of an ordered sequence
/// leaves it ordered.
[[nodiscard]] constexpr bool mayBreakOrder(CommandKind kind) {
    switch (kind) {
    case CommandKind::SetStart:
    case CommandKind::Insert:
    case CommandKind::Shift:
    case CommandKind::Transform:
    case CommandKind::ConvertFrameRate:
        return true;
    case CommandKind::SetText:
    case CommandKind::SetEnd:
    case CommandKind::Remove:
    case CommandKind::Sort:
    // It does only what the two above it do — rewrite texts, take subtitles
    // out. Not one position moves.
    case CommandKind::RemoveHearingImpaired:
    // It moves starts, and it is still the only such operation that cannot
    // break the order: rounding to the nearest frame is monotone, so two
    // starts a frame or more apart stay in order, and closer than that they
    // may only coincide.
    case CommandKind::Snap:
        return false;
    }
    // Exhaustive above, and the compiler checks that it is. Only a cast from
    // an out-of-range integer could land here.
    std::unreachable();
}

} // namespace subedit::core
