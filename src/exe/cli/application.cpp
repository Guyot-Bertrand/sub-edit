#include "application.hpp"

#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/cli/encoding_listing.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shared_settings.hpp>
#include <subedit/cli/verbosity.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/version.hpp>
#include <subedit/platform/locations.hpp>

#include <CLI/CLI.hpp>
#include <expected>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "commands/table.hpp"

namespace subedit::cli {

namespace {

/// **The words CLI11 puts around ours in `--help`, in the user's language** (spec 15,
/// D3): the headings, the help flag's description, `REQUIRED`. CLI11 lets each be
/// replaced, and a help whose descriptions are translated and whose headings are not
/// reads as half done. Its *diagnostics* — a missing required option, an unknown flag —
/// are left in English on purpose, as the manual says (`invocation.md`, Language): they
/// are worded inside CLI11, and replacing them would mean rewording every error it can
/// raise. The metavariables (`TEXT`, `OPTIONS`, `SUBCOMMAND`) are typed-like names and
/// stay as they are, like ours.
///
/// Before any option and any subcommand: a subcommand copies its parent's help flag,
/// formatter and option defaults when it is made.
void speakTheLanguage(CLI::App& app) {
    const std::shared_ptr<CLI::FormatterBase> formatter = app.get_formatter();
    // The first word is CLI11's key, the second what is shown for it.
    formatter->label("Usage", std::string{core::translate("Usage")});
    formatter->label("Positionals", std::string{core::translate("Positionals")});
    // TRANSLATORS: written after an argument that must be given.
    formatter->label("REQUIRED", std::string{core::translate("REQUIRED")});
    // TRANSLATORS: written before the options an option cannot be given with.
    formatter->label("Excludes", std::string{core::translate("Excludes")});
    // TRANSLATORS: written before the options an option needs.
    formatter->label("Needs", std::string{core::translate("Needs")});
    app.option_defaults()->group(std::string{core::translate("Options")});
    // Made again, now that the group is the translated one.
    app.set_help_flag("-h,--help",
                      std::string{core::translate("Print this help message and exit")});
}

/// The heading of the subcommands, translated, at every depth (`config` has its own) —
/// and the group of each one's help flag, which a subcommand makes before it copies
/// the option defaults of its parent.
void headSubcommands(CLI::App& app) {
    for (CLI::App* subcommand : app.get_subcommands([](const CLI::App*) { return true; })) {
        subcommand->group(std::string{core::translate("Subcommands")});
        if (CLI::Option* help = subcommand->get_help_ptr(); help != nullptr) {
            help->group(std::string{core::translate("Options")});
        }
        headSubcommands(*subcommand);
    }
}

} // namespace

ExitCode run(int argc, char** argv) {
    CLI::App app{std::string{core::translate("Read, inspect and retime subtitle files.")}};

    // The program name, and not argv[0]: otherwise the usage line shows the
    // build path it was started from, which `make manual` would then copy into
    // the manual.
    app.name("subedit-cli");
    speakTheLanguage(app);
    // The version is a name and a number, the same in every language.
    app.set_version_flag(
        "--version",
        std::string{"subedit"} + " " + core::versionString(),
        std::string{core::translate("Display program version information and exit")});

    // How many times -v was given, not the level itself: CLI11 zeroes a bound
    // counter when the flag is absent, which would silently turn the default
    // level into silence. levelFrom() owns that decision.
    int verboseCount = 0;
    app.add_flag(
           "-v",
           verboseCount,
           std::string{core::translate("Say more: -v is the default, -vv details, -vvv debugs")})
        ->option_text(" ");

    bool quiet = false;
    app.add_flag("-q,--quiet", quiet, std::string{core::translate("Say nothing but errors")});

    // **Global, because every subcommand reads.** A file whose encoding is
    // guessed wrong is guessed wrong whatever is being done to it, and an
    // option that only `inspect` carried would leave a shift no way to be told.
    std::string reading;
    app.add_option(
           "--encoding",
           reading,
           std::string{core::translate("Encoding to read the files in; detected by default")})
        ->option_text("NAME");

    // **What goes to standard output**: the text a human reads, or one JSON
    // object per input for a script (ADR 0038). Global, like the others, and for
    // the same reason: it is about the run, not about one subcommand. A value that
    // is neither is a usage error, answered before any file is touched.
    std::string format = "text";
    app.add_option(
           "--format",
           format,
           std::string{core::translate("Form of the result on standard output: text or json")})
        ->check(CLI::IsMember({"text", "json"}))
        ->option_text("text|json");

    // **Global, because every command that has a parameter in the file reads it
    // the same way** (ADR 0045). The two exclude each other.
    std::string configFile;
    bool noConfig = false;
    CLI::Option* configOption =
        app.add_option(
               "--config",
               configFile,
               std::string{core::translate(
                   "Read the operation settings from FILE instead of the user's settings file")})
            ->option_text("FILE");
    app.add_flag(
           "--no-config",
           noConfig,
           std::string{core::translate("Ignore any settings file: use the built-in defaults")})
        ->excludes(configOption);

    // **The encodings the tool can read and write**, one a line, and the tool
    // stops — as `--version` does, and for the same reason: it is the answer, not a
    // step. Global because the question is about the tool, not about a subcommand.
    bool listEncodings = false;
    app.add_flag("--list-encodings",
                 listEncodings,
                 std::string{core::translate(
                     "Write the encodings that can be read and written, one a line, and stop")});

    std::vector<Declared> declared;
    for (const Command& command : commands()) {
        declared.push_back(command.declare(app, command.name));
    }
    headSubcommands(app);

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
        return refuse(core::translate("--config needs a file name"));
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
