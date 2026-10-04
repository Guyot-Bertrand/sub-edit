#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `dialogue-dashes` on `app`.
[[nodiscard]] Declared declareDialogueDashes(CLI::App& app, std::string_view name);

} // namespace subedit::cli
