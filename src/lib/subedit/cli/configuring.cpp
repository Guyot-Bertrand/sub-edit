#include <subedit/cli/configuring.hpp>
#include <subedit/cli/json.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/wording/formats.hpp>

#include <algorithm>
#include <expected>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

[[nodiscard]] std::string_view surfaceName(core::SettingSurface surface) {
    return surface == core::SettingSurface::Both ? "both" : "window-only";
}

/// A record of `config`, its schema and command first as every record has.
[[nodiscard]] Json recordOf(const Reporter& reporter) {
    return Json::object().set("schema", kSchema).set("command", reporter.command());
}

/// The value a fresh `Settings` writes for `key`.
[[nodiscard]] std::string defaultOf(const std::vector<core::SettingEntry>& defaults,
                                    std::string_view key) {
    const auto found = std::ranges::find_if(
        defaults, [key](const core::SettingEntry& entry) { return entry.key->key == key; });
    return found == defaults.end() ? std::string{} : found->value;
}

/// Reads the settings at `path` whole, applies `change`, and writes them back:
/// a rewrite keeps every other setting, display preferences included.
///
/// **The change is judged first**, so that a refused key or value is a usage
/// error whatever the state of the file, and writes nothing. A file that is
/// there and cannot be read is not rewritten either: that would replace what
/// nobody has seen with defaults.
template<typename Change>
[[nodiscard]] ExitCode rewrite(core::FileSystem& files,
                               const std::filesystem::path& path,
                               const Reporter& reporter,
                               Change change) {
    core::SettingsRead read = core::readSettings(files, path);
    if (const std::expected<void, std::string> changed = change(read.settings); !changed) {
        reporter.failed(changed.error());
        return ExitCode::Usage;
    }
    if (read.unreadable) {
        // TRANSLATORS: %1 is the settings file, %2 why it could not be read.
        reporter.failed(core::substitute(core::translate("%1: %2; nothing written"),
                                         {path.string(), core::reasonOf(read.unreadable->kind)}));
        return ExitCode::AllFailed;
    }
    // Said at the default level: the rewrite loses what the file held, and a
    // loss is not a detail.
    for (const core::SettingsDiagnostic& diagnostic : read.diagnostics) {
        reporter.say(
            1,
            // TRANSLATORS: %1 is a settings key, %2 the value the file gave it.
            core::substitute(core::translate("%1: unreadable value '%2' replaced by its default"),
                             {diagnostic.key, diagnostic.value}));
    }
    if (const std::expected<void, core::FileError> saved =
            core::writeSettings(files, path, read.settings);
        !saved) {
        reporter.failed(core::substitute(
            core::translate("%1: %2"), {path.string(), core::reasonOfWriting(saved.error().kind)}));
        return ExitCode::AllFailed;
    }
    return ExitCode::Success;
}

/// The value `key` has in `settings`, as the file writes it.
[[nodiscard]] std::string valueOf(const core::Settings& settings, std::string_view key) {
    return defaultOf(core::describeOperationSettings(settings), key);
}

/// Every operation key of `settings` with its value, source and default: one
/// record each, and the text of them returned.
[[nodiscard]] std::string entriesOf(const core::Settings& settings, const Reporter& reporter) {
    std::string text;
    for (const core::SettingEntry& entry : core::describeOperationSettings(settings)) {
        reporter.record(recordOf(reporter)
                            .set("key", entry.key->key)
                            .set("value", entry.value)
                            .set("source", entry.fromFile ? "file" : "default")
                            .set("default", entry.defaultValue)
                            .set("used_by", entry.key->usedBy)
                            .set("surface", surfaceName(entry.key->surface)));
        // TRANSLATORS: %1 is a settings key, %2 its value, %3 its default. The value
        // came from the file; « file » and « default » name where it came from.
        text += entry.fromFile
                    ? core::substitute(core::translate("%1 = %2   [file]   (default %3)"),
                                       {entry.key->key, entry.value, entry.defaultValue})
                    // TRANSLATORS: %1 is a settings key, %2 its default value.
                    : core::substitute(core::translate("%1 = %2   [default]"),
                                       {entry.key->key, entry.value});
        text += '\n';
    }
    return text;
}

} // namespace

ExitCode listSettingKeys(const Reporter& reporter) {
    const std::vector<core::SettingEntry> defaults =
        core::describeOperationSettings(core::Settings{});
    std::string text;
    for (const core::SettingKey& key : core::operationKeys()) {
        const std::string fallback = defaultOf(defaults, key.key);
        if (reporter.recording()) {
            reporter.record(recordOf(reporter)
                                .set("key", key.key)
                                .set("description", key.description)
                                .set("accepts", key.accepts)
                                .set("default", fallback)
                                .set("used_by", key.usedBy)
                                .set("surface", surfaceName(key.surface)));
            continue;
        }
        text += std::string{key.key} + '\n';
        text += "    " + std::string{key.description} + '\n';
        // TRANSLATORS: %1 is what a settings key accepts, as « true or false ».
        text += "    " + core::substitute(core::translate("Accepts: %1"), {key.accepts}) + '\n';
        // TRANSLATORS: %1 is the default value of a settings key.
        text += "    " + core::substitute(core::translate("Default: %1"), {fallback}) + '\n';
        // TRANSLATORS: %1 lists the commands and window actions that read a settings key.
        text += "    " + core::substitute(core::translate("Used by: %1"), {key.usedBy}) + '\n';
    }
    reporter.result(text);
    return ExitCode::Success;
}

ExitCode
showSettings(core::FileSystem& files, const std::filesystem::path& path, const Reporter& reporter) {
    const core::SettingsRead read = core::readSettings(files, path);
    const bool exists = read.found || read.unreadable.has_value();

    std::string text;
    if (read.unreadable) {
        // TRANSLATORS: %1 is the settings file, %2 why it could not be read.
        text = core::substitute(core::translate("File: %1 (unreadable: %2)"),
                                {path.string(), core::reasonOf(read.unreadable->kind)});
    } else if (read.found) {
        // TRANSLATORS: %1 is the settings file.
        text = core::substitute(core::translate("File: %1 (found)"), {path.string()});
    } else {
        // TRANSLATORS: %1 is the settings file.
        text = core::substitute(core::translate("File: %1 (not found)"), {path.string()});
    }
    text += '\n';
    reporter.record(recordOf(reporter).set("file", path.string()).set("exists", exists));
    reporter.result(text + entriesOf(read.settings, reporter));
    return ExitCode::Success;
}

ExitCode showDefaultSettings(const Reporter& reporter) {
    reporter.record(recordOf(reporter).set("file", nullptr).set("exists", false));
    reporter.result(std::string{core::translate("File: none (--no-config)")} + '\n' +
                    entriesOf(core::Settings{}, reporter));
    return ExitCode::Success;
}

ExitCode setSetting(core::FileSystem& files,
                    const std::filesystem::path& path,
                    std::string_view key,
                    std::string_view value,
                    const Reporter& reporter) {
    std::string now;
    const ExitCode code = rewrite(files, path, reporter, [&](core::Settings& settings) {
        std::expected<void, std::string> set = core::setOperationOption(settings, key, value);
        if (set)
            now = valueOf(settings, key);
        return set;
    });
    if (code == ExitCode::Success)
        reporter.say(1, std::string{key} + " = " + now);
    return code;
}

ExitCode unsetSetting(core::FileSystem& files,
                      const std::filesystem::path& path,
                      std::string_view key,
                      const Reporter& reporter) {
    std::string now;
    const ExitCode code = rewrite(files, path, reporter, [&](core::Settings& settings) {
        std::expected<void, std::string> unset = core::unsetOperationOption(settings, key);
        if (unset)
            now = valueOf(settings, key);
        return unset;
    });
    if (code == ExitCode::Success)
        // TRANSLATORS: %1 is a settings key, %2 the default it now has.
        reporter.say(1,
                     core::substitute(core::translate("%1 reset to its default (%2)"), {key, now}));
    return code;
}

} // namespace subedit::cli
