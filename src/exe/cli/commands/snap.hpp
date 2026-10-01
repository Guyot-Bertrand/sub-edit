#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `snap` on `app`.
[[nodiscard]] Declared declareSnap(CLI::App& app, std::string_view name);

} // namespace subedit::cli
