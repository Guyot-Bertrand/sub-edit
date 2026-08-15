#pragma once

// Reading a run of decimal digits into an integer, without wrapping.

#include <cstdint>
#include <optional>

namespace subedit::cli {

/// Multiplies `value` by ten and adds `digit`, or nothing if that would not
/// hold.
///
/// The three grammars of the command line — the time, the subtitle number, the
/// frame rate — all read a run of digits, and all of them need the same guard.
/// It lives here because it was written three times and **missing from one of
/// them**: `shift --by 99999999999999999999` overflowed a signed integer, which
/// is undefined behaviour, and then wrote a file shifted by two hundred
/// thousand billion seconds, with a successful exit code.
///
/// Checked before the multiplication rather than detected after it. After is
/// too late: a signed overflow has no value to look at, it has a compiler free
/// to assume it never happened.
///
/// `digit` is the character, not its value, so that no caller writes
/// `digit - '0'` a fourth time. Appending `'0'` is how a decimal place is
/// made — ten times as many parts.
///
/// `value` is expected non-negative: every grammar reads the sign apart and
/// builds the magnitude. Nothing here would catch a negative one.
[[nodiscard]] std::optional<std::int64_t> appendedDigit(std::int64_t value, char digit);

} // namespace subedit::cli
