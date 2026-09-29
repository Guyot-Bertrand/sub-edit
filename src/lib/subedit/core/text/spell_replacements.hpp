#pragma once

// The list of replacements a user has made, one file per language — Gaupol's
// `spell-check/<language>.repl` (`aeidon/spell.py`), decision D6 of the spec
// of phase 12, issue #507. **In this program's own configuration, never
// Gaupol's**: it is not read from there.

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// A word and what the user replaced it with.
struct SpellReplacement {
    std::string word;
    std::string replacement;

    friend bool operator==(const SpellReplacement&, const SpellReplacement&) = default;
};

/// The most a file keeps: the last ten thousand distinct ones, Gaupol's own.
inline constexpr std::size_t kMaxSpellReplacements = 10000;

/// The file of `language` under `configDirectory`: `spell-check/<language>.repl`.
/// **The directory is given**, never resolved here: a test that resolved a
/// configuration location would write to whoever runs it.
[[nodiscard]] std::filesystem::path
spellReplacementFile(const std::filesystem::path& configDirectory, std::string_view language);

/// Reads `word|replacement` lines: duplicated lines dropped, blank ones too,
/// each split at its first `|`. **A line with no `|` is skipped** — Gaupol
/// reads it as a one-item pair that fails the first time a word is suggested.
[[nodiscard]] std::vector<SpellReplacement> parseSpellReplacements(std::string_view text);

/// The text of `replacements`, one `word|replacement` line each, newline
/// terminated: duplicates dropped keeping the last of each, and only the last
/// `kMaxSpellReplacements` kept. **Empty for none**, since Gaupol writes no
/// file then.
[[nodiscard]] std::string renderSpellReplacements(std::span<const SpellReplacement> replacements);

} // namespace subedit::core
