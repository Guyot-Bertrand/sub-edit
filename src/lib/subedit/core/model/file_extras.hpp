#pragma once

#include <subedit/core/time/frame_rate.hpp>

#include <string>
#include <variant>
#include <vector>

namespace subedit::core {

/// What a Sub Station Alpha file declares about its own events.
///
/// The `[Events]` section opens on a `Format:` line naming its columns, and
/// **the order varies from one producer to the next**. Writing has to read it
/// back to compose each `Dialogue:` line, so it cannot live in the free header
/// text: it would have to be parsed out of it at every writing.
struct SubStationAlphaFile {
    /// The event field names, in the order the file declares them.
    std::vector<std::string> eventFields{};

    friend bool operator==(const SubStationAlphaFile&, const SubStationAlphaFile&) = default;
};

/// What a TMPlayer file declares by the shape of its timestamps.
///
/// `0:00:12:` and `00:00:12:` are both TMPlayer, and a file uses one or the
/// other throughout. Nothing else in the file says which, so a reading that
/// forgets it writes the other one back and changes every line.
struct TMPlayerFile {
    bool twoDigitHour = true;

    friend bool operator==(const TMPlayerFile&, const TMPlayerFile&) = default;
};

/// The rate a MicroDVD file was read at.
///
/// **A MicroDVD file counts in frames and declares no rate** — its positions
/// *are* frame numbers, `{1194}{1285}`. The model counts in whole milliseconds
/// since ADR 0006, so reading one needs a rate from outside, and writing it
/// back needs the same one. Keeping it here is what makes that round trip
/// exact, whatever the rate was.
///
/// The default is Gaupol's, and iso-functionality is the only reason to prefer
/// it to 25: two tools opening the same file have to give the same timestamps.
/// ADR 0030 holds the reasoning, and the three sources it comes from.
struct MicroDvdFile {
    FrameRate rate{StandardFrameRate::Fps23976};

    friend bool operator==(const MicroDvdFile&, const MicroDvdFile&) = default;
};

/// What a file declares about itself, beyond its subtitles and its header.
///
/// The twin of `FormatExtras` one level up: that one holds what a *subtitle*
/// carries beyond its positions and text, this one what a *file* carries
/// beyond its subtitles. Both are variants for the same reason — the compiler
/// knows which fields exist, and asking a SubRip document for its event fields
/// does not compile.
///
/// **The free header stays the string it is.** It suffices to what a round trip
/// has to give back, because nothing needs to understand it. What is here is
/// the rest: the three things a writer has to *read* rather than repeat.
using FileExtras = std::variant<std::monostate, SubStationAlphaFile, TMPlayerFile, MicroDvdFile>;

} // namespace subedit::core
