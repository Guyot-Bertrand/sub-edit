#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `hearing-impaired` on `app`.
[[nodiscard]] Declared declareHearingImpaired(CLI::App& app, std::string_view name);

} // namespace subedit::cli
