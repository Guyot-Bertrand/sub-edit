#pragma once

// Sentences made of a count, a word or a flag, and nothing else — the light
// family of `core/wording/`, which pulls in no other header of the project so
// that saying "3 subtitles" costs a unit only <string>. See `formats.hpp` for
// why this wording lives in the core.

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::core {

/// What applying accepted corrections did, across every project it touched —
/// decision D8 of the phase-12 spec, Gaupol's own template
/// (`gaupol/assistants.py`): "Edited N and removed M subtitles". `corrected`
/// never counts a subtitle `removed` also counts: a text the assistant
/// blanked is a removal, not an edit that happened to leave nothing.
[[nodiscard]] std::string noticeOfCorrection(std::size_t corrected, std::size_t removed);

/// That removing the hearing-impaired mentions found none — the window says it
/// in a box and the command line on its error stream, in these words: said,
/// and nothing put in the history (issue #545).
[[nodiscard]] std::string noMentionToRemove();

/// What removing the hearing-impaired mentions did: how many subtitles it
/// rewrote and how many it took out. `cleaned` never counts a subtitle
/// `removed` also counts.
[[nodiscard]] std::string noticeOfMentionsRemoved(std::size_t cleaned, std::size_t removed);

/// Why a shift is refused when a subtitle would start before the origin, which
/// no subtitle file can hold. **The rule** is `firstBeforeOrigin`, shared since
/// #132; this is its sentence, and `number` is the first subtitle it names.
[[nodiscard]] std::string shiftBeforeTheOrigin(std::size_t number);

/// Why spell-checking is unavailable for `language` — decision D6 of the
/// phase-12 spec: the program works and the function switches itself off, and
/// says so in these words, where Gaupol drops the page without a word.
[[nodiscard]] std::string noDictionaryFor(std::string_view language);

/// A count and its noun, agreeing: "1 subtitle", "2 subtitles".
///
/// Small, and worth it: "1 subtitles" is the kind of sloppiness a reader
/// notices immediately and that ends up copied into the manual, which is
/// generated from what the tool actually prints.
[[nodiscard]] std::string countOf(std::size_t count, std::string_view noun);

/// What a search that found nothing says: Gaupol's « "…" not found ».
[[nodiscard]] std::string notFound(std::string_view pattern);

/// What no rewrite had to do — italics, case or dialogue dashes alike. Never
/// followed by a history entry.
[[nodiscard]] std::string nothingToChange();

/// What toggling italics did: "N subtitle(s) put in italics" or "… taken out
/// of italics".
[[nodiscard]] std::string noticeOfItalics(std::size_t count, bool italic);

/// What re-casing did: "N subtitle(s) recased".
[[nodiscard]] std::string noticeOfRecase(std::size_t count);

/// What toggling dialogue dashes did: "N subtitle(s) dashed" or "… undashed".
[[nodiscard]] std::string noticeOfDialogueDashes(std::size_t count, bool dashed);

/// What `Replace All` did: how many matches it replaced.
[[nodiscard]] std::string noticeOfReplaceAll(std::size_t count);

/// The same, over several projects: how many matches, and in how many projects
/// they were.
[[nodiscard]] std::string noticeOfReplaceAll(std::size_t count, std::size_t projects);

/// What `Split Project…` did: how many subtitles left for the new project.
[[nodiscard]] std::string noticeOfSplit(std::size_t count);

} // namespace subedit::core
