#pragma once

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <optional>

namespace subedit::core {

class Project;

/// Where the duration of a frame was read — issue #618, decision D6.
enum class FrameRateSource {
    Video,    ///< what the container of the associated video declares
    Document, ///< the rate of a document that is counted in frames
    Grid,     ///< the grid the positions themselves fall on
};

/// The frame rate a frame is counted by, and where it came from.
struct CountedFrameRate {
    FrameRate rate;
    FrameRateSource source;
};

/// Returns the rate a frame of `project` lasts by, **in this order**: the one the video declares,
/// the one of a document counted in frames, the grid deduced from the positions.
///
/// **Nothing when there is none, and that is an answer**: a rate chosen at random would move
/// every edge it was applied to by a different length than a frame, and nothing on screen would
/// say so. The caller refuses, and says why.
///
/// The video comes first because it is the only one that says what the *pictures* are; a document
/// counted in frames says what its author counted by, which is usually the same and is not always.
/// The grid is last because it is a deduction, and a verdict that is silent is no rate at all.
[[nodiscard]] std::optional<CountedFrameRate> countedFrameRateOf(const Project& project);

/// Returns `position` moved by `frames` frames of `rate`, earlier when negative — **rounded once**,
/// from the exact rational, so that a step of N is N frames to within the millisecond whatever the
/// rate, and never fewer than the origin.
[[nodiscard]] Timestamp movedByFrames(Timestamp position, FrameRate rate, int frames);

} // namespace subedit::core
