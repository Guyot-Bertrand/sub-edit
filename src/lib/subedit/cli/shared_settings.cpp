#include <subedit/cli/reporter.hpp>
#include <subedit/cli/shared_settings.hpp>
#include <subedit/core/i18n/messages.hpp>

#include <optional>
#include <string>

namespace subedit::cli {

SharedSettings loadSharedSettings(const core::FileSystem& files, const SettingsChoice& choice) {
    SharedSettings shared;
    shared.target = choice.explicitPath.empty() ? choice.userPath : choice.explicitPath;
    shared.named = !choice.explicitPath.empty();
    if (choice.disabled) {
        return shared;
    }

    shared.path = shared.target;
    // Neither XDG_CONFIG_HOME nor HOME: there is no file to look at, which is
    // the same as one that is not there.
    if (shared.path.empty()) {
        shared.source = SettingsSource::Missing;
        return shared;
    }

    core::OperationSettingsRead read = core::readOperationSettings(files, shared.path);
    if (read.unreadable) {
        shared.source = SettingsSource::Unreadable;
        return shared;
    }
    if (!read.found) {
        shared.source = SettingsSource::Missing;
        return shared;
    }
    shared.source = SettingsSource::Read;
    shared.values = read.settings;
    shared.diagnostics = std::move(read.diagnostics);
    return shared;
}

std::optional<std::string> missingNamedFile(const SharedSettings& shared) {
    if (shared.named && shared.source == SettingsSource::Missing) {
        // TRANSLATORS: %1 is the settings file named by --config.
        return core::substitute(core::translate("settings file not found: %1"),
                                {shared.path.string()});
    }
    return std::nullopt;
}

void announceSettings(const Reporter& reporter, const SharedSettings& shared) {
    const std::string path = shared.path.string();
    switch (shared.source) {
    case SettingsSource::Disabled:
        return;
    case SettingsSource::Read:
        reporter.tell(1, Key::Settings, path);
        for (const core::SettingsDiagnostic& diagnostic : shared.diagnostics) {
            reporter.tell(
                1,
                Key::Warning,
                // TRANSLATORS: %1 is a key of the settings file, %2 the value the file gave it.
                core::substitute(core::translate("setting %1: cannot read '%2', default kept"),
                                 {diagnostic.key, diagnostic.value}));
        }
        return;
    case SettingsSource::Missing:
        if (!path.empty()) {
            reporter.tell(
                1,
                Key::Warning,
                // TRANSLATORS: %1 is the settings file.
                core::substitute(core::translate("settings file %1 not found, defaults used"),
                                 {path}));
        }
        return;
    case SettingsSource::Unreadable: {
        // TRANSLATORS: %1 is the settings file.
        const auto said = core::translate("settings file %1 unreadable, defaults used");
        reporter.tell(1, Key::Warning, core::substitute(said, {path}));
        return;
    }
    }
}

Json settingsRecord(const SharedSettings& shared) {
    if (shared.source == SettingsSource::Read) {
        return Json{shared.path.string()};
    }
    return Json{};
}

} // namespace subedit::cli
