#pragma once

// The one way the command line writes an amount of time.

#include <subedit/core/time/duration.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::cli {

/// Reads an amount of time, or says why the text is not one.
///
/// Two forms, and only two:
///
///     [-]S[.mmm]        seconds — `30`, `2.999`, `-7.001`
///     [-]HH:MM:SS.mmm   a timestamp — `01:39:37.040`, `-00:00:07.001`
///
/// **The decimal mark is the point**, the command line speaking English. A
/// comma is refused rather than interpreted: half the world writes `7,001` for
/// seven thousand and one, and reading it as seven seconds would be a guess
/// that is wrong for that half.
///
/// **The resolution is the millisecond**, as the model's (ADR 0006). Anything
/// finer is refused rather than rounded: a silently rounded value gives a
/// result matching nothing that was asked for.
[[nodiscard]] std::expected<subedit::core::Duration, std::string> parseTime(std::string_view text);

} // namespace subedit::cli
