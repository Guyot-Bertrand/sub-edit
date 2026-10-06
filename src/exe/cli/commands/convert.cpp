#include "convert.hpp"

#include <subedit/cli/conversion.hpp>
#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/core/wording/formats.hpp>

#include <CLI/CLI.hpp>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `convert` was asked for, verbatim.
///
/// Kept as the strings CLI11 filled in rather than as the types the core wants.
/// Translating them belongs to the run functions below: doing it here would
/// mean deciding before the command line has been read whole.
struct ConvertOptions {
    std::vector<std::string> files;
    bool recursive = false;
    std::string target;
    std::string frameRate;
    std::string lineEndings;
    std::string encoding;
    bool bom = false;
    bool noBom = false;
    DestinationOptions destination;
};

CLI::App* describeConvert(CLI::App& app, std::string_view name, ConvertOptions& options) {
    CLI::App* convert = app.add_subcommand(std::string{name},
                                           "Write a subtitle file out in another format or shape");
    convert->add_option("files", options.files, "Subtitle files to convert")->required();
    describeRecursive(convert, options.recursive);
    convert->add_option("--to", options.target, "Format to write")
        ->required()
        // **One value per format that can be written**, and the list grew by
        // one with each format of phase 9 until it held all nine. It is written
        // here rather than derived, so that offering a format the library
        // cannot write is a line someone had to add.
        ->check(CLI::IsMember(
            {"srt", "vtt", "subviewer2", "ssa", "ass", "mpl2", "microdvd", "tmplayer", "lrc"}));

    // Left empty on purpose: empty means "as the source had it", and the model
    // of phase 1 kept both so that a conversion would not throw them away.
    convert
        ->add_option(
            "--line-endings", options.lineEndings, "Line endings to write; the source's by default")
        ->check(CLI::IsMember({"unix", "windows", "mac"}));
    convert
        ->add_option(
            "--to-encoding", options.encoding, "Encoding to write; the source's by default")
        ->option_text("NAME");
    // **One option for both directions**, because there is one rate. A file
    // counted in frames is read at it, and a file written in frames is counted
    // at it; naming them apart would invite giving two and mean nothing.
    convert
        ->add_option(
            "--frame-rate", options.frameRate, "Frame rate of a file counted in frames: 25, 23.976")
        ->option_text("RATE");
    convert->add_flag("--bom", options.bom, "Write a byte order mark");
    convert->add_flag("--no-bom", options.noBom, "Write no byte order mark");

    describeDestination(convert, options.destination);
    return convert;
}

core::Newline newlineNamed(const std::string& name) {
    if (name == "windows") {
        return core::Newline::CrLf;
    }
    return name == "mac" ? core::Newline::Cr : core::Newline::Lf;
}

std::expected<WriteShape, std::string> shapeOf(const ConvertOptions& options) {
    if (options.bom && options.noBom) {
        return std::unexpected{
            std::string{"--bom and --no-bom ask for opposite things; give one or the other"}};
    }

    WriteShape shape;
    if (!options.lineEndings.empty()) {
        shape.newline = newlineNamed(options.lineEndings);
    }
    if (!options.encoding.empty()) {
        const std::expected<core::Encoding, std::string> named = encodingNamed(options.encoding);
        if (!named) {
            return std::unexpected(named.error());
        }
        shape.encoding = *named;
    }
    if (options.bom) {
        shape.bom = core::ByteOrderMark::Present;
    }
    if (options.noBom) {
        shape.bom = core::ByteOrderMark::Absent;
    }
    return shape;
}

ExitCode runConvert(const ConvertOptions& options,
                    core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter) {
    // Checked by the option itself, so the fallback is unreachable; SubRip is
    // what the tool wrote before there was anything to choose.
    const core::SubtitleFormat target =
        core::formatNamed(options.target).value_or(core::SubtitleFormat::SubRip);

    const std::expected<core::ReadingChoices, std::string> choices =
        readingWith(reading, options.frameRate);
    if (!choices) {
        return refuse(choices.error());
    }

    const std::expected<PreparedWriting, std::string> prepared =
        prepareWriting(files,
                       reporter,
                       {.files = options.files, .recursive = options.recursive},
                       options.destination);
    if (!prepared) {
        return refuse(prepared.error());
    }

    // Refused rather than obeyed: in place there is no second name to carry the
    // new format, and the file would be left under an extension its content no
    // longer justifies.
    if (options.destination.inPlace && wouldMisname(prepared->inputs.paths, target)) {
        return refuse("--in-place cannot change the format: the file would keep a name "
                      "its content no longer matches");
    }

    const std::expected<WriteShape, std::string> shape = shapeOf(options);
    if (!shape) {
        return refuse(shape.error());
    }

    return convertAll(
        files, prepared->inputs.paths, *choices, target, *shape, prepared->destination, reporter);
}

} // namespace

Declared declareConvert(CLI::App& app, std::string_view name) {
    return declareWith<ConvertOptions>(app, name, describeConvert, runConvert);
}

} // namespace subedit::cli
