#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string>
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
/// forget the same thing. The break marker below is the reason it became a
/// file: `[br]` has to be read and written the same way, and nothing else in
/// the project knows about it.
[[nodiscard]] std::optional<SubViewer2TimeLine> parseSubViewer2TimeLine(std::string_view line);

/// Turns one file line into the text of a subtitle, `[br]` becoming a break.
[[nodiscard]] std::string textFromSubViewer2(std::string_view line);

/// Turns the text of a subtitle into one file line, a break becoming `[br]`.
[[nodiscard]] std::string textToSubViewer2(std::string_view text);

} // namespace subedit::core
