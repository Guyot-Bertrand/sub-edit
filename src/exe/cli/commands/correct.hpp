#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `correct` on `app`.
[[nodiscard]] Declared declareCorrect(CLI::App& app, std::string_view name);

} // namespace subedit::cli
