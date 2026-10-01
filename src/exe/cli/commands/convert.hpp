#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `convert` on `app`.
[[nodiscard]] Declared declareConvert(CLI::App& app, std::string_view name);

} // namespace subedit::cli
