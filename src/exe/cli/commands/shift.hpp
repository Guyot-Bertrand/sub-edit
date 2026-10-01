#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `shift` on `app`.
[[nodiscard]] Declared declareShift(CLI::App& app, std::string_view name);

} // namespace subedit::cli
