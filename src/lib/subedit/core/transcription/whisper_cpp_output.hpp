#pragma once

#include <subedit/core/transcription/transcript.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// What `whisper-cli` wrote, read.
struct WhisperOutput {
    Transcription transcription;

    /// Whether the model that produced it was multilingual.
    bool multilingual = false;
};

/// Reads the JSON file `whisper-cli -oj -ojf` writes, or nothing when it is not
/// one — a run killed in the middle of a write leaves half a document.
///
/// What the real tool writes (checked on whisper.cpp 1.9.5-dev, see
/// `src/test/data/whisper/`), and what this does about it:
///
/// - the segment text keeps the space that begins it: **it is removed**;
/// - the words are not a field of their own. They are rebuilt from the
///   **tokens**: a token that begins with a space begins a word, the others
///   (a comma, the end of a long word) are appended to the word before. The
///   word spans from the start of its first token to the end of its last, and
///   its confidence is the mean of its tokens';
/// - the special tokens (`[_BEG_]`, `[_TT_…]`) are not words and are dropped;
/// - a segment with no text is dropped;
/// - the timings of the tokens are the tool's own and approximate: a word can
///   begin and end at the same moment.
[[nodiscard]] std::optional<WhisperOutput> readWhisperJson(std::string_view text);

/// Puts the words of every segment back on the film's time, for a run that used a voice-activity
/// model.
///
/// **With `--vad`, the tool writes the times of the segments on the film's time and the times of
/// the tokens on the timeline with the silences removed** (measured on a real film, #808: a segment
/// at 118 s whose tokens are at 67 s). The words are right relative to one another, so the words of
/// a segment are **shifted so that the first begins where the segment begins**, and kept inside it.
/// Without this, the post-processing cuts a piece at times that are not those of its own.
void putWordsOnTheSegments(std::vector<TranscribedSegment>& segments);

/// The progress, from 0 to 1, that a line of standard error reports — nothing
/// for any other line. The tool writes `progress =  25%` and the like.
[[nodiscard]] std::optional<double> readWhisperProgress(std::string_view line);

/// A piece of speech as the tool prints it while it works.
struct WhisperSegmentLine {
    Timestamp start;
    Timestamp end;
    std::string text;
};

/// A line of standard output of the form `[00:01:00.000 --> 00:01:17.000]   And so…`, read —
/// nothing for any other line.
///
/// **This is what tells how far the tool is**, long before its own progress does: the tool
/// reports the progress by steps of 5 % of the audio and only once a window of thirty
/// seconds is done, which on a long film and a large model on a processor is a quarter of
/// an hour of silence at 0 %. The segments come as they are decoded.
[[nodiscard]] std::optional<WhisperSegmentLine> readWhisperSegmentLine(std::string_view line);

/// The length of the audio that the line the tool prints when it starts —
/// `main: processing 'f.wav' (176000 samples, 11.0 sec), …` — gives, or nothing for any
/// other line.
[[nodiscard]] std::optional<Timestamp> readWhisperAudioLength(std::string_view line);

/// Whether the model whose first bytes are `header` is multilingual — nothing
/// when they are not those of a model.
///
/// A `ggml` model begins with the four bytes `lmgg` (the magic number, written
/// little-endian) and the size of its vocabulary as a 32-bit integer. Whisper
/// decides the same way: **51 865 words and more is a multilingual model**,
/// 51 864 an English-only one (checked on the headers of `ggml-tiny.bin` and
/// `ggml-tiny.en.bin`). Reading it spares the file name, which is a convention
/// and not a fact.
[[nodiscard]] std::optional<bool> readModelIsMultilingual(std::string_view header);

/// How many bytes `readModelIsMultilingual` needs.
inline constexpr std::size_t kModelHeaderSize = 8;

} // namespace subedit::core
