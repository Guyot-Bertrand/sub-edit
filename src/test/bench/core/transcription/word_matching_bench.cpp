// What matching a text to heard words costs on a full-length film — issue #810, phase 23.
//
// A film of two hours is about three thousand lines of six words. The ear misses one word in ten.
// Two vocabularies: a wide one, where most words occur once and pin the alignment, and a narrow one
// of sixty words, where none does and the one gap is the whole film — larger than the matching
// agrees to fill, so it answers at once that nothing was found (the honest answer, bounded memory).

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/transcript.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using subedit::core::TimedWord;
using subedit::core::Timestamp;

constexpr std::size_t kLines = 3000;
constexpr std::size_t kWordsPerLine = 6;
constexpr std::size_t kMissedEvery = 10;
constexpr std::int64_t kWordMilliseconds = 400;
constexpr std::size_t kNarrowVocabulary = 60;

struct Film {
    std::vector<std::string> lines;
    std::vector<TimedWord> heard;
};

Film filmOf(std::size_t vocabulary) {
    Film film;
    film.lines.reserve(kLines);
    std::int64_t clock = 0;
    std::size_t spoken = 0;
    for (std::size_t line = 0; line < kLines; ++line) {
        std::string text;
        for (std::size_t at = 0; at < kWordsPerLine; ++at) {
            const std::size_t number = (line * kWordsPerLine + at) * 7919 % vocabulary;
            const std::string word = "word" + std::to_string(number);
            text += (at == 0 ? "" : " ") + word;
            if (++spoken % kMissedEvery != 0)
                film.heard.push_back(
                    TimedWord{.text = word,
                              .start = Timestamp::fromMilliseconds(clock),
                              .end = Timestamp::fromMilliseconds(clock + kWordMilliseconds)});
            clock += kWordMilliseconds;
        }
        film.lines.push_back(std::move(text));
    }
    return film;
}

} // namespace

TEST_CASE("matching the text of a full-length film to what was heard", "[bench][transcription]") {
    const Film wide = filmOf(kLines * kWordsPerLine);
    const Film narrow = filmOf(kNarrowVocabulary);

    BENCHMARK("3000 lines, words that occur once") {
        return subedit::core::matchLinesToWords(wide.lines, wide.heard);
    };
    BENCHMARK("3000 lines, sixty words, one gap beyond the limit") {
        return subedit::core::matchLinesToWords(narrow.lines, narrow.heard);
    };
}
