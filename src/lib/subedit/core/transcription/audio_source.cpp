#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/transcription/audio_source.hpp>

#include <charconv>
#include <system_error>
#include <utility>

namespace subedit::core {

namespace {

constexpr std::string_view kProbeName = "ffprobe";
constexpr std::string_view kExtractorName = "ffmpeg";

constexpr std::string_view kStreamEnd = "[/STREAM]";
constexpr std::string_view kLanguageTag = "TAG:language=";
constexpr std::string_view kTitleTag = "TAG:title=";

[[nodiscard]] std::vector<std::string> probeArguments(const std::filesystem::path& video) {
    return {"-v",
            "error",
            "-select_streams",
            "a",
            "-show_entries",
            "stream=index:stream_tags=language,title",
            video.string()};
}

/// The arguments that extract one track as 16 kHz mono 16-bit PCM.
[[nodiscard]] std::vector<std::string> extractionArguments(
    const std::filesystem::path& source, int track, const std::filesystem::path& destination) {
    return {"-v",
            "error",
            "-nostdin",
            "-y",
            "-i",
            source.string(),
            "-map",
            "0:a:" + std::to_string(track),
            "-vn",
            "-ac",
            "1",
            "-ar",
            "16000",
            "-c:a",
            "pcm_s16le",
            destination.string()};
}

} // namespace

std::vector<ContainerAudioTrack> parseAudioTracks(std::string_view probeOutput) {
    std::vector<ContainerAudioTrack> tracks;
    ContainerAudioTrack current;
    while (!probeOutput.empty()) {
        const std::size_t end = probeOutput.find('\n');
        const std::string_view line = probeOutput.substr(0, end);
        probeOutput.remove_prefix(end == std::string_view::npos ? probeOutput.size() : end + 1);

        if (line.starts_with(kLanguageTag)) {
            current.language = std::string{line.substr(kLanguageTag.size())};
        } else if (line.starts_with(kTitleTag)) {
            current.title = std::string{line.substr(kTitleTag.size())};
        } else if (line == kStreamEnd) {
            current.position = static_cast<int>(tracks.size());
            tracks.push_back(std::exchange(current, ContainerAudioTrack{}));
        }
    }
    return tracks;
}

std::optional<std::vector<ContainerAudioTrack>> readAudioTracks(
    const FileSystem& files, std::string_view searchPath, const std::filesystem::path& video) {
    const std::optional<std::filesystem::path> probe =
        findExecutable(files, kProbeName, searchPath);
    if (!probe)
        return std::nullopt;

    const std::expected<ProgramOutput, LaunchError> answered =
        runAndCapture(*probe, probeArguments(video));
    if (!answered || answered->code != 0)
        return std::nullopt;
    return parseAudioTracks(answered->output);
}

std::expected<void, AudioPreparationError> extractAudio(const FileSystem& files,
                                                        std::string_view searchPath,
                                                        const std::filesystem::path& source,
                                                        std::optional<int> track,
                                                        const std::filesystem::path& destination,
                                                        const StreamObserver& observer) {
    const std::optional<std::filesystem::path> extractor =
        findExecutable(files, kExtractorName, searchPath);
    if (!extractor)
        return std::unexpected(
            AudioPreparationError{.kind = AudioPreparationErrorKind::ToolMissing});

    const int wanted = track.value_or(0);
    // When the probe can count the tracks, a track that is not there is told
    // before anything is run; when it cannot, the extraction says so itself.
    if (const auto tracks = readAudioTracks(files, searchPath, source);
        tracks && !tracks->empty() && std::cmp_greater_equal(wanted, tracks->size()))
        return std::unexpected(AudioPreparationError{.kind = AudioPreparationErrorKind::NoSuchTrack,
                                                     .detail = std::to_string(wanted)});

    std::string lastLine;
    StreamObserver streamed = observer;
    streamed.onLine = [&](const StreamedLine& line) {
        if (line.stream == StreamName::Error)
            lastLine = line.text;
        if (observer.onLine)
            observer.onLine(line);
    };

    const std::expected<StreamedRun, LaunchError> ran =
        runStreaming(*extractor, extractionArguments(source, wanted, destination), streamed);

    std::optional<AudioPreparationError> failure;
    if (!ran)
        failure = AudioPreparationError{.kind = AudioPreparationErrorKind::Failed,
                                        .detail = ran.error().detail};
    else if (ran->cancelled)
        failure = AudioPreparationError{.kind = AudioPreparationErrorKind::Cancelled};
    else if (ran->code != 0)
        failure =
            AudioPreparationError{.kind = AudioPreparationErrorKind::Failed, .detail = lastLine};

    if (!failure)
        return {};
    std::error_code ignored;
    std::filesystem::remove(destination, ignored);
    return std::unexpected(*failure);
}

} // namespace subedit::core
