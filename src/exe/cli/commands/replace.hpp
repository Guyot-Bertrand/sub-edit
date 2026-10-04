#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `replace` on `app`.
[[nodiscard]] Declared declareReplace(CLI::App& app, std::string_view name);

} // namespace subedit::cli
