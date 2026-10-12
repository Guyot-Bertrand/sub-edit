#include "application.hpp"

#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/cli/encoding_listing.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shared_settings.hpp>
#include <subedit/cli/verbosity.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/version.hpp>
#include <subedit/platform/locations.hpp>

#include <CLI/CLI.hpp>
#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "commands/table.hpp"

namespace subedit::cli {

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

    // **What goes to standard output**: the text a human reads, or one JSON
    // object per input for a script (ADR 0038). Global, like the others, and for
    // the same reason: it is about the run, not about one subcommand. A value that
    // is neither is a usage error, answered before any file is touched.
    std::string format = "text";
    app.add_option("--format", format, "Form of the result on standard output: text or json")
        ->check(CLI::IsMember({"text", "json"}))
        ->option_text("text|json");

    // **Global, because every command that has a parameter in the file reads it
    // the same way** (ADR 0045). The two exclude each other.
    std::string configFile;
    bool noConfig = false;
    CLI::Option* configOption =
        app.add_option("--config",
                       configFile,
                       "Read the operation settings from FILE instead of the user's settings file")
            ->option_text("FILE");
    app.add_flag("--no-config", noConfig, "Ignore any settings file: use the built-in defaults")
        ->excludes(configOption);

    // **The encodings the tool can read and write**, one a line, and the tool
    // stops — as `--version` does, and for the same reason: it is the answer, not a
    // step. Global because the question is about the tool, not about a subcommand.
    bool listEncodings = false;
    app.add_flag("--list-encodings",
                 listEncodings,
                 "Write the encodings that can be read and written, one a line, and stop");

    std::vector<Declared> declared;
    for (const Command& command : commands()) {
        declared.push_back(command.declare(app, command.name));
    }

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

    // An empty name names no file: taken as not given, it would fall back on
    // the user's own file, the one `--config` was given to avoid.
    if (configOption->count() > 0 && configFile.empty()) {
        return refuse("--config needs a file name");
    }

    // Before the help and before any subcommand: the list is the whole answer, and
    // it does not depend on `-q` — a result is not narration.
    if (listEncodings) {
        (void)subedit::cli::listEncodings(std::cout, format == "json");
        return ExitCode::Success;
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
    Reporter reporter{std::cerr, *level};
    reporter = reporter.withTextOutput(std::cout);
    if (format == "json") {
        reporter = reporter.withRecords(std::cout);
    }

    // Loaded once for every command, and announced by those that use it: a
    // command with nothing in the file says nothing about it.
    const SharedSettings shared =
        loadSharedSettings(files,
                           {.disabled = noConfig,
                            .explicitPath = configFile,
                            .userPath = platform::resolvedUserSettingsPath()});

    for (const Declared& command : declared) {
        if (command.app->parsed()) {
            return command.run(
                files, encoding, reporter.forCommand(command.app->get_name()), shared);
        }
    }
    return ExitCode::Success;
}

} // namespace subedit::cli
