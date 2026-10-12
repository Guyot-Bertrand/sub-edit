#pragma once

// The operation settings the command line shares with the window — spec 15,
// decisions S1, S2, S4 and S6.
//
// **Loaded once, before any command runs**, and handed to the commands that
// have a parameter in the file. Reading it never fails: a file that is absent,
// unreadable or partly unreadable leaves the core defaults where it says
// nothing, which is what the window does with the same file.
//
// **Except for a file `--config` names and that is not there**: loading still
// does not fail, but a command that reads the values asks `missingNamedFile`
// and refuses to run on defaults the user did not ask for.

#include <subedit/cli/json.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/io/file_system.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

/// Where the values of a `SharedSettings` come from.
enum class SettingsSource {
    /// `--no-config`: nothing was read.
    Disabled,
    /// The file is not there, or no path could be resolved.
    Missing,
    /// The file is there and could not be read at all.
    Unreadable,
    /// The file was read; some values may still have been refused.
    Read,
};

/// What the command line asked for, and what the platform resolved.
struct SettingsChoice {
    /// `--no-config`.
    bool disabled = false;

    /// `--config FILE`, empty when absent.
    std::filesystem::path explicitPath{};

    /// The user's settings file, handed down by main; empty when unresolvable.
    std::filesystem::path userPath{};
};

/// The operation settings a command is given.
struct SharedSettings {
    /// The core defaults when disabled, missing or unreadable.
    core::OperationSettings values{};

    /// The file looked at; empty when `Disabled`.
    std::filesystem::path path{};

    /// What `config set` writes: the explicit path, else the user's, even when
    /// reading is disabled.
    std::filesystem::path target{};

    SettingsSource source = SettingsSource::Disabled;

    /// Whether the file was named by `--config`, rather than the user's own.
    bool named = false;

    /// The values that could not be read, in the file's order.
    std::vector<core::SettingsDiagnostic> diagnostics{};
};

/// Reads the settings `choice` designates. Never fails.
[[nodiscard]] SharedSettings loadSharedSettings(const core::FileSystem& files,
                                                const SettingsChoice& choice);

/// Why a command that reads the values cannot run: the file `--config` named
/// is not there. Nothing when the file was read, unreadable, the user's own or
/// disabled — a missing user file is the defaults, a misspelt name is not.
[[nodiscard]] std::optional<std::string> missingNamedFile(const SharedSettings& shared);

/// Says, at the default level, which file was read and what it refused.
void announceSettings(const Reporter& reporter, const SharedSettings& shared);

/// The path that was read, as a JSON string, or null when none was.
[[nodiscard]] Json settingsRecord(const SharedSettings& shared);

} // namespace subedit::cli
