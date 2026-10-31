#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `sync` on `app`.
[[nodiscard]] Declared declareSync(CLI::App& app, std::string_view name);

} // namespace subedit::cli
