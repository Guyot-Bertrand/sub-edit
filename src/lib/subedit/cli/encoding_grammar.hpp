#pragma once

// The one way the command line names an encoding.

#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::cli {

/// Reads an encoding, or says why the text names none.
///
/// **The set is ICU's, and no list of it is written here** — ADR 0027. Anything
/// ICU can convert is accepted, under any of the names it knows it by: `cp1252`,
/// `windows-1252` and `WINDOWS 1252` are one encoding, and the value keeps the
/// spelling ICU settles on so that a report names it the same way twice.
///
/// **What is refused is a name nothing can convert**, and it is refused here
/// rather than at the moment of reading a file: an option is answered while the
/// user is still being asked something.
///
/// A closed set is not offered on purpose. Ninety-seven encodings is not a help
/// message, and the short list a menu will offer is the window's business — the
/// command line takes what ICU takes.
[[nodiscard]] std::expected<subedit::core::Encoding, std::string>
encodingNamed(std::string_view text);

} // namespace subedit::cli
