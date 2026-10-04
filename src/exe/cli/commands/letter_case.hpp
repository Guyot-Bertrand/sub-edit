#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `case` on `app`.
[[nodiscard]] Declared declareCase(CLI::App& app, std::string_view name);

} // namespace subedit::cli
