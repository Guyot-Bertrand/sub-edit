#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `pair` on `app`.
[[nodiscard]] Declared declarePair(CLI::App& app, std::string_view name);

} // namespace subedit::cli
