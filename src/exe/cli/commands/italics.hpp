#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `italics` on `app`.
[[nodiscard]] Declared declareItalics(CLI::App& app, std::string_view name);

} // namespace subedit::cli
