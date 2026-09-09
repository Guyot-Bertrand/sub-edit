#pragma once

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_reader.hpp>

#include <expected>
#include <string_view>

namespace subedit::core {

/// Reads LRC — the lyrics format, and the last of the nine.
///
/// **One position per line, and it is the start.** `[00:12.34]text` says when
/// a line begins and never when it ends: the ends are worked out here, the
/// next start for each one and five seconds for the last, and the reading says
/// so — ADR 0029.
///
/// **It is also the only one of the nine that cannot hold a line break at
/// all.** A subtitle of two lines comes back on one, its words joined by a
/// space, and that is the one loss of the phase visible to the naked eye in
/// the scene fixture. It has no vocabulary for italics either.
///
/// What it does have is a free header — the `[ar:…]`, `[ti:…]` tags a player
/// reads — which is every line before the first timed one.
class LrcReader final : public SubtitleReader {

public:
    [[nodiscard]] std::expected<ReadResult, ReadError>
    read(std::string_view content) const override;
};

} // namespace subedit::core
