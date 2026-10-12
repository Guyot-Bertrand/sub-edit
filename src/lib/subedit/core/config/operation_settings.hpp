#pragma once

// The operation part of the settings — spec 15, decisions S3, S7 and S8.
//
// **A slice of `Settings`, not a second struct to keep in step**: the window
// keeps one flat `Settings`, and the command line is handed only this slice, so
// that it never reads a display preference is a property of the type.

#include <subedit/core/config/settings.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// What both the window and the command line read from the settings file.
struct OperationSettings {
    DurationAdjustmentSettings durationAdjustment{};
    CorrectionSettings correction{};

    friend bool operator==(const OperationSettings&, const OperationSettings&) = default;
};

/// What reading the operation part of a settings file gave.
struct OperationSettingsRead {
    OperationSettings settings{};

    /// The unreadable values of operation keys only, in the file's order: a
    /// display preference nobody can read is the window's business.
    std::vector<SettingsDiagnostic> diagnostics{};

    /// Set when the file is there and could not be read at all.
    std::optional<FileError> unreadable{};

    /// True when the file existed and was read.
    bool found = false;
};

/// Reads the operation part of the settings at `path`, with the tolerance of
/// `readSettings`: it never fails.
[[nodiscard]] OperationSettingsRead readOperationSettings(const FileSystem& files,
                                                          const std::filesystem::path& path);

/// The operation part of `settings`.
[[nodiscard]] OperationSettings operationPartOf(const Settings& settings);

/// Whether the command line honours a key, or only the window does — D8.
enum class SettingSurface {
    Both,
    WindowOnly,
};

/// One operation key of the catalogue — D7. The default is not here: it is
/// what the file writes for a fresh `Settings`, so it cannot drift.
struct SettingKey {
    std::string_view key;

    /// What it is for, in one sentence.
    std::string_view description;

    /// The closed set, the bounds, or the form of the values it accepts.
    std::string_view accepts;

    /// The commands and window actions that read it.
    std::string_view usedBy;

    /// `Both` when the command line honours it.
    SettingSurface surface;
};

/// Every operation key, in the order the file writes them.
/// `correction.activations` is not one of them: the window manages that list,
/// and the command line has `--enable` and `--disable`.
[[nodiscard]] std::span<const SettingKey> operationKeys();

/// The catalogue entry of `key`, or null when it is not an operation key.
[[nodiscard]] const SettingKey* findOperationKey(std::string_view key);

/// One operation key with its current value, as `config show` prints it.
struct SettingEntry {
    const SettingKey* key = nullptr;

    /// The current value, as the file writes it.
    std::string value;

    /// The value a fresh `Settings` writes.
    std::string defaultValue;

    /// False when the value is the default, which the file writes commented.
    bool fromFile = false;
};

/// Every catalogue key with its value in `settings`, its default and its
/// source, in the catalogue's order.
[[nodiscard]] std::vector<SettingEntry> describeOperationSettings(const Settings& settings);

/// Sets operation key `key` to `value`, read by the file's own reader.
///
/// On refusal — a display preference, an unknown key, or a value the reader
/// cannot read — `settings` is left untouched and the error is a sentence for
/// the user.
[[nodiscard]] std::expected<void, std::string>
setOperationOption(Settings& settings, std::string_view key, std::string_view value);

/// Puts operation key `key` back to its default. A key already at its default
/// is a success that changes nothing.
[[nodiscard]] std::expected<void, std::string> unsetOperationOption(Settings& settings,
                                                                    std::string_view key);

} // namespace subedit::core
