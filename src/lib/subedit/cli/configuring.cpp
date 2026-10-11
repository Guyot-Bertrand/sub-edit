#include <subedit/cli/configuring.hpp>
#include <subedit/cli/json.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/config/settings.hpp>
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
        reporter.failed(path.string() + ": " + std::string{core::reasonOf(read.unreadable->kind)} +
                        "; nothing written");
        return ExitCode::AllFailed;
    }
    for (const core::SettingsDiagnostic& diagnostic : read.diagnostics) {
        reporter.say(2,
                     "settings: " + diagnostic.key + ": cannot read '" + diagnostic.value +
                         "', default written");
    }
    if (const std::expected<void, core::FileError> saved =
            core::writeSettings(files, path, read.settings);
        !saved) {
        reporter.failed(path.string() + ": " +
                        std::string{core::reasonOfWriting(saved.error().kind)});
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
        text += std::string{entry.key->key} + " = " + entry.value;
        text += entry.fromFile ? "   [file]   (default " + entry.defaultValue + ")\n"
                               : std::string{"   [default]\n"};
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
        text += "    Accepts: " + std::string{key.accepts} + '\n';
        text += "    Default: " + fallback + '\n';
        text += "    Used by: " + std::string{key.usedBy} + '\n';
    }
    reporter.result(text);
    return ExitCode::Success;
}

ExitCode
showSettings(core::FileSystem& files, const std::filesystem::path& path, const Reporter& reporter) {
    const core::SettingsRead read = core::readSettings(files, path);
    const bool exists = read.found || read.unreadable.has_value();

    std::string text = "File: " + path.string();
    if (read.unreadable) {
        text += " (unreadable: " + std::string{core::reasonOf(read.unreadable->kind)} + ")\n";
    } else {
        text += read.found ? " (found)\n" : " (not found)\n";
    }
    reporter.record(recordOf(reporter).set("file", path.string()).set("exists", exists));
    reporter.result(text + entriesOf(read.settings, reporter));
    return ExitCode::Success;
}

ExitCode showDefaultSettings(const Reporter& reporter) {
    reporter.record(recordOf(reporter).set("file", nullptr).set("exists", false));
    reporter.result("File: none (--no-config)\n" + entriesOf(core::Settings{}, reporter));
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
        reporter.say(1, std::string{key} + " reset to its default (" + now + ")");
    return code;
}

} // namespace subedit::cli
