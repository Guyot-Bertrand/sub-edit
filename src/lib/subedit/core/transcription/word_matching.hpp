#pragma once

#include <subedit/core/transcription/alignment_engine.hpp>
#include <subedit/core/transcription/transcript.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {

/// The fraction of a line's words that must be heard for the line to be found (ADR 0047). It is a
/// setting of phase 23; this is its default.
inline constexpr double kDefaultMinimumMatch = 0.5;

/// What the matching made of one line of the text.
struct LineMatch {
    /// Where the line is said, from its first heard word to its last. **Nothing** when too few of
    /// its words were heard: a position is never invented.
    AlignedLine place{};

    /// How many words the line has once normalised, and how many of them were heard.
    std::size_t words = 0;
    std::size_t matched = 0;

    friend bool operator==(const LineMatch&, const LineMatch&) = default;
};

/// Matches the lines of a text to the words an engine heard, and answers one `LineMatch` per line,
/// in order.
///
/// A pure function: case, accents, punctuation and markup (styles, bracketed sound notes) are
/// ignored on both sides; the two sequences of words are aligned **in order**, so that the lines
/// come out in the order they are in the file; a heard word that is a slip of the ear for a text
/// word (one letter apart, on a word of some length) counts as heard. A line is found when at least
/// `minimumMatch` of its words are heard, and at least one.
[[nodiscard]] std::vector<LineMatch> matchLinesToWords(const std::vector<std::string>& lines,
                                                       std::span<const TimedWord> heard,
                                                       double minimumMatch = kDefaultMinimumMatch);

} // namespace subedit::core
