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

std::string noticeOfReplacement(std::size_t subtitles) {
    return substitute(
        translatePlural("The %1 subtitle of this project will be replaced by the transcription. "
                        "You can undo it.",
                        "The %1 subtitles of this project will be replaced by the transcription. "
                        "You can undo it.",
                        subtitles),
        {std::to_string(subtitles)});
}

std::string notATimeInRange(std::string_view text) {
    return substitute(
        translate("\"%1\" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds"),
        {text});
}

std::string emptyRange() {
    return std::string{translate("the end of the range must come after its start")};
}

std::string clockText(Timestamp position) {
    return position.format(DecimalMark::Period, HourField::Unpadded, Decimals::Seconds);
}

std::string remainingTimeText(std::chrono::seconds left) {
    constexpr long kMinutesPerHour = 60;
    const long minutes = std::chrono::duration_cast<std::chrono::minutes>(left).count();
    if (minutes < 1)
        return std::string{translate("less than a minute left")};
    if (minutes < kMinutesPerHour)
        return substitute(translate("about %1 min left"), {std::to_string(minutes)});
    const std::string rest = std::to_string(minutes % kMinutesPerHour);
    return substitute(
        translate("about %1 h %2 min left"),
        {std::to_string(minutes / kMinutesPerHour), rest.size() < 2 ? "0" + rest : rest});
}

std::vector<std::string> transcriptionStatusParts(const TranscriptionProgress& progress,
                                                  std::optional<std::chrono::seconds> left) {
    std::vector<std::string> parts;
    if (progress.stage == TranscriptionStage::Extracting)
        return {std::string{translate("extracting the audio…")}};

    constexpr double kPercent = 100.0;
    parts.push_back(progress.fraction
                        ? transcriptionProgressText(static_cast<int>(*progress.fraction * kPercent))
                        : std::string{translate("transcribing…")});
    if (progress.position && progress.total)
        parts.push_back(substitute(translate("%1 of %2"),
                                   {clockText(*progress.position), clockText(*progress.total)}));
    else if (progress.position)
        parts.push_back(clockText(*progress.position));
    if (left)
        parts.push_back(remainingTimeText(*left));
    return parts;
}

std::string lastTextLine(std::string_view text) {
    return substitute(translate("last text: %1"), {text});
}

std::string transcriptionToolsReport(const TranscriptionTools& tools,
                                     const TranscriptionSettings& settings) {
    const std::string engine =
        tools.engine
            ? substitute(translate("%1: found (%2)"), {settings.engine, tools.engine->string()})
            : substitute(translate("%1: not found"), {settings.engine});
    const std::string extractor =
        std::string{tools.extractor ? translate("ffmpeg: found") : translate("ffmpeg: not found")};

    const std::string model = settings.model.string();
    std::string state;
    switch (tools.model) {
    case ModelState::NotSet:
        state = std::string{translate("Model: none set")};
        break;
    case ModelState::NotFound:
        state = substitute(translate("Model: not found (%1)"), {model});
        break;
    case ModelState::NotAModel:
        state = substitute(translate("Model: not a model (%1)"), {model});
        break;
    case ModelState::EnglishOnly:
        state = substitute(translate("Model: English only (%1)"), {model});
        break;
    case ModelState::Multilingual:
        state = substitute(translate("Model: multilingual (%1)"), {model});
        break;
    }
    return engine + "\n" + extractor + "\n" + state;
}

std::string transcriptionProgressText(int percent) {
    return substitute(translate("transcribing: %1%"), {std::to_string(percent)});
}

} // namespace subedit::core
