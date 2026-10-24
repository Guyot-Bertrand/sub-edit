#pragma once

#include "command.hpp"

namespace subedit::cli {

/// Declares `transcribe` on `app`.
[[nodiscard]] Declared declareTranscribe(CLI::App& app, std::string_view name);

} // namespace subedit::cli
