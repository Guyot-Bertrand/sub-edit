#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/file_system.hpp>

#include <variant>

namespace subedit::core {

/// Why a file could not be opened at all.
///
/// **Two errors meet here, and neither is crushed into the other.** Opening a
/// path is two steps — the system hands over bytes, then a reader makes sense
/// of them — and each fails its own way. `FileError` says absent, refused, or
/// broken; `ReadError` says not UTF-8, no format, nothing recognisable. Four
/// causes on one side, three on the other, and the user asking « why won't my
/// file open » is asking for exactly one of the seven.
///
/// The window used to translate every `FileError` into
/// `ReadErrorKind::NoSubtitleFound`, so a missing file and a Word document
/// produced the same sentence — and only one of the two was true. The command
/// line never had that problem, because it kept the two errors apart. This
/// type is what lets both surfaces keep them apart without repeating anything.
///
/// **A sum rather than a third enumeration.** Merging the seven causes into one
/// list would mean maintaining a mapping in both directions and would break the
/// rule both enumerations follow: every one of their values is produced by
/// something. Here nothing is produced that was not already; only the choice is
/// new. `reasonOf` in `core/wording.hpp` puts words on it.
using OpenError = std::variant<FileError, ReadError>;

} // namespace subedit::core
