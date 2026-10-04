#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `adjust` on `app`.
[[nodiscard]] Declared declareAdjust(CLI::App& app, std::string_view name);

} // namespace subedit::cli
