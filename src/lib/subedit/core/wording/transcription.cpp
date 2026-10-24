#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <string>
#include <utility>

namespace subedit::core {

namespace {

/// `sentence`, then the pointer to the guide.
[[nodiscard]] std::string withGuidePointer(std::string sentence) {
    return std::move(sentence) + " " + installationGuidePointer();
}

} // namespace

std::string installationGuidePointer() {
    return std::string{translate(
        "See the manual page \"Installing the transcription tools\" (install-transcription).")};
}

std::string transcriptionErrorText(const TranscriptionError& error) {
    switch (error.kind) {
    case TranscriptionErrorKind::EngineMissing:
        return withGuidePointer(std::string{translate("the transcription engine was not found")});
    case TranscriptionErrorKind::ModelMissing:
        return withGuidePointer(
            std::string{translate("no model was given, or the model file is not there")});
    case TranscriptionErrorKind::UnsupportedLanguage:
        return substitute(translate("the model cannot transcribe the language %1"), {error.detail});
    case TranscriptionErrorKind::LanguageRequired:
        return std::string{translate("the model cannot detect the language: give one")};
    case TranscriptionErrorKind::AudioUnreadable:
        return std::string{translate("the audio could not be read")};
    case TranscriptionErrorKind::Failed:
        if (error.detail.empty())
            return std::string{translate("the transcription failed")};
        return substitute(translate("the transcription failed: %1"), {error.detail});
    case TranscriptionErrorKind::Cancelled:
        break;
    }
    return std::string{translate("the transcription was cancelled")};
}

std::string audioPreparationErrorText(const AudioPreparationError& error) {
    switch (error.kind) {
    case AudioPreparationErrorKind::ToolMissing:
        return withGuidePointer(std::string{
            translate("ffmpeg was not found, so the audio of a video cannot be extracted")});
    case AudioPreparationErrorKind::NoSuchTrack:
        return substitute(translate("the video has no audio track %1"), {error.detail});
    case AudioPreparationErrorKind::Failed:
        if (error.detail.empty())
            return std::string{translate("the audio could not be extracted")};
        return substitute(translate("the audio could not be extracted: %1"), {error.detail});
    case AudioPreparationErrorKind::Cancelled:
        break;
    }
    return std::string{translate("the transcription was cancelled")};
}

std::string audioTracksText(const std::vector<ContainerAudioTrack>& tracks) {
    if (tracks.empty())
        return std::string{translate("the video has no audio track")};

    std::string text;
    for (const ContainerAudioTrack& track : tracks) {
        std::string tags = track.language;
        if (!track.title.empty())
            tags += tags.empty() ? track.title : ", " + track.title;
        if (tags.empty())
            tags = std::string{translate("no language or title")};
        if (!text.empty())
            text += '\n';
        text += substitute(translate("audio track %1: %2"), {std::to_string(track.position), tags});
    }
    return text;
}

std::string noSuchTranscriptionEngine(std::string_view name) {
    return substitute(translate("no transcription engine is called %1"), {name});
}

std::string noticeOfTranscription(std::size_t subtitles, std::string_view language) {
    return substitute(translatePlural("%1 subtitle transcribed, language: %2",
                                      "%1 subtitles transcribed, language: %2",
                                      subtitles),
                      {std::to_string(subtitles), language});
}

std::string transcriptionProgressText(int percent) {
    return substitute(translate("transcribing: %1%"), {std::to_string(percent)});
}

} // namespace subedit::core
