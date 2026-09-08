#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads MPL2 — one of the two formats that hide behind a `.txt`.
///
/// **The simplest of the nine.** No header, no numbering, one line per
/// subtitle: `[10][30]texte`, where the two numbers are tenths of a second.
///
/// The tenth is the coarsest grain of any of the time formats, and it is the
/// whole of what this format loses. Everything else it carries: both positions,
/// the line breaks as `|`, and its own way of writing italics — a `/` opening
/// a line, which stays in the text as ADR 0009 wants.
class Mpl2Reader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
