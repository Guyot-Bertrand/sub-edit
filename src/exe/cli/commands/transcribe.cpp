#include "transcribe.hpp"

#include <subedit/cli/shared_settings.hpp>
#include <subedit/cli/transcribing.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/transcription/default_engines.hpp>
#include <subedit/core/wording/formats.hpp>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `transcribe` was asked for, as CLI11 filled it in.
struct TranscribeCommandOptions {
    TranscribeOptions request;
    int audioTrack = -1;
};

/// The `--format` values: the short name of every format, in the core's order.
[[nodiscard]] std::vector<std::string> formatOptionNames() {
    std::vector<std::string> names;
    names.reserve(core::kSubtitleFormats.size());
    for (const core::SubtitleFormat format : core::kSubtitleFormats)
        names.emplace_back(core::optionNameOf(format));
    return names;
}

CLI::App*
describeTranscribe(CLI::App& app, std::string_view name, TranscribeCommandOptions& options) {
    CLI::App* transcribe = app.add_subcommand(
        std::string{name},
        std::string{core::translate("Write the speech of a video or an audio file as subtitles")});
    transcribe
        ->add_option("input",
                     options.request.input,
                     std::string{core::translate("Video or audio file to transcribe")})
        ->required();
    transcribe
        ->add_option("--output",
                     options.request.output,
                     std::string{core::translate("Subtitle file to write")})
        ->required()
        ->option_text("FILE");
    transcribe
        ->add_option("--to",
                     options.request.format,
                     std::string{core::translate(
                         "Format to write; the one the extension of --output says by default")})
        ->check(CLI::IsMember(formatOptionNames()));
    transcribe
        ->add_option("--audio-track",
                     options.audioTrack,
                     std::string{core::translate(
                         "Audio track of a video, counted from 0; the first by default")})
        ->check(CLI::NonNegativeNumber)
        ->option_text("N");
    transcribe
        ->add_option("--start",
                     options.request.start,
                     std::string{core::translate(
                         "Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default")})
        ->option_text("TIME");
    transcribe
        ->add_option("--end",
                     options.request.end,
                     std::string{core::translate(
                         "Where to stop, as 2:00, 1:02:03 or seconds; the end by default")})
        ->option_text("TIME");
    transcribe
        ->add_option(
            "--language",
            options.request.language,
            std::string{core::translate("Language of the speech: fr, en; detected by default")})
        ->option_text("CODE");
    transcribe
        ->add_option(
            "--model",
            options.request.model,
            std::string{core::translate("Model file; the transcription.model setting by default")})
        ->option_text("FILE");
    transcribe
        ->add_option(
            "--engine",
            options.request.engine,
            std::string{core::translate("Engine; the transcription.engine setting by default")})
        ->option_text("NAME");
    transcribe
        ->add_option("--keep-audio",
                     options.request.keepAudio,
                     std::string{core::translate("Keep the extracted audio in this file")})
        ->option_text("FILE");
    transcribe->add_flag("--raw",
                         options.request.raw,
                         std::string{core::translate("Write the segments as the engine cut them")});
    return transcribe;
}

ExitCode runTranscribe(TranscribeCommandOptions& options,
                       core::FileSystem& files,
                       const std::optional<core::Encoding>& /*reading*/,
                       const Reporter& reporter,
                       const SharedSettings& shared) {
    if (const std::optional<std::string> missing = missingNamedFile(shared))
        return refuse(*missing);

    if (options.audioTrack >= 0)
        options.request.audioTrack = options.audioTrack;

    const char* const path = std::getenv("PATH");
    const std::string searchPath = path != nullptr ? path : "";
    const core::TranscriptionSettings& settings = shared.values.transcription;
    const core::EngineRegistry engines = core::makeDefaultEngines(
        files,
        searchPath,
        settings.executable.empty() ? std::nullopt : std::optional{settings.executable});

    return transcribeFile(files, engines, searchPath, options.request, settings, reporter);
}

} // namespace

Declared declareTranscribe(CLI::App& app, std::string_view name) {
    return declareWith<TranscribeCommandOptions>(app, name, describeTranscribe, runTranscribe);
}

} // namespace subedit::cli
