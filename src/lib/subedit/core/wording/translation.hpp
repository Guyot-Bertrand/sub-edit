#pragma once

// What the tool says of opening a translation. Shared wording of
// `core/wording/`; see `formats.hpp` for why it lives in the core.

#include <subedit/core/format/translation_file.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

struct TranslationOutcome;

/// What opening a translation did, in one sentence — decision D4 of the phase-11
/// spec, and ADR 0008: said, and not left for the user to find out.
///
/// **Each clause only when it is not zero**, in the order a reader meets the
/// things in: the lines that found their subtitle, the subtitles born of a line,
/// the subtitles no line reached, the lines that were out of order. A file with
/// no line says so, rather than blame the subtitles for it.
///
/// It lives here for the reason the rest of this file does: the window says it
/// in a box or in the status bar, and the command line will say it in the same
/// words when the translation reaches it.
[[nodiscard]] std::string noticeOf(const TranslationOutcome& outcome);

/// Why a translation could not be opened, in the words of the file it names —
/// or, when it is the main file itself, in a sentence of its own.
[[nodiscard]] std::string_view reasonOf(const TranslationError& error);

} // namespace subedit::core
