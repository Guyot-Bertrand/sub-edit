#pragma once

#include <subedit/core/transcription/transcript.hpp>

#include <cstddef>
#include <optional>
#include <string_view>

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

/// The progress, from 0 to 1, that a line of standard error reports — nothing
/// for any other line. The tool writes `progress =  25%` and the like.
[[nodiscard]] std::optional<double> readWhisperProgress(std::string_view line);

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
