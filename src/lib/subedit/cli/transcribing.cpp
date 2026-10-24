#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/temporary_directory.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/core/wording/transcription.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

using core::TranscriptionErrorKind;

/// The audio containers an engine reads as they are, when `ffmpeg` is not there.
constexpr std::array kReadableAudio{".wav", ".mp3", ".flac", ".ogg"};

[[nodiscard]] bool isReadableAudio(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::ranges::transform(
        extension, extension.begin(), [](unsigned char letter) { return std::tolower(letter); });
    return std::ranges::find(kReadableAudio, extension) != kReadableAudio.end();
}

[[nodiscard]] std::string_view idOf(TranscriptionErrorKind kind) {
    switch (kind) {
    case TranscriptionErrorKind::EngineMissing:
        return "engine-missing";
    case TranscriptionErrorKind::ModelMissing:
        return "model-missing";
    case TranscriptionErrorKind::UnsupportedLanguage:
        return "unsupported-language";
    case TranscriptionErrorKind::LanguageRequired:
        return "language-required";
    case TranscriptionErrorKind::AudioUnreadable:
        return "audio-unreadable";
    case TranscriptionErrorKind::Failed:
        return "failed";
    case TranscriptionErrorKind::Cancelled:
        break;
    }
    return "cancelled";
}

[[nodiscard]] std::string_view idOf(core::AudioPreparationErrorKind kind) {
    switch (kind) {
    case core::AudioPreparationErrorKind::ToolMissing:
        return "tool-missing";
    case core::AudioPreparationErrorKind::NoSuchTrack:
        return "no-such-track";
    case core::AudioPreparationErrorKind::Failed:
        return "audio-extraction-failed";
    case core::AudioPreparationErrorKind::Cancelled:
        break;
    }
    return "cancelled";
}

/// The format to write: the one asked for, else the one the extension of `output` says, else
/// SubRip.
[[nodiscard]] core::SubtitleFormat formatToWrite(const TranscribeOptions& options) {
    if (const auto named = core::formatNamed(options.format))
        return *named;
    std::string extension = std::filesystem::path{options.output}.extension().string();
    std::ranges::transform(
        extension, extension.begin(), [](unsigned char letter) { return std::tolower(letter); });
    for (const core::SubtitleFormat format : core::kSubtitleFormats)
        if (core::extensionOf(format) == extension)
            return format;
    return core::SubtitleFormat::SubRip;
}

[[nodiscard]] std::vector<core::Subtitle> rawSubtitlesOf(const core::Transcription& transcription) {
    std::vector<core::Subtitle> subtitles;
    subtitles.reserve(transcription.segments.size());
    for (const core::TranscribedSegment& segment : transcription.segments)
        subtitles.push_back(
            core::Subtitle{.start = segment.start, .end = segment.end, .mainText = segment.text});
    return subtitles;
}

/// What the audio preparation said, with the tracks the video has when it was a track that
/// was missing.
[[nodiscard]] Failure preparationFailure(const core::AudioPreparationError& error,
                                         const std::string& tracks) {
    return Failure{idOf(error.kind), [&] {
                       std::string text = core::audioPreparationErrorText(error);
                       if (!tracks.empty())
                           text += "\n" + tracks;
                       return text;
                   }};
}

} // namespace

ExitCode transcribeFile(core::FileSystem& files,
                        const core::EngineRegistry& engines,
                        std::string_view searchPath,
                        const TranscribeOptions& options,
                        const core::TranscriptionSettings& settings,
                        const Reporter& reporter) {
    const std::string engineName = options.engine.empty() ? settings.engine : options.engine;
    const core::TranscriptionEngine* const engine = engines.find(engineName);
    if (engine == nullptr) {
        reporter.failed(core::noSuchTranscriptionEngine(engineName));
        return ExitCode::Usage;
    }

    const core::TemporaryDirectory work{"subedit-transcribe"};
    const std::filesystem::path extracted = options.keepAudio.empty()
                                                ? work.path() / "audio.wav"
                                                : std::filesystem::path{options.keepAudio};

    // Where the files go is made if it is not there yet: writing is part of it. A directory that
    // cannot be made is told by the write that follows.
    if (!options.keepAudio.empty())
        (void)files.createDirectories(extracted.parent_path());

    std::filesystem::path audio = extracted;
    if (const auto prepared =
            core::extractAudio(files, searchPath, options.input, options.audioTrack, extracted);
        !prepared) {
        const bool usableAsIs =
            prepared.error().kind == core::AudioPreparationErrorKind::ToolMissing &&
            !options.audioTrack && isReadableAudio(options.input);
        if (!usableAsIs) {
            std::string tracks;
            if (prepared.error().kind == core::AudioPreparationErrorKind::NoSuchTrack)
                tracks =
                    core::audioTracksText(core::readAudioTracks(files, searchPath, options.input)
                                              .value_or(std::vector<core::ContainerAudioTrack>{}));
            reportFailure(reporter, options.input, preparationFailure(prepared.error(), tracks));
            return ExitCode::AllFailed;
        }
        audio = options.input;
    }

    core::TranscriptionRequest request{.audio = audio};
    const std::string& languageName =
        options.language.empty() ? settings.language : options.language;
    if (!languageName.empty())
        request.language = languageName;
    const std::filesystem::path& modelPath =
        options.model.empty() ? settings.model : std::filesystem::path{options.model};
    if (!modelPath.empty())
        request.model = modelPath;

    const core::TranscriptionObserver observer{.progress = [&reporter](double fraction) {
        reporter.say(2, core::transcriptionProgressText(static_cast<int>(fraction * 100)));
    }};
    const auto transcribed = engine->transcribe(request, observer);
    if (!transcribed) {
        reportFailure(reporter, options.input, Failure{idOf(transcribed.error().kind), [&] {
                                                           return core::transcriptionErrorText(
                                                               transcribed.error());
                                                       }});
        return ExitCode::AllFailed;
    }

    const std::vector<core::Subtitle> subtitles =
        options.raw ? rawSubtitlesOf(*transcribed)
                    : core::assembleSubtitles(transcribed->segments, core::assemblyOf(settings));

    (void)files.createDirectories(std::filesystem::path{options.output}.parent_path());
    const core::SubtitleFormat format = formatToWrite(options);
    const auto written = writeSubtitlesTo(files, options.output, format, {.subtitles = subtitles});
    if (!written) {
        reportFailure(reporter, options.input, written.error().about(options.input));
        return ExitCode::AllFailed;
    }

    const std::string language = transcribed->language.value_or(std::string{});
    const std::string sentence = core::noticeOfTranscription(subtitles.size(), language);
    reporter.say(1,
                 // TRANSLATORS: %1 is a file, %2 what was done to it, %3 the file written.
                 core::substitute(core::translate("%1: %2 -> %3"),
                                  {options.input, sentence, options.output}));

    Fields fields;
    fields.emplace_back("language", language);
    if (!options.keepAudio.empty())
        fields.emplace_back("audio", options.keepAudio);
    reporter.record(
        writtenRecord(reporter.command(),
                      options.input,
                      options.output,
                      {{"subtitles", static_cast<std::int64_t>(subtitles.size())},
                       {"segments", static_cast<std::int64_t>(transcribed->segments.size())}},
                      Json::array(),
                      std::nullopt,
                      fields));
    return ExitCode::Success;
}

} // namespace subedit::cli
