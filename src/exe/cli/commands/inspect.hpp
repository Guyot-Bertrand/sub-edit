#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `inspect` on `app`.
[[nodiscard]] Declared declareInspect(CLI::App& app, std::string_view name);

} // namespace subedit::cli
