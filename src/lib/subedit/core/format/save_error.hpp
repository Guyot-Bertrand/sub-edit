#pragma once

#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/file_system.hpp>

#include <variant>

namespace subedit::core {

/// Why a file could not be saved at all.
///
/// **The mirror of `OpenError`, and for the same reason** — ADR 0025. Saving is
/// two steps: the subtitles become bytes, then the system takes them. Each
/// fails its own way, and crushing one into the other would give the user a
/// sentence that is true of the wrong step. A disk that is full and a subtitle
/// holding a character the encoding cannot write are not the same problem, and
/// only one of the two is theirs to fix by choosing something else.
///
/// `reasonOf` in `core/wording.hpp` puts words on it, once, for both surfaces.
using SaveError = std::variant<WriteError, FileError>;

} // namespace subedit::core
