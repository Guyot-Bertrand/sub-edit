#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/process/start_process.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/video/declared_frame_rate.hpp>

#include <charconv>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kProbeName = "ffprobe";

/// What `ffprobe` is asked, and nothing more.
///
/// `-v error` silences everything it says about itself; `-select_streams v:0`
/// picks the first video stream, and none at all in a file that has none;
/// `noprint_wrappers=1:nokey=1` leaves the value alone on its line, so that
/// what comes back is the answer rather than a report containing it.
///
/// `r_frame_rate` and not `avg_frame_rate`: the first is what the container
/// declares, the second what the stream averaged out to. A film with a few
/// dropped frames declares the rate it was timed at and averages something
/// close to it, and the rate it was timed at is the one that converts
/// positions.
[[nodiscard]] std::vector<std::string> questionAbout(const std::filesystem::path& video) {
    return {"-v",
            "error",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=r_frame_rate",
            "-of",
            "default=noprint_wrappers=1:nokey=1",
            video.string()};
}

/// Reads a whole number, and refuses anything that is not entirely one.
[[nodiscard]] std::optional<std::int64_t> wholeNumber(std::string_view text) {
    std::int64_t value = 0;
    const std::from_chars_result read =
        std::from_chars(text.data(), text.data() + text.size(), value);

    // `read.ptr` short of the end means the field held something else after a
    // number — `24000/1001 fps`, or a space. A number followed by anything is
    // not a number.
    if (read.ec != std::errc{} || read.ptr != text.data() + text.size())
        return std::nullopt;
    return value;
}

} // namespace

std::optional<FrameRate> parseDeclaredFrameRate(std::string_view text) {
    // The answer is the first line: the question asks about one stream, and
    // reading past it would be reading whatever else happened to be written.
    const std::string_view line = text.substr(0, text.find('\n'));

    const std::size_t slash = line.find('/');
    if (slash == std::string_view::npos)
        return std::nullopt;

    const std::optional<std::int64_t> frames = wholeNumber(line.substr(0, slash));
    const std::optional<std::int64_t> seconds = wholeNumber(line.substr(slash + 1));
    if (!frames.has_value() || !seconds.has_value())
        return std::nullopt;

    // `0/0` — which is what a stream with no rate to declare looks like — is
    // refused here by `FrameRate` itself, which knows that a rate is strictly
    // positive.
    return FrameRate::create(*frames, *seconds);
}

std::optional<FrameRate> readDeclaredFrameRate(const FileSystem& files,
                                               std::string_view searchPath,
                                               const std::filesystem::path& video) {
    const std::optional<std::filesystem::path> probe =
        findExecutable(files, kProbeName, searchPath);
    if (!probe.has_value())
        return std::nullopt;

    const std::expected<ProgramOutput, LaunchError> answered =
        runAndCapture(*probe, questionAbout(video));

    // A launch that was refused, or a program that ended badly — a file that is
    // not there is the ordinary case of the second. Neither is worth a word to
    // the user: what they lose is a proposal.
    if (!answered.has_value() || answered->code != 0)
        return std::nullopt;

    return parseDeclaredFrameRate(answered->output);
}

} // namespace subedit::core
