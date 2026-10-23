#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {

/// One word of a transcription, with the moment it is said.
///
/// **Optional all the way down**: an engine may give no word at all, and the
/// first adapter obtains them from a splitting option of the tool rather than
/// from a field of its own, so that their timings are approximate. Phase 23
/// reuses them; it must not trust them more than the engine does.
struct TimedWord {
    /// The word as the engine wrote it, without the space that separates it
    /// from the previous one.
    std::string text;

    Timestamp start;
    Timestamp end;

    /// How sure the engine is, from 0 to 1, when it says.
    std::optional<double> confidence{};

    friend bool operator==(const TimedWord&, const TimedWord&) = default;
};

/// One piece of speech as an engine cut it — not yet a subtitle.
///
/// An engine cuts where it likes. Turning segments into entries that respect
/// the reading limits is the post-processing's job (phase 22, D7), and it
/// works from this type alone, so that it knows nothing of any engine.
struct TranscribedSegment {
    std::string text;

    Timestamp start;
    Timestamp end;

    /// The words of the segment in order, each inside the segment — empty when
    /// the engine gives none.
    std::vector<TimedWord> words{};

    /// How sure the engine is of the whole segment, from 0 to 1, when it says.
    std::optional<double> confidence{};

    /// Who speaks, when the engine tells. The engines of the first adapter do
    /// not, and nothing downstream may count on it.
    std::optional<std::string> speaker{};

    friend bool operator==(const TranscribedSegment&, const TranscribedSegment&) = default;
};

/// What an engine made of an audio file.
struct Transcription {
    /// In the order they are said, starts never decreasing.
    std::vector<TranscribedSegment> segments;

    /// The language of the speech as an ISO 639-1 code (`en`, `fr`): the one
    /// the engine detected, or the one it was asked for. Nothing when the
    /// engine does not say.
    std::optional<std::string> language{};

    friend bool operator==(const Transcription&, const Transcription&) = default;
};

} // namespace subedit::core
