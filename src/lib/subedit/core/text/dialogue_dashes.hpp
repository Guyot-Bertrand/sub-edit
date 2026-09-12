#pragma once

#include <subedit/core/model/subtitle_format.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

/// Tells whether a text is missing a dialogue dash on any of its lines.
///
/// Read on the **visible** text: a line that opens with `<i>-` opens with a
/// dash, and the tag says nothing about it.
///
/// **One entry for the two ways, and this is what decides which** — Gaupol's
/// rule, and the one the italic toggle already follows: if a single line has no
/// dash, they all gain one; otherwise they all lose theirs. Pressing twice
/// leaves the text as it was found.
[[nodiscard]] bool wantsDialogueDashes(std::string_view text, SubtitleFormat format);

/// Returns `text` with a dash at the head of every one of its lines.
///
/// **Idempotent**: a dash already there is taken off before the new one goes
/// on, so a line never gains two. The three characters a subtitle uses for one
/// are all accepted — hyphen, en dash, em dash — and a hyphen is written.
[[nodiscard]] std::string withDialogueDashes(std::string_view text, SubtitleFormat format);

/// Returns `text` with the dash at the head of every line taken off, and the
/// space that followed it.
[[nodiscard]] std::string withoutDialogueDashes(std::string_view text, SubtitleFormat format);

} // namespace subedit::core
