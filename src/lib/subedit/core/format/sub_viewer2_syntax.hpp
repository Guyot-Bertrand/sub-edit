#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string_view>

namespace subedit::core {

/// What one SubViewer 2 timestamp line says.
struct SubViewer2TimeLine {
    Timestamp start;
    Timestamp end;
};

/// Reads `00:00:01.00,00:00:03.00`, or nothing if that is not what the line is.
///
/// **Shared by the reader, the writer and the detection**, which is a departure
/// from what SubRip does — there, detection carries its own narrower check and
/// says why. Here the three ask the same question of the same grammar, and the
/// grammar is small enough that three copies of it would be three places to
/// forget the same thing.
[[nodiscard]] std::optional<SubViewer2TimeLine> parseSubViewer2TimeLine(std::string_view line);

} // namespace subedit::core
