#pragma once

#include <subedit/core/time/frame.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// What one MicroDVD line says, before its text is read.
///
/// **These are frames, not times.** No MicroDVD file states a rate, which is
/// what makes this format the only one of the nine whose positions mean nothing
/// on their own — ADR 0030 says where the rate comes from.
struct MicroDvdFrameLine {
    Frame start = Frame::fromNumber(0);
    Frame end = Frame::fromNumber(0);

    /// Where the text begins, just past the second brace.
    std::size_t textAt = 0;
};

/// Reads `{25}{75}` at the head of a line, or nothing if that is not what it is.
///
/// **Shared with the detection.** Two braced whole numbers opening a line is
/// MicroDVD and nothing else among the nine — MPL2 writes the same grammar with
/// brackets, which is the only pair the phase had to be careful about.
[[nodiscard]] std::optional<MicroDvdFrameLine> parseMicroDvdFrameLine(std::string_view line);

/// Tells whether `line` is the one header line this format has.
///
/// `{DEFAULT}{}{...}` and nothing else: a line of style defaults that applies
/// to the whole file. Gaupol keeps it verbatim, and so does this.
[[nodiscard]] bool isMicroDvdHeader(std::string_view line);

} // namespace subedit::core
