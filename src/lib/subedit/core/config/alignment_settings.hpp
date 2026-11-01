#pragma once

#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/transcription/transcript_matching_aligner.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

inline constexpr int kDefaultMinimumMatchPercent = 50;
inline constexpr int kMostMinimumMatchPercent = 100;
inline constexpr double kPercent = 100.0;

/// How a subtitle file is laid on the sound (phase 23, #820).
enum class AlignmentMethod : std::uint8_t {
    /// From when there is speech and when a subtitle is shown, whatever the language. Corrects the
    /// file as a whole, and cannot find a line that is alone out of place.
    Speech,

    /// From the words heard: each line is matched to what was said. The text must be in the
    /// language that is spoken, and the sound is transcribed.
    Words,
};

inline constexpr std::string_view kSpeechMethodName = "speech";
inline constexpr std::string_view kWordsMethodName = "words";

/// The method called `name`, or nothing.
[[nodiscard]] constexpr std::optional<AlignmentMethod> alignmentMethodNamed(std::string_view name) {
    if (name == kSpeechMethodName)
        return AlignmentMethod::Speech;
    if (name == kWordsMethodName)
        return AlignmentMethod::Words;
    return std::nullopt;
}

[[nodiscard]] constexpr std::string_view nameOf(AlignmentMethod method) {
    return method == AlignmentMethod::Speech ? kSpeechMethodName : kWordsMethodName;
}

/// What the automatic alignment retains — phase 23, D7 of its spec.
///
/// The model, the voice-activity model and the engine that **listens** are those of
/// `TranscriptionSettings`: there is no second set. These are the choices of the alignment itself.
/// They are read from the settings file where the first surface that shows them is written (the
/// `sync` command, #813); until then they are the defaults.
struct AlignmentSettings {
    /// The alignment engine, as `makeAlignmentEngine` knows it.
    std::string engine{kDefaultAlignmentEngine};

    /// How the file is laid on the sound; see `AlignmentMethod`.
    AlignmentMethod method = AlignmentMethod::Speech;

    /// The share of a line's words that must be heard for the line to be found, in percent.
    int minimumMatchPercent = kDefaultMinimumMatchPercent;

    /// How far a shift may depart from its neighbours', in milliseconds.
    std::int64_t doubtGapMilliseconds = kDefaultDoubtGapMilliseconds;

    /// The same share, as the matching takes it.
    [[nodiscard]] double minimumMatch() const { return minimumMatchPercent / kPercent; }

    friend bool operator==(const AlignmentSettings&, const AlignmentSettings&) = default;
};

} // namespace subedit::core
