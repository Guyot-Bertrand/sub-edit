#include "application.hpp"

#include <subedit/cli/conversion.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/frame_rate_grammar.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/cli/transforming.hpp>
#include <subedit/cli/verbosity.hpp>
#include <subedit/core/format/real_file_system.hpp>
#include <subedit/core/version.hpp>

#include <CLI/CLI.hpp>
#include <expected>
#include <iostream>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// What `inspect` was asked for.
struct InspectOptions {
    std::vector<std::string> files;
    std::string reading = "breaks";
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
    bool bom = false;
    bool noBom = false;
    std::string output;
    std::string outputDir;
    bool inPlace = false;
};

CLI::App* describeInspect(CLI::App& app, InspectOptions& options) {
    CLI::App* inspect = app.add_subcommand("inspect", "Report what a subtitle file is made of");
    inspect->add_option("files", options.files, "Subtitle files to report on")->required();

    // Both readings of disorder are offered rather than one chosen: this phase
    // is a harness, and comparing them on real files will settle the question
    // better than arguing it without data. Phase 5 inherits the answer and this
    // option goes — see docs/specs/03-cli.md.
    inspect
        ->add_option(
            "--order-report", options.reading, "Which lines to name when the file is out of order")
        ->check(CLI::IsMember({"breaks", "late"}))
        ->capture_default_str();
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
    convert->add_flag("--bom", options.bom, "Write a byte order mark");
    convert->add_flag("--no-bom", options.noBom, "Write no byte order mark");

    convert->add_option("--output", options.output, "File to write, for a single input");
    convert->add_option("--output-dir", options.outputDir, "Directory to write into");
    convert->add_flag("--in-place", options.inPlace, "Write back over the inputs");
    return convert;
}

/// What `shift` was asked for.
struct ShiftOptions {
    std::vector<std::string> files;
    std::string by;
    std::string output;
    std::string outputDir;
    bool inPlace = false;
};

CLI::App* describeShift(CLI::App& app, ShiftOptions& options) {
    CLI::App* shift =
        app.add_subcommand("shift", "Move every position of a file by a fixed amount");
    shift->add_option("files", options.files, "Subtitle files to shift")->required();
    shift->add_option("--by", options.by, "Amount to move by: 2.999, -7.001, or 00:00:07.001")
        ->required();

    shift->add_option("--output", options.output, "File to write, for a single input");
    shift->add_option("--output-dir", options.outputDir, "Directory to write into");
    shift->add_flag("--in-place", options.inPlace, "Write back over the inputs");
    return shift;
}

ExitCode runShift(const ShiftOptions& options, core::FileSystem& files, const Reporter& reporter) {
    const std::expected<core::Duration, std::string> by = parseTime(options.by);
    if (!by) {
        std::cerr << by.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<Destination, std::string> destination =
        Destination::from(options.output, options.outputDir, options.inPlace, options.files.size());
    if (!destination) {
        std::cerr << destination.error() << '\n';
        return ExitCode::Usage;
    }

    return shiftAll(files, options.files, *by, *destination, reporter);
}

/// What `transform` was asked for.
struct TransformOptions {
    std::vector<std::string> files;
    std::string first;
    std::string last;
    std::string output;
    std::string outputDir;
    bool inPlace = false;
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

    transform->add_option("--output", options.output, "File to write, for a single input");
    transform->add_option("--output-dir", options.outputDir, "Directory to write into");
    transform->add_flag("--in-place", options.inPlace, "Write back over the inputs");
    return transform;
}

ExitCode
runTransform(const TransformOptions& options, core::FileSystem& files, const Reporter& reporter) {
    const std::expected<Reference, std::string> first = parseReference(options.first);
    if (!first) {
        std::cerr << first.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<Reference, std::string> last = parseReference(options.last);
    if (!last) {
        std::cerr << last.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<Transform, std::string> transform = Transform::between(*first, *last);
    if (!transform) {
        std::cerr << transform.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<Destination, std::string> destination =
        Destination::from(options.output, options.outputDir, options.inPlace, options.files.size());
    if (!destination) {
        std::cerr << destination.error() << '\n';
        return ExitCode::Usage;
    }

    return transformAll(files, options.files, *transform, *destination, reporter);
}

/// What `framerate` was asked for.
struct FrameRateOptions {
    std::vector<std::string> files;
    std::string from;
    std::string to;
    std::string output;
    std::string outputDir;
    bool inPlace = false;
};

CLI::App* describeFrameRate(CLI::App& app, FrameRateOptions& options) {
    CLI::App* framerate =
        app.add_subcommand("framerate", "Re-time a file mastered at one frame rate for another");
    framerate->add_option("files", options.files, "Subtitle files to re-time")->required();
    framerate->add_option("--from", options.from, "Frame rate the file is timed at: 25, 23.976")
        ->required();
    framerate->add_option("--to", options.to, "Frame rate to time it for: 24, 29.97")->required();

    framerate->add_option("--output", options.output, "File to write, for a single input");
    framerate->add_option("--output-dir", options.outputDir, "Directory to write into");
    framerate->add_flag("--in-place", options.inPlace, "Write back over the inputs");
    return framerate;
}

ExitCode
runFrameRate(const FrameRateOptions& options, core::FileSystem& files, const Reporter& reporter) {
    const std::expected<core::FrameRate, std::string> from = parseFrameRate(options.from);
    if (!from) {
        std::cerr << from.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<core::FrameRate, std::string> to = parseFrameRate(options.to);
    if (!to) {
        std::cerr << to.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<Destination, std::string> destination =
        Destination::from(options.output, options.outputDir, options.inPlace, options.files.size());
    if (!destination) {
        std::cerr << destination.error() << '\n';
        return ExitCode::Usage;
    }

    return convertFrameRateAll(files, options.files, *from, *to, *destination, reporter);
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
    if (options.bom) {
        shape.bom = core::Utf8Bom::Present;
    }
    if (options.noBom) {
        shape.bom = core::Utf8Bom::Absent;
    }
    return shape;
}

ExitCode
runConvert(const ConvertOptions& options, core::FileSystem& files, const Reporter& reporter) {
    const core::SubtitleFormat target =
        options.target == "vtt" ? core::SubtitleFormat::WebVtt : core::SubtitleFormat::SubRip;

    // Refused rather than obeyed: in place there is no second name to carry the
    // new format, and the file would be left under an extension its content no
    // longer justifies.
    if (options.inPlace && wouldMisname(options.files, target)) {
        std::cerr << "--in-place cannot change the format: the file would keep a name "
                     "its content no longer matches\n";
        return ExitCode::Usage;
    }

    const std::expected<WriteShape, std::string> shape = shapeOf(options);
    if (!shape) {
        std::cerr << shape.error() << '\n';
        return ExitCode::Usage;
    }

    const std::expected<Destination, std::string> destination =
        Destination::from(options.output, options.outputDir, options.inPlace, options.files.size());
    if (!destination) {
        std::cerr << destination.error() << '\n';
        return ExitCode::Usage;
    }

    return convertAll(files, options.files, target, *shape, *destination, reporter);
}

ExitCode
runInspect(const InspectOptions& options, const core::FileSystem& files, const Reporter& reporter) {
    // One of the two: CLI11 refused anything else before we got here, which is
    // what makes this a lookup rather than a decision.
    const core::OrderReport order =
        options.reading == "late" ? core::OrderReport::Late : core::OrderReport::Breaks;
    return inspectAll(files, options.files, std::cout, reporter, order);
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
        std::cerr << level.error() << '\n';
        return ExitCode::Usage;
    }

    // No subcommand: show what the tool can be asked to do. On standard
    // output, because here the help is the result rather than a complaint.
    if (app.get_subcommands().empty()) {
        std::cout << app.help();
        return ExitCode::Success;
    }

    core::RealFileSystem files;
    const Reporter reporter{std::cerr, *level};

    if (inspect->parsed()) {
        return runInspect(inspectOptions, files, reporter);
    }
    if (convert->parsed()) {
        return runConvert(convertOptions, files, reporter);
    }
    if (shift->parsed()) {
        return runShift(shiftOptions, files, reporter);
    }
    if (transform->parsed()) {
        return runTransform(transformOptions, files, reporter);
    }
    if (framerate->parsed()) {
        return runFrameRate(frameRateOptions, files, reporter);
    }
    return ExitCode::Success;
}

} // namespace subedit::cli
