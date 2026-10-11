#include <subedit/cli/shared_settings.hpp>

#include <string>

namespace subedit::cli {

SharedSettings loadSharedSettings(const core::FileSystem& files, const SettingsChoice& choice) {
    SharedSettings shared;
    shared.target = choice.explicitPath.empty() ? choice.userPath : choice.explicitPath;
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

void announceSettings(const Reporter& reporter, const SharedSettings& shared) {
    const std::string path = shared.path.string();
    switch (shared.source) {
    case SettingsSource::Disabled:
        return;
    case SettingsSource::Read:
        reporter.say(1, "settings: read " + path);
        for (const core::SettingsDiagnostic& diagnostic : shared.diagnostics) {
            reporter.say(1,
                         "settings: " + diagnostic.key + ": cannot read '" + diagnostic.value +
                             "', default kept");
        }
        return;
    case SettingsSource::Missing:
        if (!path.empty()) {
            reporter.say(1, "settings: " + path + " not found, defaults used");
        }
        return;
    case SettingsSource::Unreadable:
        reporter.say(1, "settings: " + path + " unreadable, defaults used");
        return;
    }
}

Json settingsRecord(const SharedSettings& shared) {
    if (shared.source == SettingsSource::Read) {
        return Json{shared.path.string()};
    }
    return Json{};
}

} // namespace subedit::cli
