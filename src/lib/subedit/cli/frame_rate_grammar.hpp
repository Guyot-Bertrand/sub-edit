#pragma once

// The one way the command line writes a number of frames per second.

#include <subedit/core/time/frame_rate.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::cli {

/// Reads a frame rate, or says why the text is not one.
///
/// Written in frames per second, decimals included: `25`, `24`, `23.976`. The
/// decimal mark is the point, as everywhere else on this command line, and a
/// comma is refused rather than interpreted.
///
/// **The three NTSC spellings name their exact rate.** `23.976`, `29.97` and
/// `59.94` are labels for `24000/1001`, `30000/1001` and `60000/1001` — none of
/// the three is a terminating decimal, and reading them literally would make
/// this tool disagree with every other, and with its own window in phase 5,
/// about what the same words mean. Recognised by **value** and not by letters,
/// so `23.9760` names the same rate.
///
/// Any other decimal is taken at face value: `23.9` is nobody's standard, and
/// there is nothing to read into it.
///
/// A null or negative rate is refused — a rate is a rate — and so is one whose
/// terms pass the bound the core sets, which is what keeps every product of a
/// conversion inside a `std::int64_t`.
[[nodiscard]] std::expected<subedit::core::FrameRate, std::string>
parseFrameRate(std::string_view text);

} // namespace subedit::cli
