#pragma once

// The `config` subcommands: list the operation keys, show the file, set and
// unset one key — spec 17, decision D7.
//
// **The display preferences are never touched by name**: they are the window's,
// and `set`/`unset` refuse them. They survive a rewrite all the same, because
// the file is read whole and written whole.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/file_system.hpp>

#include <filesystem>
#include <string_view>

namespace subedit::cli {

/// `config list`: the keys, what they are for and what they accept. No file.
[[nodiscard]] ExitCode listSettingKeys(const Reporter& reporter);

/// `config show`: the path, whether it exists, then every key with value,
/// source and default.
[[nodiscard]] ExitCode
showSettings(core::FileSystem& files, const std::filesystem::path& path, const Reporter& reporter);

/// `config set`: rewrite the file atomically with `key` at `value`, keeping
/// every other setting. A refused key or value writes nothing.
[[nodiscard]] ExitCode setSetting(core::FileSystem& files,
                                  const std::filesystem::path& path,
                                  std::string_view key,
                                  std::string_view value,
                                  const Reporter& reporter);

/// `config unset`: rewrite the file atomically with `key` at its default,
/// keeping every other setting. A refused key writes nothing.
[[nodiscard]] ExitCode unsetSetting(core::FileSystem& files,
                                    const std::filesystem::path& path,
                                    std::string_view key,
                                    const Reporter& reporter);

} // namespace subedit::cli
