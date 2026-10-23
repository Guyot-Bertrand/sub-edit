// Reading what whisper-cli really wrote: the versioned samples of
// `src/test/data/whisper/`, captured from a run of the tool.

#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <iterator>
#include <string>

namespace {

using subedit::core::readModelIsMultilingual;
using subedit::core::readWhisperJson;
using subedit::core::readWhisperProgress;
using subedit::core::Timestamp;
using subedit::core::WhisperOutput;

std::string sample(const std::string& name) {
    std::ifstream file{std::string{SUBEDIT_TEST_DATA_DIR} + "/whisper/" + name, std::ios::binary};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

/// What the tool wrote in `text`, or a failed requirement.
WhisperOutput read(std::string_view text) {
    const std::optional<WhisperOutput> output = readWhisperJson(text);
    return output.value_or(WhisperOutput{});
}

/// The first bytes of a model: the magic number and the vocabulary, little-endian.
std::string header(unsigned vocabulary) {
    std::string bytes = "lmgg";
    for (int index = 0; index < 4; ++index)
        bytes += static_cast<char>((vocabulary >> (8 * index)) & 0xFFU);
    return bytes;
}

} // namespace

TEST_CASE("the full output of a run gives the segment, the language and the model", "[whisper]") {
    const WhisperOutput output = read(sample("jfk.full.json"));

    CHECK(output.multilingual);
    CHECK(output.transcription.language == "en");
    REQUIRE(output.transcription.segments.size() == 1);
    const auto& segment = output.transcription.segments.front();
    CHECK(segment.text.starts_with("And so, my fellow Americans"));
    CHECK(segment.text.ends_with("for your country."));
    CHECK(segment.start == Timestamp::fromMilliseconds(0));
    CHECK(segment.end == Timestamp::fromMilliseconds(10500));
}

TEST_CASE("the words are rebuilt from the tokens, and the special ones are dropped", "[whisper]") {
    const WhisperOutput output = read(sample("jfk.full.json"));

    const auto& words = output.transcription.segments.front().words;
    REQUIRE(words.size() == 22);
    CHECK(words.front().text == "And");
    CHECK(words.front().start == Timestamp::fromMilliseconds(320));
    // The comma is a token of its own, and belongs to the word before.
    CHECK(words[1].text == "so,");
    CHECK(words.back().text == "country.");
    CHECK(words.back().end == Timestamp::fromMilliseconds(10500));
    for (const auto& word : words) {
        CHECK(word.text.find("[_") == std::string::npos);
        CHECK(word.text.front() != ' ');
        const double confidence = word.confidence.value_or(-1.0);
        CHECK(confidence >= 0.0);
        CHECK(confidence <= 1.0);
    }
}

TEST_CASE("output without tokens gives segments and no words", "[whisper]") {
    const WhisperOutput output = read(sample("jfk.json"));

    REQUIRE(output.transcription.segments.size() == 1);
    CHECK(output.transcription.segments.front().words.empty());
}

TEST_CASE("a segment with no text is dropped", "[whisper]") {
    const WhisperOutput output = read(sample("jfk.words.json"));

    // The tool writes an empty first entry before the first word.
    for (const auto& segment : output.transcription.segments)
        CHECK_FALSE(segment.text.empty());
}

TEST_CASE("output that is cut short, or is not whisper's, is not read", "[whisper]") {
    const std::string complete = sample("jfk.full.json");

    CHECK_FALSE(readWhisperJson(complete.substr(0, complete.size() / 2)).has_value());
    CHECK_FALSE(readWhisperJson("{}").has_value());
    CHECK_FALSE(readWhisperJson(R"({"transcription": {}})").has_value());
    CHECK_FALSE(readWhisperJson(R"({"transcription": [{"text": "x"}]})").has_value());
    CHECK_FALSE(readWhisperJson(
                    R"({"transcription": [{"text": "x", "offsets": {"from": "a", "to": "b"}}]})")
                    .has_value());
    // An offset no film has, which would overflow once converted to milliseconds.
    CHECK_FALSE(readWhisperJson(
                    R"({"transcription": [{"text": "x", "offsets": {"from": 0, "to": 1e300}}]})")
                    .has_value());
    CHECK_FALSE(readWhisperJson(
                    R"({"transcription": [{"text": "x", "offsets": {"from": -1e300, "to": 0}}]})")
                    .has_value());
    CHECK_FALSE(
        readWhisperJson(R"({"transcription": [{"offsets": {"from": 0, "to": 1}}]})").has_value());
}

TEST_CASE("tokens that lack a text or an offset are skipped", "[whisper]") {
    const WhisperOutput output = read(R"({
      "transcription": [{
        "text": " Hi", "offsets": {"from": 0, "to": 500},
        "tokens": [
          {"offsets": {"from": 0, "to": 100}},
          {"text": " Hi", "offsets": {"from": 0}},
          {"text": " Hi", "offsets": {"from": 0, "to": 500}}
        ]}]})");

    const auto& words = output.transcription.segments.front().words;
    REQUIRE(words.size() == 1);
    CHECK(words.front().text == "Hi");
    CHECK_FALSE(words.front().confidence.has_value());
}

TEST_CASE("the first token begins a word even without a leading space", "[whisper]") {
    const WhisperOutput output = read(R"({
      "transcription": [{
        "text": "Hi", "offsets": {"from": 0, "to": 500},
        "tokens": [{"text": "Hi", "offsets": {"from": 0, "to": 500}, "p": 0.5}]}]})");

    CHECK(output.transcription.segments.front().words.front().text == "Hi");
    CHECK(output.transcription.segments.front().words.front().confidence == 0.5);
}

TEST_CASE("progress lines are read as a fraction", "[whisper]") {
    CHECK(readWhisperProgress("whisper_print_progress_callback: progress =  25%") == 0.25);
    CHECK(readWhisperProgress("whisper_print_progress_callback: progress = 100%") == 1.0);
}

TEST_CASE("a line that is not progress says nothing", "[whisper]") {
    CHECK_FALSE(
        readWhisperProgress("whisper_full_with_state: auto-detected language: en").has_value());
    CHECK_FALSE(readWhisperProgress("progress = x").has_value());
    CHECK_FALSE(readWhisperProgress("progress = 25").has_value());
    CHECK_FALSE(readWhisperProgress("progress =").has_value());
}

TEST_CASE("a model is multilingual from the size of its vocabulary", "[whisper]") {
    // 51865 and 51864 are those of ggml-tiny.bin and ggml-tiny.en.bin.
    CHECK(readModelIsMultilingual(header(51865)) == true);
    CHECK(readModelIsMultilingual(header(51864)) == false);
}

TEST_CASE("something that is not a model has no language capability", "[whisper]") {
    CHECK_FALSE(readModelIsMultilingual("lmgg").has_value());
    CHECK_FALSE(readModelIsMultilingual("GGUF\x99\xca\x00\x00").has_value());
    CHECK_FALSE(readModelIsMultilingual("").has_value());
}
