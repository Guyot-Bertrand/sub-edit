#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `join-repeats` on `app`.
[[nodiscard]] Declared declareJoinRepeats(CLI::App& app, std::string_view name);

} // namespace subedit::cli
