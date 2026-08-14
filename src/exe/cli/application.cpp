#include "application.hpp"

#include <subedit/cli/inspection.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/verbosity.hpp>
#include <subedit/core/format/real_file_system.hpp>
#include <subedit/core/version.hpp>

#include <CLI/CLI.hpp>
#include <expected>
#include <iostream>
#include <string>
#include <vector>

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

    CLI::App* inspect = app.add_subcommand("inspect", "Report what a subtitle file is made of");
    std::vector<std::string> inspected;
    inspect->add_option("files", inspected, "Subtitle files to report on")->required();

    // Both readings of disorder are offered rather than one chosen: this phase
    // is a harness, and comparing them on real files will settle the question
    // better than arguing it without data. Phase 5 inherits the answer and this
    // option goes — see docs/specs/03-cli.md.
    std::string reading = "breaks";
    inspect
        ->add_option("--order-report", reading, "Which lines to name when the file is out of order")
        ->check(CLI::IsMember({"breaks", "late"}))
        ->capture_default_str();

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

    const core::RealFileSystem files;
    const Reporter reporter{std::cerr, *level};

    if (inspect->parsed()) {
        // The value is one of the two: CLI11 refused anything else before we
        // got here, which is what makes this a lookup rather than a decision.
        const core::OrderReport order =
            reading == "late" ? core::OrderReport::Late : core::OrderReport::Breaks;
        return inspectAll(files, inspected, std::cout, reporter, order);
    }
    return ExitCode::Success;
}

} // namespace subedit::cli
