// What both surfaces say of a transcription.

#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <vector>

namespace {

using Catch::Matchers::ContainsSubstring;
using subedit::core::AudioPreparationError;
using subedit::core::AudioPreparationErrorKind;
using subedit::core::ContainerAudioTrack;
using subedit::core::TranscriptionError;
using subedit::core::TranscriptionErrorKind;

std::string said(TranscriptionErrorKind kind, std::string detail = {}) {
    return subedit::core::transcriptionErrorText({.kind = kind, .detail = std::move(detail)});
}

std::string saidOfAudio(AudioPreparationErrorKind kind, std::string detail = {}) {
    return subedit::core::audioPreparationErrorText({.kind = kind, .detail = std::move(detail)});
}

} // namespace

TEST_CASE("what is to be installed is said with the pointer to the guide",
          "[wording][transcription]") {
    for (const std::string& sentence : {said(TranscriptionErrorKind::EngineMissing),
                                        said(TranscriptionErrorKind::ModelMissing),
                                        saidOfAudio(AudioPreparationErrorKind::ToolMissing)})
        CHECK_THAT(sentence, ContainsSubstring("Installing the transcription tools"));
}

TEST_CASE("each way a transcription fails has its own sentence", "[wording][transcription]") {
    CHECK(said(TranscriptionErrorKind::EngineMissing)
              .starts_with("the transcription engine was not found"));
    CHECK(said(TranscriptionErrorKind::ModelMissing).starts_with("no model was given"));
    CHECK(said(TranscriptionErrorKind::UnsupportedLanguage, "fr") ==
          "the model cannot transcribe the language fr");
    CHECK(said(TranscriptionErrorKind::LanguageRequired) ==
          "the model cannot detect the language: give one");
    CHECK(said(TranscriptionErrorKind::AudioUnreadable) == "the audio could not be read");
    CHECK(said(TranscriptionErrorKind::Failed) == "the transcription failed");
    CHECK(said(TranscriptionErrorKind::Failed, "exit 4") == "the transcription failed: exit 4");
    CHECK(said(TranscriptionErrorKind::Cancelled) == "the transcription was cancelled");
}

TEST_CASE("each way the audio can fail to be prepared has its own sentence",
          "[wording][transcription]") {
    CHECK(saidOfAudio(AudioPreparationErrorKind::NoSuchTrack, "3") ==
          "the video has no audio track 3");
    CHECK(saidOfAudio(AudioPreparationErrorKind::Failed) == "the audio could not be extracted");
    CHECK(saidOfAudio(AudioPreparationErrorKind::Failed, "bad") ==
          "the audio could not be extracted: bad");
    CHECK(saidOfAudio(AudioPreparationErrorKind::Cancelled) == "the transcription was cancelled");
}

TEST_CASE("the tracks of a video are listed with what the file says of them",
          "[wording][transcription]") {
    CHECK(subedit::core::audioTracksText({}) == "the video has no audio track");

    const std::vector<ContainerAudioTrack> tracks{
        {0, "fra", "Original"}, {1, "eng", ""}, {2, "", "Commentary"}, {3, "", ""}};
    CHECK(subedit::core::audioTracksText(tracks) ==
          "audio track 0: fra, Original\naudio track 1: eng\naudio track 2: Commentary\n"
          "audio track 3: no language or title");
}

TEST_CASE("the result and the progress are counted", "[wording][transcription]") {
    CHECK(subedit::core::noticeOfTranscription(1, "en") == "1 subtitle transcribed, language: en");
    CHECK(subedit::core::noticeOfTranscription(12, "fr") ==
          "12 subtitles transcribed, language: fr");
    CHECK(subedit::core::transcriptionProgressText(40) == "transcribing: 40%");
    CHECK(subedit::core::noSuchTranscriptionEngine("x") == "no transcription engine is called x");
}

// ## The language a container declares

TEST_CASE("a container's language becomes the code an engine takes", "[transcription][language]") {
    using subedit::core::twoLetterCodeOf;

    CHECK(twoLetterCodeOf("fra") == "fr");
    CHECK(twoLetterCodeOf("fre") == "fr");
    CHECK(twoLetterCodeOf("ENG") == "en");
    CHECK(twoLetterCodeOf("de") == "de");
    CHECK(twoLetterCodeOf("zho") == "zh");
}

TEST_CASE("a tag that names no language of the table gives nothing", "[transcription][language]") {
    using subedit::core::twoLetterCodeOf;

    CHECK_FALSE(twoLetterCodeOf("und").has_value());
    CHECK_FALSE(twoLetterCodeOf("").has_value());
    CHECK_FALSE(twoLetterCodeOf("xx").has_value());
}
