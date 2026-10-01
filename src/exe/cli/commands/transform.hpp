#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `transform` on `app`.
[[nodiscard]] Declared declareTransform(CLI::App& app, std::string_view name);

} // namespace subedit::cli
