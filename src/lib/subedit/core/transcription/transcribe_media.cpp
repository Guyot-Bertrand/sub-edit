#include <subedit/core/io/atomic_write.hpp>
#include <subedit/core/io/temporary_directory.hpp>
#include <subedit/core/process/start_process.hpp>
#include <subedit/core/transcription/transcribe_media.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace subedit::core {

namespace {

/// The audio containers an engine reads as they are, when `ffmpeg` is not there.
constexpr std::array kReadableAudio{".wav", ".mp3", ".flac", ".ogg"};

[[nodiscard]] bool isReadableAudio(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::ranges::transform(
        extension, extension.begin(), [](unsigned char letter) { return std::tolower(letter); });
    return std::ranges::find(kReadableAudio, extension) != kReadableAudio.end();
}

/// Has `engine` transcribe `audio`, leaving the result in `done`.
std::expected<void, TranscriptionError> transcribeOn(const TranscriptionEngine& engine,
                                                     const MediaTranscriptionRequest& request,
                                                     const TranscriptionObserver& observer,
                                                     std::optional<Transcription>& done,
                                                     const std::filesystem::path& audio) {
    auto transcribed = engine.transcribe({.audio = audio,
                                          .language = request.language,
                                          .model = request.model,
                                          .suppressNotes = request.suppressNotes,
                                          .vadModel = request.vadModel,
                                          .start = request.start,
                                          .end = request.end},
                                         observer);
    if (!transcribed)
        return std::unexpected(transcribed.error());
    done = std::move(*transcribed);
    return {};
}

} // namespace

std::expected<void, MediaTranscriptionError>
withPreparedAudio(FileSystem& files,
                  const EngineCapabilities& capabilities,
                  std::string_view searchPath,
                  const MediaTranscriptionRequest& request,
                  const TranscriptionObserver& observer,
                  const std::function<std::expected<void, TranscriptionError>(
                      const std::filesystem::path& audio)>& use) {
    // **What the engine can do is asked before any work**: the audio is extracted at the rate it
    // wants, and a request it cannot honour — a language, a missing model — is refused before the
    // extraction, which takes a while on a film, rather than after it.
    if (const auto checked =
            checkRequest(capabilities,
                         TranscriptionRequest{
                             .audio = {}, .language = request.language, .model = request.model});
        !checked)
        return std::unexpected(MediaTranscriptionError{.cause = checked.error()});

    const TemporaryDirectory work{"subedit-transcribe"};
    const std::filesystem::path extracted =
        request.keepAudio.empty() ? work.path() / "audio.wav" : request.keepAudio;
    if (!request.keepAudio.empty())
        ensureDirectory(files, extracted.parent_path());

    StreamObserver stop;
    stop.cancelled = [&observer] {
        observer.wait();
        return observer.isCancelled();
    };

    observer.advance({.stage = TranscriptionStage::Extracting});
    std::filesystem::path audio = extracted;
    if (const auto prepared = extractAudio(files,
                                           searchPath,
                                           request.input,
                                           request.audioTrack,
                                           extracted,
                                           capabilities.sampleRate,
                                           stop);
        !prepared) {
        const bool usableAsIs = prepared.error().kind == AudioPreparationErrorKind::ToolMissing &&
                                !request.audioTrack && isReadableAudio(request.input);
        if (!usableAsIs) {
            MediaTranscriptionError error{.cause = prepared.error()};
            if (prepared.error().kind == AudioPreparationErrorKind::NoSuchTrack)
                error.tracks = readAudioTracks(files, searchPath, request.input)
                                   .value_or(std::vector<ContainerAudioTrack>{});
            return std::unexpected(std::move(error));
        }
        audio = request.input;
    }

    if (const auto used = use(audio); !used)
        return std::unexpected(MediaTranscriptionError{
            .cause = used.error(), .tracks = {}, .partial = used.error().partial});
    return {};
}

std::expected<Transcription, MediaTranscriptionError>
transcribeMedia(FileSystem& files,
                const TranscriptionEngine& engine,
                std::string_view searchPath,
                const MediaTranscriptionRequest& request,
                const TranscriptionObserver& observer) {
    const auto capabilities = engine.capabilitiesFor(request.model);
    if (!capabilities)
        return std::unexpected(MediaTranscriptionError{.cause = capabilities.error()});

    std::optional<Transcription> done;
    const auto prepared = withPreparedAudio(files,
                                            *capabilities,
                                            searchPath,
                                            request,
                                            observer,
                                            std::bind_front(transcribeOn,
                                                            std::cref(engine),
                                                            std::cref(request),
                                                            std::cref(observer),
                                                            std::ref(done)));
    if (!prepared)
        return std::unexpected(prepared.error());
    return std::move(done).value_or(Transcription{});
}

} // namespace subedit::core
