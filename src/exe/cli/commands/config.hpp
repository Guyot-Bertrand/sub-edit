#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `config` on `app`, with its four subcommands `list`, `show`, `set`
/// and `unset` — spec 17, decision D7.
[[nodiscard]] Declared declareConfig(CLI::App& app, std::string_view name);

} // namespace subedit::cli
