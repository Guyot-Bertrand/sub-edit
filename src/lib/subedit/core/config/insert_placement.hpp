#pragma once

namespace subedit::core {

/// Which side of the selection an insertion lays its rows on.
///
/// **Two values and a setting that is kept, because Gaupol makes one of it** —
/// decision D7 of the scoping of phase 7. `subtitle_insert.above` has been a
/// persisted preference there for twenty years, and the reason is one of use:
/// one does not insert once, one inserts ten rows in a row, always on the same
/// side. Asking for the side every time would be asking again for an answer
/// that never changes.
///
/// An enumeration and not a boolean, for the reason that holds for `Theme`: the
/// configuration file carries words rather than a `true` nobody can guess what
/// it is true of.
enum class InsertPlacement {
    Above, ///< avant la sélection
    Below, ///< après elle, ce qui est le défaut
};

} // namespace subedit::core
