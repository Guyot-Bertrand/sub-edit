// What the post-processing of a transcription costs on a full-length film — issue #781, the
// measurement of phase 22.
//
// A film of two hours is about three thousand pieces. Every fifth one is too long and is cut,
// every third one is close to the one before and is merged, and one in ten is a placeholder that is
// dropped: the three steps all work. The assembly is a pure function of the pieces and the limits,
// which is what lets the preview make the right-hand side again at every key typed.

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/transcription/transcript.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using subedit::core::TimedWord;
using subedit::core::Timestamp;
using subedit::core::TranscribedSegment;

constexpr std::size_t kPieces = 3000;
constexpr std::int64_t kPieceMilliseconds = 2400;
constexpr std::int64_t kLongPieceMilliseconds = 9000;

/// Words with their moments, spread over the piece.
std::vector<TimedWord> wordsOf(const std::string& text, Timestamp start, std::int64_t length) {
    std::vector<TimedWord> words;
    std::size_t from = 0;
    std::vector<std::string> pieces;
    while (from < text.size()) {
        const std::size_t end = std::min(text.find(' ', from), text.size());
        pieces.push_back(text.substr(from, end - from));
        from = end + 1;
    }
    const auto each = length / static_cast<std::int64_t>(pieces.size());
    std::int64_t at = 0;
    for (std::string& piece : pieces) {
        words.push_back(
            TimedWord{.text = std::move(piece),
                      .start = Timestamp::fromMilliseconds(start.milliseconds() + at),
                      .end = Timestamp::fromMilliseconds(start.milliseconds() + at + each)});
        at += each;
    }
    return words;
}

std::vector<TranscribedSegment> filmOf(std::size_t count) {
    std::vector<TranscribedSegment> segments;
    segments.reserve(count);
    std::int64_t clock = 0;
    for (std::size_t index = 0; index < count; ++index) {
        const bool tooLong = index % 5 == 0;
        const std::int64_t length = tooLong ? kLongPieceMilliseconds : kPieceMilliseconds;
        std::string text = tooLong ? "When a change comes, some species feel the urge to "
                                     "migrate, and they call it a pull of the soul."
                                   : "They follow a scent";
        if (index % 10 == 7)
            text = "...";
        const Timestamp start = Timestamp::fromMilliseconds(clock);
        std::vector<TimedWord> words = wordsOf(text, start, length);
        segments.push_back(TranscribedSegment{.text = std::move(text),
                                              .start = start,
                                              .end = Timestamp::fromMilliseconds(clock + length),
                                              .words = std::move(words)});
        // Every third piece starts right after the one before, so that it is merged.
        clock += length + (index % 3 == 0 ? 100 : 1500);
    }
    return segments;
}

} // namespace

TEST_CASE("assembling the subtitles of a full-length transcript", "[bench][transcription]") {
    const std::vector<TranscribedSegment> segments = filmOf(kPieces);

    BENCHMARK("assembling 3000 pieces, as the preview does at every key typed") {
        return subedit::core::assemble(segments);
    };
}
