#pragma once

// The encodings the tool can read and write, said once and on purpose.

#include <iosfwd>

namespace subedit::cli {

/// Writes every encoding ICU converts and the model carries, one a line, sorted
/// by name — and says nothing else.
///
/// **In `json`, one object per encoding**, `{"schema":1,"command":"list-encodings",
/// "name":"…"}`, and the exception to the envelope that ADR 0038 now writes: the
/// objects of a file carry `file` and `ok`, and an encoding is not a file's
/// account. It is still one object per entry, one entry per line, so that the
/// stream cuts, concatenates and filters like the others.
///
/// **Asked of the core, never written down here**: the set is the one
/// `--encoding` accepts, name by name, and it depends on the ICU the machine has —
/// which is why no test fixes its size.
///
/// Returns how many it wrote.
std::size_t listEncodings(std::ostream& out, bool json);

} // namespace subedit::cli
