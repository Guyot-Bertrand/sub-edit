#include <subedit/cli/records.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/cli/writing.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/language_codes.hpp>
#include <subedit/core/transcription/language_display.hpp>
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

} // namespace

/// What a transcription that came to nothing said: the words for the surface, and for the
/// record the identifier a script can test.
Failure failureOf(const core::MediaTranscriptionError& error) {
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

ProgressNarration::ProgressNarration(const Reporter& reporter,
                                     std::function<Clock::time_point()> clock,
                                     std::chrono::seconds heartbeat)
    : m_reporter(reporter),
      m_clock(std::move(clock)),
      m_heartbeat(heartbeat),
      m_started(m_clock()),
      m_lastSaid(m_started) {
    m_remaining.note(0.0, m_started);
}

void ProgressNarration::operator()(const core::TranscriptionProgress& progress) {
    const Clock::time_point now = m_clock();
    if (progress.fraction)
        m_remaining.note(*progress.fraction, now);
    m_last = progress;

    const int percent = progress.fraction ? static_cast<int>(*progress.fraction * kPercent) : -1;
    if (!m_shown || m_shown->first != progress.stage || m_shown->second != percent) {
        m_shown = std::pair{progress.stage, percent};
        say(progress, now);
    }
    if (!progress.lastText.empty() && progress.lastText != m_lastText) {
        m_lastText = progress.lastText;
        m_reporter.say(3, core::lastTextLine(m_lastText));
    }
}

void ProgressNarration::idle() {
    const Clock::time_point now = m_clock();
    if (m_last && now - m_lastSaid >= m_heartbeat)
        say(*m_last, now);
}

void ProgressNarration::say(const core::TranscriptionProgress& progress, Clock::time_point now) {
    m_lastSaid = now;
    std::string line;
    for (const std::string& part : core::transcriptionStatusParts(
             progress,
             m_remaining.left(now),
             std::chrono::duration_cast<std::chrono::seconds>(now - m_started)))
        line += (line.empty() ? "" : ", ") + part;
    m_reporter.say(2, line);
}

namespace {

/// `--list-languages`: what the engine offers for the model, one language per line.
[[nodiscard]] ExitCode listLanguagesOf(const core::TranscriptionEngine& engine,
                                       const std::filesystem::path& model,
                                       const Reporter& reporter) {
    const auto capabilities =
        engine.capabilitiesFor(model.empty() ? std::nullopt : std::optional{model});
    if (!capabilities) {
        reporter.failed(core::transcriptionErrorText(capabilities.error()));
        return ExitCode::AllFailed;
    }
    const std::string reader{core::interfaceLanguageName()};
    for (const std::string& code : capabilities->languages)
        reporter.result(code + '\t' + core::languageDisplayName(code, reader) + '\n');
    return ExitCode::Success;
}

/// What is missing or contradictory in the arguments, said, as the exit code to leave with.
[[nodiscard]] std::optional<ExitCode> refusedArguments(const TranscribeOptions& options,
                                                       const Reporter& reporter) {
    for (const auto& [value, name] :
         {std::pair<const std::string&, std::string_view>{options.input, "input"},
          std::pair<const std::string&, std::string_view>{options.output, "--output"}}) {
        if (value.empty()) {
            reporter.failed(core::substitute(core::translate("%1 is required"), {name}));
            return ExitCode::Usage;
        }
    }
    if (options.suppressNotes && options.keepNotes) {
        reporter.failed(core::substitute(
            // TRANSLATORS: %1 and %2 are two options, such as --shorten and --no-shorten.
            core::translate("%1 and %2 say opposite things; give one of them"),
            {"--suppress-notes", "--keep-notes"}));
        return ExitCode::Usage;
    }
    return std::nullopt;
}

} // namespace

std::optional<ExitCode> readListenerChoices(const std::string& languageName,
                                            const std::string& startText,
                                            const std::string& endText,
                                            core::MediaTranscriptionRequest& request,
                                            const Reporter& reporter) {
    if (!languageName.empty()) {
        request.language = core::readLanguageCode(languageName);
        if (!request.language) {
            reporter.failed(core::notALanguageCode(languageName));
            return ExitCode::Usage;
        }
    }
    for (const auto& [text, bound] :
         {std::pair<const std::string&, std::optional<core::Timestamp>&>{startText, request.start},
          std::pair<const std::string&, std::optional<core::Timestamp>&>{endText, request.end}}) {
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
    return std::nullopt;
}

/// What the voice-activity model asked for allows. **An option names a file, and is refused by name
/// when the engine cannot use it or when the file is missing; a setting is dropped without a word**
/// for an engine that cannot, since a setting is written for all the engines.
std::optional<ExitCode> checkVoiceModel(const std::string& vadOption,
                                        const core::EngineCapabilities* capabilities,
                                        const std::string& engineName,
                                        const core::FileSystem& files,
                                        core::MediaTranscriptionRequest& request,
                                        const Reporter& reporter) {
    if (capabilities != nullptr && !capabilities->voiceActivity) {
        if (!vadOption.empty()) {
            reporter.failed(core::engineCannotDetectVoice(engineName));
            return ExitCode::Usage;
        }
        request.vadModel.reset();
    }
    if (request.vadModel && !vadOption.empty() && !files.exists(*request.vadModel)) {
        reporter.failed(core::voiceModelNotFound(request.vadModel->string()));
        return ExitCode::Usage;
    }
    return std::nullopt;
}

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

    if (options.listLanguages)
        return listLanguagesOf(*engine, modelPath, reporter);

    if (const std::optional<ExitCode> refused = refusedArguments(options, reporter))
        return *refused;

    core::MediaTranscriptionRequest request{
        .input = options.input, .audioTrack = options.audioTrack, .keepAudio = options.keepAudio};
    // The setting decides, an option overrides it.
    request.suppressNotes = options.suppressNotes || (!options.keepNotes && settings.suppressNotes);
    if (!modelPath.empty())
        request.model = modelPath;
    // The option names a file; the setting only says where one is, and is dropped for an engine
    // that cannot use it.
    const std::filesystem::path& voiceModel =
        options.vadModel.empty() ? settings.vadModel : std::filesystem::path{options.vadModel};
    if (!voiceModel.empty())
        request.vadModel = voiceModel;
    if (const std::optional<ExitCode> refused =
            readListenerChoices(languageName, options.start, options.end, request, reporter))
        return *refused;

    const auto capabilities = engine->capabilitiesFor(request.model);
    // The wildcards of the name of the kept audio are the engine's: the rate it asked for.
    if (!options.keepAudio.empty())
        request.keepAudio =
            core::expandAudioName(options.keepAudio, capabilities ? capabilities->sampleRate : 0);
    // What the person asked for by name, and the engine cannot do, is refused before any work; what
    // only a setting says is dropped without a word, since a setting is written for all the
    // engines.
    if (options.suppressNotes && capabilities && !capabilities->suppressNotes) {
        reporter.failed(core::engineCannotSuppressNotes(engineName));
        return ExitCode::Usage;
    }

    if (const std::optional<ExitCode> refused =
            checkVoiceModel(options.vadModel,
                            capabilities ? &*capabilities : nullptr,
                            engineName,
                            files,
                            request,
                            reporter))
        return *refused;

    // A model for English alone writes everything as English: said before the long work, not after.
    if (capabilities && !capabilities->languageDetection && capabilities->languages.size() == 1 &&
        capabilities->languages.front() == "en")
        reporter.say(1, core::noticeOfEnglishOnlyModel());

    ProgressNarration narration{reporter};
    const core::TranscriptionObserver observer{
        .advanced =
            [&narration](const core::TranscriptionProgress& progress) { narration(progress); },
        .cancelled = cancelled,
        .waiting = [&narration] { narration.idle(); }};
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
        fields.emplace_back("audio", request.keepAudio.string());
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
