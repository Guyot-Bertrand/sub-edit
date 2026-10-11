#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `repair` on `app`.
[[nodiscard]] Declared declareRepair(CLI::App& app, std::string_view name);

} // namespace subedit::cli
