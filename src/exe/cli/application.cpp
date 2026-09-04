#include "application.hpp"

#include <subedit/cli/aligning.hpp>
#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/frame_rate_grammar.hpp>
#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/cli/transforming.hpp>
#include <subedit/cli/verbosity.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/version.hpp>

#include <CLI/CLI.hpp>
#include <cstddef>
#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::cli {

namespace {

/// Where a subcommand writes, as the three options that say it.
///
/// A type of its own rather than three fields repeated in four structs: they
/// always travel together, they are always declared the same way, and they are
/// always turned into a `Destination` by the same call. A fifth subcommand that
/// writes now costs none of the three.
struct DestinationOptions {
    std::string output;
    std::string outputDir;
    bool inPlace = false;
};

/// Declares the three options on `command`.
///
/// Called last by every `describe…`, so that a subcommand lists its own options
/// before those it shares — the order the help shows and the manual quotes.
void describeDestination(CLI::App* command, DestinationOptions& options) {
    command->add_option("--output", options.output, "File to write, for a single input");
    command->add_option("--output-dir", options.outputDir, "Directory to write into");
    command->add_flag("--in-place", options.inPlace, "Write back over the inputs");
}

/// The destination those options describe, or why they cannot be honoured.
std::expected<Destination, std::string> destinationOf(const DestinationOptions& options,
                                                      std::size_t inputCount) {
    return Destination::from(options.output, options.outputDir, options.inPlace, inputCount);
}

/// Writes a refusal and gives the code that goes with it.
///
/// Every value this file reads can be refused, and each refusal was written out
/// in four lines, nine times over. Named once, a usage error stops being a shape
/// one has to recognise.
ExitCode refuse(std::string_view why) {
    std::cerr << why << '\n';
    return ExitCode::Usage;
}

/// What `inspect` was asked for.
struct InspectOptions {
    std::vector<std::string> files;
};

/// What `convert` was asked for, verbatim.
///
/// Kept as the strings CLI11 filled in rather than as the types the core wants.
/// Translating them belongs to the run functions below: doing it here would
/// mean deciding before the command line has been read whole.
struct ConvertOptions {
    std::vector<std::string> files;
    std::string target;
    std::string lineEndings;
    std::string encoding;
    bool bom = false;
    bool noBom = false;
    DestinationOptions destination;
};

CLI::App* describeInspect(CLI::App& app, InspectOptions& options) {
    CLI::App* inspect = app.add_subcommand("inspect", "Report what a subtitle file is made of");
    inspect->add_option("files", options.files, "Subtitle files to report on")->required();

    // `--order-report` lived here, offering both readings of disorder while
    // real files settled the question. They did not — none of the corpus is out
    // of order — so it was settled by reasoning, and the option is gone: the
    // report names what breaks the order, because that is the subtitle there is
    // something to do about.
    return inspect;
}

CLI::App* describeConvert(CLI::App& app, ConvertOptions& options) {
    CLI::App* convert =
        app.add_subcommand("convert", "Write a subtitle file out in another format or shape");
    convert->add_option("files", options.files, "Subtitle files to convert")->required();
    convert->add_option("--to", options.target, "Format to write")
        ->required()
        ->check(CLI::IsMember({"srt", "vtt"}));

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
    convert->add_flag("--bom", options.bom, "Write a byte order mark");
    convert->add_flag("--no-bom", options.noBom, "Write no byte order mark");

    describeDestination(convert, options.destination);
    return convert;
}

/// What `shift` was asked for.
///
/// The amount is given by `--by`, or measured by `--to-grid`. Exactly one of
/// the two, and neither is `required()` on its own: CLI11 would then insist on
/// both, and the check below can say why in a sentence rather than in the
/// usage.
struct ShiftOptions {
    std::vector<std::string> files;
    std::string by;
    bool toGrid = false;
    DestinationOptions destination;
};

CLI::App* describeShift(CLI::App& app, ShiftOptions& options) {
    CLI::App* shift =
        app.add_subcommand("shift", "Move every position of a file by a fixed amount");
    shift->add_option("files", options.files, "Subtitle files to shift")->required();
    shift->add_option("--by", options.by, "Amount to move by: 2.999, -7.001, or 00:00:07.001");
    shift->add_flag("--to-grid",
                    options.toGrid,
                    "Move by the amount that puts the positions back on their frame grid");

    describeDestination(shift, options.destination);
    return shift;
}

ExitCode runShift(const ShiftOptions& options,
                  core::FileSystem& files,
                  const std::optional<core::Encoding>& reading,
                  const Reporter& reporter) {
    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    if (options.toGrid && !options.by.empty()) {
        return refuse("--by and --to-grid both say by how much to move; give one or the other");
    }

    // Measured rather than given, and file by file: two files shifted off the
    // same grid by different amounts come back by different amounts.
    if (options.toGrid) {
        return shiftOntoGridAll(files, options.files, reading, *destination, reporter);
    }

    if (options.by.empty()) {
        return refuse("shift needs --by, or --to-grid to work the amount out from the positions");
    }

    const std::expected<core::Duration, std::string> by = parseTime(options.by);
    if (!by) {
        return refuse(by.error());
    }

    return shiftAll(files, options.files, reading, *by, *destination, reporter);
}

/// What `snap` was asked for.
struct SnapOptions {
    std::vector<std::string> files;
    std::string rate;
    DestinationOptions destination;
};

CLI::App* describeSnap(CLI::App& app, SnapOptions& options) {
    CLI::App* snap = app.add_subcommand(
        "snap", "Move every position onto the nearest frame of a frame rate (see framerate)");
    snap->add_option("files", options.files, "Subtitle files to align")->required();
    snap->add_option("--rate", options.rate, "Frame rate to align on: 25, 23.976")->required();

    describeDestination(snap, options.destination);
    return snap;
}

ExitCode runSnap(const SnapOptions& options,
                 core::FileSystem& files,
                 const std::optional<core::Encoding>& reading,
                 const Reporter& reporter) {
    const std::expected<core::FrameRate, std::string> rate = parseFrameRate(options.rate);
    if (!rate) {
        return refuse(rate.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return alignAll(files, options.files, reading, *rate, *destination, reporter);
}

/// What `hearing-impaired` was asked for.
///
/// Nothing but files and a destination: the rule is decided, not configurable,
/// and it applies to the whole of the main text. What a phase 12 will make
/// adjustable is written in its spec, not guessed at here.
struct HearingImpairedOptions {
    std::vector<std::string> files;
    DestinationOptions destination;
};

CLI::App* describeHearingImpaired(CLI::App& app, HearingImpairedOptions& options) {
    CLI::App* hearing = app.add_subcommand(
        "hearing-impaired", "Remove the sounds described between brackets or parentheses");
    hearing->add_option("files", options.files, "Subtitle files to clean")->required();

    describeDestination(hearing, options.destination);
    return hearing;
}

ExitCode runHearingImpaired(const HearingImpairedOptions& options,
                            core::FileSystem& files,
                            const std::optional<core::Encoding>& reading,
                            const Reporter& reporter) {
    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return removeHearingImpairedIn(files, options.files, reading, *destination, reporter);
}

/// What `transform` was asked for.
struct TransformOptions {
    std::vector<std::string> files;
    std::string first;
    std::string last;
    DestinationOptions destination;
};

CLI::App* describeTransform(CLI::App& app, TransformOptions& options) {
    CLI::App* transform =
        app.add_subcommand("transform", "Correct every position from two points known to be right");
    transform->add_option("files", options.files, "Subtitle files to transform")->required();
    transform
        ->add_option(
            "--first", options.first, "Earlier reference, as <index>=<time>: 1=00:00:01.000")
        ->required();
    transform
        ->add_option("--last", options.last, "Later reference, as <index>=<time>: 3=00:00:10.000")
        ->required();

    describeDestination(transform, options.destination);
    return transform;
}

ExitCode runTransform(const TransformOptions& options,
                      core::FileSystem& files,
                      const std::optional<core::Encoding>& reading,
                      const Reporter& reporter) {
    const std::expected<Reference, std::string> first = parseReference(options.first);
    if (!first) {
        return refuse(first.error());
    }

    const std::expected<Reference, std::string> last = parseReference(options.last);
    if (!last) {
        return refuse(last.error());
    }

    const std::expected<Transform, std::string> transform = Transform::between(*first, *last);
    if (!transform) {
        return refuse(transform.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return transformAll(files, options.files, reading, *transform, *destination, reporter);
}

/// What `framerate` was asked for.
struct FrameRateOptions {
    std::vector<std::string> files;
    std::string from;
    std::string to;
    DestinationOptions destination;
};

CLI::App* describeFrameRate(CLI::App& app, FrameRateOptions& options) {
    CLI::App* framerate =
        app.add_subcommand("framerate", "Re-time a file mastered at one frame rate for another");
    framerate->add_option("files", options.files, "Subtitle files to re-time")->required();
    framerate->add_option("--from", options.from, "Frame rate the file is timed at: 25, 23.976")
        ->required();
    framerate->add_option("--to", options.to, "Frame rate to time it for: 24, 29.97")->required();

    describeDestination(framerate, options.destination);
    return framerate;
}

ExitCode runFrameRate(const FrameRateOptions& options,
                      core::FileSystem& files,
                      const std::optional<core::Encoding>& reading,
                      const Reporter& reporter) {
    const std::expected<core::FrameRate, std::string> from = parseFrameRate(options.from);
    if (!from) {
        return refuse(from.error());
    }

    const std::expected<core::FrameRate, std::string> to = parseFrameRate(options.to);
    if (!to) {
        return refuse(to.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return convertFrameRateAll(files, options.files, reading, *from, *to, *destination, reporter);
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
    const core::SubtitleFormat target =
        options.target == "vtt" ? core::SubtitleFormat::WebVtt : core::SubtitleFormat::SubRip;

    // Refused rather than obeyed: in place there is no second name to carry the
    // new format, and the file would be left under an extension its content no
    // longer justifies.
    if (options.destination.inPlace && wouldMisname(options.files, target)) {
        return refuse("--in-place cannot change the format: the file would keep a name "
                      "its content no longer matches");
    }

    const std::expected<WriteShape, std::string> shape = shapeOf(options);
    if (!shape) {
        return refuse(shape.error());
    }

    const std::expected<Destination, std::string> destination =
        destinationOf(options.destination, options.files.size());
    if (!destination) {
        return refuse(destination.error());
    }

    return convertAll(files, options.files, reading, target, *shape, *destination, reporter);
}

ExitCode runInspect(const InspectOptions& options,
                    const core::FileSystem& files,
                    const std::optional<core::Encoding>& reading,
                    const Reporter& reporter) {
    return inspectAll(files, options.files, reading, std::cout, reporter);
}

} // namespace

ExitCode run(int argc, char** argv) {
    CLI::App app{"Read, inspect and retime subtitle files."};

    // The program name, and not argv[0]: otherwise the usage line shows the
    // build path it was started from, which `make manual` would then copy into
    // the manual.
    app.name("subedit-cli");
    app.set_version_flag("--version", "subedit " + core::versionString());

    // How many times -v was given, not the level itself: CLI11 zeroes a bound
    // counter when the flag is absent, which would silently turn the default
    // level into silence. levelFrom() owns that decision.
    int verboseCount = 0;
    app.add_flag("-v", verboseCount, "Say more: -v is the default, -vv details, -vvv debugs")
        ->option_text(" ");

    bool quiet = false;
    app.add_flag("-q,--quiet", quiet, "Say nothing but errors");

    // **Global, because every subcommand reads.** A file whose encoding is
    // guessed wrong is guessed wrong whatever is being done to it, and an
    // option that only `inspect` carried would leave a shift no way to be told.
    std::string reading;
    app.add_option("--encoding", reading, "Encoding to read the files in; detected by default")
        ->option_text("NAME");

    InspectOptions inspectOptions;
    const CLI::App* inspect = describeInspect(app, inspectOptions);
    ConvertOptions convertOptions;
    const CLI::App* convert = describeConvert(app, convertOptions);
    ShiftOptions shiftOptions;
    const CLI::App* shift = describeShift(app, shiftOptions);
    TransformOptions transformOptions;
    const CLI::App* transform = describeTransform(app, transformOptions);
    FrameRateOptions frameRateOptions;
    const CLI::App* framerate = describeFrameRate(app, frameRateOptions);
    SnapOptions snapOptions;
    const CLI::App* snap = describeSnap(app, snapOptions);
    HearingImpairedOptions hearingOptions;
    const CLI::App* hearing = describeHearingImpaired(app, hearingOptions);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& error) {
        // CLI11 has its own exit codes; ours are the four of the manual. Help
        // and version leave through here because CLI11 signals them as
        // exceptions, and they keep code 0.
        return app.exit(error) == 0 ? ExitCode::Success : ExitCode::Usage;
    }

    const std::expected<int, std::string> level = levelFrom(quiet, verboseCount);
    if (!level) {
        return refuse(level.error());
    }

    // No subcommand: show what the tool can be asked to do. On standard
    // output, because here the help is the result rather than a complaint.
    if (app.get_subcommands().empty()) {
        std::cout << app.help();
        return ExitCode::Success;
    }

    // Read once and for every file: an encoding that names nothing is a usage
    // error, answered while the user is still being asked something.
    std::optional<core::Encoding> encoding;
    if (!reading.empty()) {
        const std::expected<core::Encoding, std::string> named = encodingNamed(reading);
        if (!named) {
            return refuse(named.error());
        }
        encoding = *named;
    }

    core::RealFileSystem files;
    const Reporter reporter{std::cerr, *level};

    if (inspect->parsed()) {
        return runInspect(inspectOptions, files, encoding, reporter);
    }
    if (convert->parsed()) {
        return runConvert(convertOptions, files, encoding, reporter);
    }
    if (shift->parsed()) {
        return runShift(shiftOptions, files, encoding, reporter);
    }
    if (transform->parsed()) {
        return runTransform(transformOptions, files, encoding, reporter);
    }
    if (framerate->parsed()) {
        return runFrameRate(frameRateOptions, files, encoding, reporter);
    }
    if (snap->parsed()) {
        return runSnap(snapOptions, files, encoding, reporter);
    }
    if (hearing->parsed()) {
        return runHearingImpaired(hearingOptions, files, encoding, reporter);
    }
    return ExitCode::Success;
}

} // namespace subedit::cli
