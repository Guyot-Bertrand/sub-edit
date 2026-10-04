#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `sort` on `app`.
[[nodiscard]] Declared declareSort(CLI::App& app, std::string_view name);

} // namespace subedit::cli
