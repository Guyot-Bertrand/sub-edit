#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/remaining_time.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/transcription/time_range.hpp>
#include <subedit/core/transcription/transcribe_media.hpp>
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

constexpr double kPercent = 100.0;

/// Says how the work goes, as it goes: **one line per percent and per step** at the second
/// level of detail, and each new text at the third. The time left is worked out from the
/// fractions it sees.
class ProgressNarration {
public:
    explicit ProgressNarration(const Reporter& reporter) : m_reporter(reporter) {}

    void operator()(const core::TranscriptionProgress& progress) {
        const auto now = core::RemainingTime::Clock::now();
        if (progress.fraction)
            m_remaining.note(*progress.fraction, now);

        const int percent =
            progress.fraction ? static_cast<int>(*progress.fraction * kPercent) : -1;
        const bool changed =
            !m_shown || m_shown->first != progress.stage || m_shown->second != percent;
        if (changed) {
            m_shown = std::pair{progress.stage, percent};
            std::string line;
            for (const std::string& part :
                 core::transcriptionStatusParts(progress, m_remaining.left(now)))
                line += (line.empty() ? "" : ", ") + part;
            m_reporter.say(2, line);
        }
        if (!progress.lastText.empty() && progress.lastText != m_lastText) {
            m_lastText = progress.lastText;
            m_reporter.say(3, core::lastTextLine(m_lastText));
        }
    }

private:
    const Reporter& m_reporter;
    core::RemainingTime m_remaining;
    std::optional<std::pair<core::TranscriptionStage, int>> m_shown{};
    std::string m_lastText;
};

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

/// What a transcription that came to nothing said: the words for the surface, and for the
/// record the identifier a script can test.
[[nodiscard]] Failure failureOf(const core::MediaTranscriptionError& error) {
    if (const auto* audio = std::get_if<core::AudioPreparationError>(&error.cause))
        return Failure{idOf(audio->kind), [&] {
                           std::string text = core::audioPreparationErrorText(*audio);
                           if (audio->kind == core::AudioPreparationErrorKind::NoSuchTrack)
                               text += "\n" + core::audioTracksText(error.tracks);
                           return text;
                       }};
    const auto& engine = std::get<core::TranscriptionError>(error.cause);
    return Failure{idOf(engine.kind), [&] { return core::transcriptionErrorText(engine); }};
}

} // namespace

ExitCode transcribeFile(core::FileSystem& files,
                        const core::EngineRegistry& engines,
                        std::string_view searchPath,
                        const TranscribeOptions& options,
                        const core::TranscriptionSettings& settings,
                        const Reporter& reporter,
                        const std::function<bool()>& cancelled) {
    const std::string engineName = options.engine.empty() ? settings.engine : options.engine;
    const core::TranscriptionEngine* const engine = engines.find(engineName);
    if (engine == nullptr) {
        reporter.failed(core::noSuchTranscriptionEngine(engineName));
        return ExitCode::Usage;
    }

    const std::string& languageName =
        options.language.empty() ? settings.language : options.language;
    const std::filesystem::path& modelPath =
        options.model.empty() ? settings.model : std::filesystem::path{options.model};

    core::MediaTranscriptionRequest request{
        .input = options.input, .audioTrack = options.audioTrack, .keepAudio = options.keepAudio};
    if (!languageName.empty())
        request.language = languageName;
    if (!modelPath.empty())
        request.model = modelPath;

    for (const auto& [text, bound] :
         {std::pair<const std::string&, std::optional<core::Timestamp>&>{options.start,
                                                                         request.start},
          std::pair<const std::string&, std::optional<core::Timestamp>&>{options.end,
                                                                         request.end}}) {
        if (text.empty())
            continue;
        bound = core::readRangeBound(text);
        if (!bound) {
            reporter.failed(core::notATimeInRange(text));
            return ExitCode::Usage;
        }
    }
    if (!core::isRangePossible(request.start, request.end)) {
        reporter.failed(core::emptyRange());
        return ExitCode::Usage;
    }

    ProgressNarration narration{reporter};
    const core::TranscriptionObserver observer{
        .advanced =
            [&narration](const core::TranscriptionProgress& progress) { narration(progress); },
        .cancelled = cancelled};
    const auto transcribed = core::transcribeMedia(files, *engine, searchPath, request, observer);
    if (!transcribed) {
        reportFailure(reporter, options.input, failureOf(transcribed.error()));
        return ExitCode::AllFailed;
    }

    const std::vector<core::Subtitle> subtitles =
        options.raw ? rawSubtitlesOf(*transcribed)
                    : core::assembleSubtitles(transcribed->segments, core::assemblyOf(settings));

    core::ensureDirectory(files, std::filesystem::path{options.output}.parent_path());
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
