#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `append` on `app`.
[[nodiscard]] Declared declareAppend(CLI::App& app, std::string_view name);

} // namespace subedit::cli
