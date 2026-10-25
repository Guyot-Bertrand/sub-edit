#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/io/temporary_directory.hpp>
#include <subedit/core/process/start_process.hpp>
#include <subedit/core/transcription/whisper_cpp_engine.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kExecutableName = "whisper-cli";

/// What whisper.cpp reads without converting: 16 kHz, whatever the file is.
constexpr int kSampleRate = 16000;

/// The languages a multilingual model transcribes, by whisper.cpp's own table (`g_lang`, checked on
/// 1.9.5-dev): English first, as it lists them. Declared as the capability, so that a surface
/// offers the closed set instead of a free field.
constexpr std::array kMultilingual{
    "en", "zh", "de", "es",  "ru", "ko", "fr", "ja", "pt", "tr", "pl", "ca", "nl", "ar", "sv",
    "it", "id", "hi", "fi",  "vi", "he", "uk", "el", "ms", "cs", "ro", "da", "hu", "ta", "no",
    "th", "ur", "hr", "bg",  "lt", "la", "mi", "ml", "cy", "sk", "te", "fa", "lv", "bn", "sr",
    "az", "sl", "kn", "et",  "mk", "br", "eu", "is", "hy", "ne", "mn", "bs", "kk", "sq", "sw",
    "gl", "mr", "pa", "si",  "km", "sn", "yo", "so", "af", "oc", "ka", "be", "tg", "sd", "gu",
    "am", "yi", "lo", "uz",  "fo", "ht", "ps", "tk", "nn", "mt", "sa", "lb", "my", "bo", "tl",
    "mg", "as", "tt", "haw", "ln", "ha", "ba", "jw", "su", "yue"};

/// What `whisper-cli` exits with, checked on 1.9.5-dev.
constexpr int kMissingInput = 2;
constexpr int kMissingModel = 3;
constexpr int kNotAudio = 11;

[[nodiscard]] std::string readAll(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

/// The first `size` bytes of `path`, and no more: a model weighs hundreds of megabytes, and the
/// windows ask for its capabilities every time they open and at every key typed in its field.
[[nodiscard]] std::string readHead(const std::filesystem::path& path, std::size_t size) {
    std::ifstream file{path, std::ios::binary};
    std::string head(size, '\0');
    file.read(head.data(), static_cast<std::streamsize>(size));
    head.resize(static_cast<std::size_t>(file.gcount()));
    return head;
}

[[nodiscard]] TranscriptionError error(TranscriptionErrorKind kind, std::string detail = {}) {
    return TranscriptionError{.kind = kind, .detail = std::move(detail)};
}

/// The error the tool's exit code stands for.
[[nodiscard]] TranscriptionError errorOfExit(int code, std::string lastError) {
    switch (code) {
    case kMissingModel:
        return error(TranscriptionErrorKind::ModelMissing, std::move(lastError));
    case kMissingInput:
    case kNotAudio:
        return error(TranscriptionErrorKind::AudioUnreadable, std::move(lastError));
    default:
        return error(TranscriptionErrorKind::Failed, std::move(lastError));
    }
}

/// What a run has learnt of how far it is.
///
/// **The tool's own progress comes in steps of 5 % and after a window of thirty seconds**, so
/// that a long film on a large model stays at 0 % for a long while. The segments it prints
/// as it decodes them are the finer clock, and the two are folded into one fraction that
/// only goes up.
class RunProgress {
public:
    RunProgress(const TranscriptionRequest& request, const TranscriptionObserver& observer)
        : m_observer(observer),
          m_start(request.start.value_or(Timestamp::origin())),
          m_end(request.end) {}

    void audioLength(Timestamp length) {
        m_total = m_end ? std::min(*m_end, length) : length;
        m_state.total = m_total;
    }

    /// The pieces printed so far.
    [[nodiscard]] std::vector<TranscribedSegment> printed() const { return m_printed; }

    void segment(const WhisperSegmentLine& line) {
        m_printed.push_back(
            TranscribedSegment{.text = line.text, .start = line.start, .end = line.end});
        m_state.position = m_total ? std::min(line.end, *m_total) : line.end;
        m_state.lastText = line.text;
        if (const std::optional<double> done = fractionAt(line.end))
            raise(std::min(*done, kBeforeTheEnd));
        publish();
    }

    void windowDone(double fraction) {
        raise(fraction);
        publish();
    }

    void begin() { publish(); }

    void finish() {
        m_state.fraction = 1.0;
        publish();
    }

private:
    /// What a position inside the range stands for, once the end of the range is known.
    [[nodiscard]] std::optional<double> fractionAt(Timestamp position) const {
        if (!m_total || m_total->milliseconds() <= m_start.milliseconds())
            return std::nullopt;
        const auto span = static_cast<double>(m_total->milliseconds() - m_start.milliseconds());
        const auto done = static_cast<double>(position.milliseconds() - m_start.milliseconds());
        return std::clamp(done / span, 0.0, 1.0);
    }

    void raise(double fraction) {
        m_state.fraction = std::max(m_state.fraction.value_or(0.0), fraction);
    }

    void publish() const { m_observer.advance(m_state); }

    /// A fraction taken from the segments never reaches 1: only the end of the run says so.
    static constexpr double kBeforeTheEnd = 0.99;

    const TranscriptionObserver& m_observer;
    Timestamp m_start;
    std::optional<Timestamp> m_end;
    std::optional<Timestamp> m_total{};
    TranscriptionProgress m_state{.stage = TranscriptionStage::Transcribing};
    std::vector<TranscribedSegment> m_printed{};
};

/// The arguments that restrict the tool to the range of `request`.
void addRange(const TranscriptionRequest& request, std::vector<std::string>& arguments) {
    const std::int64_t start = request.start ? request.start->milliseconds() : 0;
    if (start > 0) {
        arguments.emplace_back("-ot");
        arguments.push_back(std::to_string(start));
    }
    if (request.end) {
        arguments.emplace_back("-d");
        arguments.push_back(std::to_string(request.end->milliseconds() - start));
    }
}

/// Keeps what lies inside the range. **The tool decodes whole windows**, so that its last
/// segment can run past the end that was asked for.
void clipToRange(const TranscriptionRequest& request, std::vector<TranscribedSegment>& segments) {
    if (!request.end && !request.start)
        return;
    const Timestamp from = request.start.value_or(Timestamp::origin());
    std::erase_if(segments, [&](const TranscribedSegment& segment) {
        return segment.end <= from || (request.end && segment.start >= *request.end);
    });
    if (!request.end)
        return;
    for (TranscribedSegment& segment : segments) {
        segment.end = std::min(segment.end, *request.end);
        std::erase_if(segment.words,
                      [&](const TimedWord& word) { return word.start >= *request.end; });
        for (TimedWord& word : segment.words)
            word.end = std::min(word.end, *request.end);
    }
}

} // namespace

std::optional<std::filesystem::path>
findWhisperCli(const FileSystem& files,
               std::string_view searchPath,
               const std::optional<std::filesystem::path>& configured) {
    if (configured && !configured->empty())
        return files.exists(*configured) ? configured : std::nullopt;
    return findExecutable(files, kExecutableName, searchPath);
}

WhisperCppEngine::WhisperCppEngine(std::filesystem::path executable)
    : m_executable(std::move(executable)) {}

std::expected<EngineCapabilities, TranscriptionError>
WhisperCppEngine::capabilitiesFor(const std::optional<std::filesystem::path>& model) const {
    if (!std::filesystem::exists(m_executable))
        return std::unexpected(error(TranscriptionErrorKind::EngineMissing, m_executable.string()));
    if (!model || !std::filesystem::exists(*model))
        return std::unexpected(error(TranscriptionErrorKind::ModelMissing));

    const std::string header = readHead(*model, kModelHeaderSize);
    const std::optional<bool> multilingual = readModelIsMultilingual(header);
    if (!multilingual)
        return std::unexpected(error(TranscriptionErrorKind::Failed, model->string()));

    EngineCapabilities capabilities{
        .timedWords = true, .suppressNotes = true, .sampleRate = kSampleRate, .needsModel = true};
    if (*multilingual) {
        capabilities.languageDetection = true;
        capabilities.languages.assign(kMultilingual.begin(), kMultilingual.end());
    } else {
        capabilities.languages = {"en"};
    }
    return capabilities;
}

std::expected<Transcription, TranscriptionError>
WhisperCppEngine::run(const TranscriptionRequest& request,
                      const EngineCapabilities& /*capabilities*/,
                      const TranscriptionObserver& observer) const {
    if (!std::filesystem::exists(request.audio))
        return std::unexpected(
            error(TranscriptionErrorKind::AudioUnreadable, request.audio.string()));

    const TemporaryDirectory work{"subedit-whisper"};
    if (!work.made())
        return std::unexpected(error(TranscriptionErrorKind::Failed));
    const std::filesystem::path prefix = work.path() / "out";

    const std::vector<std::string> fixed{"-m",
                                         request.model.value_or(std::filesystem::path{}).string(),
                                         "-f",
                                         request.audio.string(),
                                         "-l",
                                         request.language.value_or("auto"),
                                         "-pp",
                                         "-oj",
                                         "-ojf",
                                         "-of",
                                         prefix.string()};
    std::vector<std::string> arguments = fixed;
    // No sound written as a note, `(Sonnerie de téléphone)` or `[MUSIC PLAYING]`: the decoder is
    // forbidden the symbols they are made of, in the parentheses as in the brackets.
    if (request.suppressNotes)
        arguments.emplace_back("-sns");
    addRange(request, arguments);

    RunProgress progress{request, observer};
    std::string lastError;
    StreamObserver streamed;
    streamed.cancelled = [&observer] {
        observer.wait();
        return observer.isCancelled();
    };
    streamed.onLine = [&](const StreamedLine& line) {
        if (line.stream == StreamName::Output) {
            if (const std::optional<WhisperSegmentLine> segment = readWhisperSegmentLine(line.text))
                progress.segment(*segment);
            return;
        }
        if (const std::optional<double> done = readWhisperProgress(line.text))
            progress.windowDone(*done);
        else if (const std::optional<Timestamp> length = readWhisperAudioLength(line.text))
            progress.audioLength(*length);
        else if (line.text.starts_with("error:"))
            lastError = line.text;
    };
    progress.begin();

    const std::expected<StreamedRun, LaunchError> ran =
        runStreaming(m_executable, arguments, streamed);
    if (!ran)
        return std::unexpected(error(ran.error().kind == LaunchErrorKind::NotFound
                                         ? TranscriptionErrorKind::EngineMissing
                                         : TranscriptionErrorKind::Failed,
                                     ran.error().detail));
    // Whatever stopped it, the pieces already printed are handed back with the error.
    const auto stopped = [&progress](TranscriptionError reason) {
        reason.partial = progress.printed();
        return std::unexpected(std::move(reason));
    };
    if (ran->cancelled)
        return stopped(error(TranscriptionErrorKind::Cancelled));
    if (ran->code != 0)
        return stopped(errorOfExit(ran->code, std::move(lastError)));

    std::optional<WhisperOutput> output = readWhisperJson(readAll(work.path() / "out.json"));
    if (!output)
        return stopped(error(TranscriptionErrorKind::Failed));

    clipToRange(request, output->transcription.segments);
    progress.finish();
    return std::move(output->transcription);
}

} // namespace subedit::core
