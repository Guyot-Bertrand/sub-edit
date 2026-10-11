#include "config.hpp"

#include <subedit/cli/configuring.hpp>

#include <CLI/CLI.hpp>
#include <memory>
#include <optional>
#include <string>

namespace subedit::cli {

namespace {

/// What `config` was asked for: which of its subcommands, and the key and
/// value they take.
struct ConfigOptions {
    std::string key;
    std::string value;
    const CLI::App* list = nullptr;
    const CLI::App* show = nullptr;
    const CLI::App* set = nullptr;
    const CLI::App* unset = nullptr;
};

CLI::App* describeConfig(CLI::App& app, std::string_view name, ConfigOptions& options) {
    CLI::App* config =
        app.add_subcommand(std::string{name}, "List, show and change the operation settings");
    config->require_subcommand(1);

    options.list = config->add_subcommand(
        "list", "Write every operation key, what it is for, what it accepts and its default");
    options.show = config->add_subcommand(
        "show", "Write the settings file used, then every operation key with its value");

    CLI::App* set = config->add_subcommand(
        "set", "Write KEY = VALUE into the settings file, keeping every other setting");
    set->add_option("key", options.key, "Operation key, as config list names it")->required();
    set->add_option("value", options.value, "Value, as config list says it accepts")->required();
    options.set = set;

    CLI::App* unset = config->add_subcommand(
        "unset", "Put KEY back to its default in the settings file, keeping every other setting");
    unset->add_option("key", options.key, "Operation key, as config list names it")->required();
    options.unset = unset;
    return config;
}

ExitCode runConfig(const ConfigOptions& options,
                   core::FileSystem& files,
                   const Reporter& reporter,
                   const SharedSettings& shared) {
    // The list is about the program, not about a file: neither option changes it.
    if (options.list->parsed()) {
        return listSettingKeys(reporter);
    }

    const bool disabled = shared.source == SettingsSource::Disabled;
    if (options.show->parsed() && disabled) {
        return showDefaultSettings(reporter);
    }
    if (disabled) {
        return refuse("--no-config has nothing to write to");
    }
    // Neither --config nor a home to resolve the user's file in.
    if (shared.target.empty()) {
        return refuse("no settings file could be located; give one with --config FILE");
    }

    if (options.show->parsed()) {
        return showSettings(files, shared.target, reporter);
    }
    if (options.set->parsed()) {
        return setSetting(files, shared.target, options.key, options.value, reporter);
    }
    return unsetSetting(files, shared.target, options.key, reporter);
}

} // namespace

Declared declareConfig(CLI::App& app, std::string_view name) {
    // Not `declareWith`: its subcommand is the one that runs, and here the one
    // that runs is a subcommand of it.
    auto options = std::make_shared<ConfigOptions>();
    const CLI::App* config = describeConfig(app, name, *options);
    return Declared{config,
                    [options](core::FileSystem& files,
                              const std::optional<core::Encoding>& /*reading*/,
                              const Reporter& reporter,
                              const SharedSettings& shared) {
                        return runConfig(*options, files, reporter, shared);
                    }};
}

} // namespace subedit::cli
