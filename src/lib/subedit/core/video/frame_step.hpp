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

/// Returns the rate the **positions are counted in frames** at, **in this order**: the document's
/// own when it is counted in frames, then the one the video declares, then the grid deduced from
/// the positions — issue #620, decision D8.
///
/// **The document first, where `countedFrameRateOf` puts the video first**, and the reason is the
/// question asked. A step of N frames is a distance on the film, which only the film can measure.
/// The number a MicroDVD file writes is a number *of its own*, at the rate it was read with, and
/// showing anything else would show digits the file does not contain.
///
/// Nothing when there is none: the setting is then out, and says why.
[[nodiscard]] std::optional<CountedFrameRate> numberingFrameRateOf(const Project& project);

/// Returns `position` moved by `frames` frames of `rate`, earlier when negative — **rounded once**,
/// from the exact rational, so that a step of N is N frames to within the millisecond whatever the
/// rate, and never fewer than the origin.
[[nodiscard]] Timestamp movedByFrames(Timestamp position, FrameRate rate, int frames);

} // namespace subedit::core
