#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `disjoint` on `app`.
[[nodiscard]] Declared declareDisjoint(CLI::App& app, std::string_view name);

} // namespace subedit::cli
