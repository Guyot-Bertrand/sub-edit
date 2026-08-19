#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <optional>
#include <string_view>

namespace subedit::core {

/// Recognises the format of `content`, or nothing if none claims it.
///
/// **Nothing rather than a guess.** A file whose format cannot be established
/// produces an explicit failure upstream; opening it as the wrong format would
/// either fail obscurely or, worse, half succeed.
///
/// The most specific format wins: the WebVTT signature settles the question
/// whatever follows it, so a WebVTT file whose timestamps were written with
/// commas is a malformed WebVTT file and not a SubRip one.
///
/// The rules match what the readers accept, deliberately. A detection that
/// promised a format its reader then refuses would be worse than no detection
/// at all.
[[nodiscard]] std::optional<SubtitleFormat> detectFormat(std::string_view content);

} // namespace subedit::core
