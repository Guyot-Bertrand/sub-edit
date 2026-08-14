#pragma once

// How much the tool says, from what the caller asked for.

#include <expected>
#include <string>

namespace subedit::cli {

/// The level to narrate at, or the reason the request cannot be honoured.
///
/// Silence is 0; `-v` and no flag at all are both 1. Saying `-v` when it is
/// already the default is not an error, it is a caller being explicit.
///
/// Asking for silence **and** for detail is refused rather than arbitrated in
/// favour of the last one written: two opposite intentions in the same line
/// mean the caller was not understood, and saying so is what tells them.
[[nodiscard]] std::expected<int, std::string> levelFrom(bool quiet, int verboseCount);

} // namespace subedit::cli
