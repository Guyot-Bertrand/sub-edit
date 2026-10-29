#include "sync.hpp"

#include <subedit/cli/shared_settings.hpp>
#include <subedit/cli/syncing.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/transcription/default_engines.hpp>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

constexpr int kSmallestMatch = 1;
constexpr int kLargestMatch = 100;

/// What `sync` was asked for, as CLI11 filled it in.
struct SyncCommandOptions {
    std::vector<std::string> files;
    bool recursive = false;
    SyncOptions request;
    int audioTrack = -1;
    int minimumMatch = 0;
    std::int64_t doubtGap = -1;
    DestinationOptions destination;
};

CLI::App* describeSync(CLI::App& app, std::string_view name, SyncCommandOptions& options) {
    CLI::App* sync = app.add_subcommand(
        std::string{name},
        std::string{core::translate("Re-time the subtitles of a file on the audio of a film")});
    sync->add_option(
            "files", options.files, std::string{core::translate("Subtitle files to re-time")})
        ->required();
    describeRecursive(sync, options.recursive);
    sync->add_option(
            "--video", options.request.video, std::string{core::translate("Film to listen to")})
        ->option_text("FILE");
    sync->add_option("--audio",
                     options.request.audio,
                     std::string{core::translate("Audio file to listen to, instead of a film")})
        ->option_text("FILE");
    sync->add_option("--audio-track",
                     options.audioTrack,
                     std::string{core::translate(
                         "Audio track of a film, counted from 0; the first by default")})
        ->check(CLI::NonNegativeNumber)
        ->option_text("N");
    sync->add_option("--start",
                     options.request.start,
                     std::string{core::translate(
                         "Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default")})
        ->option_text("TIME");
    sync->add_option("--end",
                     options.request.end,
                     std::string{core::translate(
                         "Where to stop, as 2:00, 1:02:03 or seconds; the end by default")})
        ->option_text("TIME");
    sync->add_option("--language",
                     options.request.language,
                     std::string{core::translate(
                         "Language of the speech, two letters: fr, en, de; detected from the first "
                         "30 seconds by default")})
        ->option_text("CODE");
    sync->add_option(
            "--model",
            options.request.model,
            std::string{core::translate("Model file; the transcription.model setting by default")})
        ->option_text("FILE");
    sync->add_option("--vad-model",
                     options.request.vadModel,
                     std::string{core::translate("Voice-activity model that skips silences; the "
                                                 "transcription.vad-model setting by default")})
        ->option_text("FILE");
    sync->add_option(
            "--engine",
            options.request.engine,
            std::string{core::translate("Engine that listens; the transcription.engine setting by "
                                        "default")})
        ->option_text("NAME");
    sync->add_option("--keep-audio",
                     options.request.keepAudio,
                     std::string{core::translate("Keep the extracted audio in this file")})
        ->option_text("FILE");
    sync->add_option("--min-match",
                     options.minimumMatch,
                     std::string{core::translate(
                         "Share of a subtitle's words that must be heard, in percent; the "
                         "alignment.min-match setting by default")})
        ->check(CLI::Range(kSmallestMatch, kLargestMatch))
        ->option_text("PERCENT");
    sync->add_option("--doubt-gap",
                     options.doubtGap,
                     std::string{core::translate(
                         "How far a subtitle may move from its neighbours' shift before it is "
                         "flagged, in milliseconds; the alignment.doubt-gap setting by default")})
        ->check(CLI::NonNegativeNumber)
        ->option_text("MS");
    sync->add_option("--report",
                     options.request.report,
                     std::string{core::translate("Write the subtitles to check to this file")})
        ->option_text("FILE");

    describeDestination(sync, options.destination);
    return sync;
}

ExitCode runSync(SyncCommandOptions& options,
                 core::FileSystem& files,
                 const std::optional<core::Encoding>& reading,
                 const Reporter& reporter,
                 const SharedSettings& shared) {
    if (const std::optional<std::string> missing = missingNamedFile(shared))
        return refuse(*missing);

    if (options.audioTrack >= 0)
        options.request.audioTrack = options.audioTrack;
    if (options.minimumMatch > 0)
        options.request.minimumMatchPercent = options.minimumMatch;
    if (options.doubtGap >= 0)
        options.request.doubtGapMilliseconds = options.doubtGap;

    const std::expected<PreparedWriting, std::string> prepared =
        prepareWriting(files,
                       reporter,
                       {.files = options.files, .recursive = options.recursive},
                       options.destination);
    if (!prepared)
        return refuse(prepared.error());

    const char* const path = std::getenv("PATH");
    const std::string searchPath = path != nullptr ? path : "";
    const core::TranscriptionSettings& transcription = shared.values.transcription;
    const core::EngineRegistry engines = core::makeDefaultEngines(
        files,
        searchPath,
        transcription.executable.empty() ? std::nullopt : std::optional{transcription.executable});

    return syncAll(files,
                   engines,
                   searchPath,
                   prepared->inputs.paths,
                   reading,
                   options.request,
                   transcription,
                   shared.values.alignment,
                   prepared->destination,
                   reporter);
}

} // namespace

Declared declareSync(CLI::App& app, std::string_view name) {
    return declareWith<SyncCommandOptions>(app, name, describeSync, runSync);
}

} // namespace subedit::cli
