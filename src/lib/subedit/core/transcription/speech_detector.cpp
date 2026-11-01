#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/process/start_process.hpp>
#include <subedit/core/transcription/speech_detector.hpp>

#include <charconv>
#include <cmath>
#include <string>
#include <utility>

namespace subedit::core {

namespace {

constexpr std::string_view kSegmentLine = "Speech segment ";
constexpr std::string_view kStartKey = "start = ";
constexpr std::string_view kEndKey = "end = ";

/// No audio is longer than a day: a stretch that ends later is not read, and neither is a number
/// that is not finite (`from_chars` reads `inf` and `nan`, and a comparison with them is false).
constexpr double kLongestAudioHundredths = 24.0 * 3600.0 * 100.0;

[[nodiscard]] TranscriptionError error(TranscriptionErrorKind kind, std::string detail = {}) {
    return TranscriptionError{.kind = kind, .detail = std::move(detail)};
}

/// The number after `key` in `line`, in hundredths of a second.
[[nodiscard]] std::optional<double> numberAfter(std::string_view line, std::string_view key) {
    const std::size_t at = line.find(key);
    if (at == std::string_view::npos)
        return std::nullopt;
    const std::string_view rest = line.substr(at + key.size());
    double value = 0.0;
    const auto [end, code] = std::from_chars(rest.data(), rest.data() + rest.size(), value);
    if (code != std::errc{})
        return std::nullopt;
    return value;
}

[[nodiscard]] std::optional<SpeechSpan> spanOfLine(std::string_view line) {
    if (!line.starts_with(kSegmentLine))
        return std::nullopt;
    const std::optional<double> start = numberAfter(line, kStartKey);
    const std::optional<double> end = numberAfter(line, kEndKey);
    if (!start || !end || !std::isfinite(*start) || !std::isfinite(*end) || *start < 0.0 ||
        *end < *start || *end > kLongestAudioHundredths)
        return std::nullopt;
    constexpr double kMillisecondsPerHundredth = 10.0;
    return SpeechSpan{
        .start = Timestamp::fromMilliseconds(std::llround(*start * kMillisecondsPerHundredth)),
        .end = Timestamp::fromMilliseconds(std::llround(*end * kMillisecondsPerHundredth))};
}

} // namespace

std::optional<std::filesystem::path> findVadTool(const FileSystem& files,
                                                 std::string_view searchPath) {
    return findExecutable(files, kVadToolName, searchPath);
}

std::vector<SpeechSpan> parseSpeechSegments(std::string_view output) {
    std::vector<SpeechSpan> spans;
    while (!output.empty()) {
        const std::size_t newline = output.find('\n');
        const std::string_view line = output.substr(0, newline);
        if (const std::optional<SpeechSpan> span = spanOfLine(line))
            spans.push_back(*span);
        output =
            newline == std::string_view::npos ? std::string_view{} : output.substr(newline + 1);
    }
    return spans;
}

WhisperVadDetector::WhisperVadDetector(std::filesystem::path executable)
    : m_executable(std::move(executable)) {}

std::expected<std::vector<SpeechSpan>, TranscriptionError>
WhisperVadDetector::detect(const std::filesystem::path& audio,
                           const std::optional<std::filesystem::path>& model,
                           const TranscriptionObserver& observer) const {
    if (!std::filesystem::exists(m_executable))
        return std::unexpected(error(TranscriptionErrorKind::EngineMissing, m_executable.string()));
    if (!model || !std::filesystem::exists(*model))
        return std::unexpected(
            error(TranscriptionErrorKind::ModelMissing, model ? model->string() : std::string{}));
    if (!std::filesystem::exists(audio))
        return std::unexpected(error(TranscriptionErrorKind::AudioUnreadable, audio.string()));

    const std::vector<std::string> arguments{"-f", audio.string(), "-vm", model->string(), "-np"};

    std::string printed;
    std::string lastError;
    StreamObserver streamed;
    streamed.cancelled = [&observer] {
        observer.wait();
        return observer.isCancelled();
    };
    streamed.onLine = [&](const StreamedLine& line) {
        if (line.stream == StreamName::Output)
            printed.append(line.text).push_back('\n');
        else if (line.text.starts_with("error:"))
            lastError = line.text;
    };

    const std::expected<StreamedRun, LaunchError> ran =
        runStreaming(m_executable, arguments, streamed);
    if (!ran)
        return std::unexpected(error(ran.error().kind == LaunchErrorKind::NotFound
                                         ? TranscriptionErrorKind::EngineMissing
                                         : TranscriptionErrorKind::Failed,
                                     ran.error().detail));
    if (ran->cancelled)
        return std::unexpected(error(TranscriptionErrorKind::Cancelled));
    if (ran->code != 0)
        return std::unexpected(error(TranscriptionErrorKind::Failed, std::move(lastError)));
    return parseSpeechSegments(printed);
}

} // namespace subedit::core
