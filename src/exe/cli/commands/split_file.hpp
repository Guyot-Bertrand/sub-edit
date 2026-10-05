#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `split-file` on `app`.
[[nodiscard]] Declared declareSplitFile(CLI::App& app, std::string_view name);

} // namespace subedit::cli
