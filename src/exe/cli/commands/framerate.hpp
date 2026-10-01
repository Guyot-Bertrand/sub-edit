#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `framerate` on `app`.
[[nodiscard]] Declared declareFrameRate(CLI::App& app, std::string_view name);

} // namespace subedit::cli
