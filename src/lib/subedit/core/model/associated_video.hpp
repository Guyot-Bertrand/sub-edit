#pragma once

#include <subedit/core/time/frame_rate.hpp>

#include <filesystem>
#include <optional>

namespace subedit::core {

/// Where the path of the associated video came from.
///
/// The distinction is not bookkeeping: decision D5 of the phase-6 spec rests
/// on it. The naming convention **proposes**, and a proposal is allowed to be
/// replaced by a later one; a user's choice **decides**, and nothing guessed
/// ever overwrites it — not opening another file, not saving under a new name.
enum class VideoOrigin {
    Guessed, ///< found by the naming convention, beside the subtitle file
    Chosen,  ///< named by the user
};

/// The video a document is watched against.
struct AssociatedVideo {
    std::filesystem::path path;
    VideoOrigin origin = VideoOrigin::Chosen;

    /// What the container declares as its frame rate, or nothing.
    ///
    /// **Nothing is the ordinary answer**, not a failure: `ffprobe` may not be
    /// installed, and the file may declare no video stream. Whoever reads this
    /// proposes what it holds and asks the user otherwise — decision D6, and
    /// the reason it is remembered rather than recomputed: the phase that
    /// deduces a rate from the positions themselves will want to set this one
    /// beside its own, and a source that has been overwritten cannot be
    /// compared with anything.
    std::optional<FrameRate> declared{};

    friend bool operator==(const AssociatedVideo&, const AssociatedVideo&) = default;
};

} // namespace subedit::core
