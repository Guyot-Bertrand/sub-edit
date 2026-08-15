#pragma once

// The document every benchmark of the edit operations measures on.
//
// **Positions and text answer to two different rules.** The positions are
// frozen — two subtitles every five seconds for three hours, the count and the
// spacing the format benchmark generates too — because the journal of
// measurements compares each figure to its own history, and a document of
// another length would end that comparison. The text, on the contrary, is
// meant to look like a real subtitling, because phase 4 is the first to
// transform text and a benchmark run on four thousand copies of one sentence
// would measure the fastest case that exists and the one that never happens.
//
// The format benchmark keeps the one sentence, and that is not an oversight:
// reading and writing are measured on the bytes of a file, and giving them a
// lighter document would move two figures that have three phases of history
// behind them, for a phase that does not touch them.
//
// What « looks real » means here was read off files rather than decided: the
// figures below come from the private corpus — 55 files, 69 802 subtitles —
// measured by `src/scripts/measure-mentions.py`. That corpus is each machine's
// own, absent from a fresh clone and from the CI, so the numbers are written
// down here and the fixture carries them in its tables; nothing at run time
// reads a file.
//
// | What                                    | Corpus         | Fixture |
// | :-------------------------------------- | :------------- | ------: |
// | subtitles carrying a mention            | 11.5 to 23.5 % |  20.0 % |
// | of those, emptied by its removal        | 58.9 %         |  58.1 % |
// | of those, a whole line being the mention| 25.0 %         |  25.6 % |
// | of those, cut by the line break         | 1.0 %          |   0.3 % |
// | brackets, against parentheses           | 67.6 %         |  65.2 % |
// | subtitles on two lines                  | 38.3 %         |  36.0 % |
// | text length — median, ninth decile      | 28, 49         |  28, 57 |
//
// The first line is the one that needed a decision. Averaged over the whole
// corpus the proportion is 2.1 %, because most files are not subtitled for the
// hard of hearing at all and carry no mention whatsoever. But an operation
// that removes mentions is run on a file that has them: the four files that do
// carry them go from 11.5 % to 23.5 %, and it is that range the fixture sits
// in. Measuring on the corpus average would reproduce, one order of magnitude
// milder, the very flaw this fixture was written to fix.
//
// The shapes the text takes — the mention alone, at the head, in the middle, at
// the end, twice, alone on one line of two, straddling the line break — are the
// cases `src/test/data/textes/mentions.cas` settled one by one, in the
// proportions the corpus gives them. Each is worth a different amount of work:
// a subtitle the removal empties is the expensive one, since the subtitle then
// goes.
//
// The composition is asserted by a test case in the implementation file, so
// that the table above cannot quietly stop being true.

#include <subedit/core/model/project.hpp>

#include <cstddef>

namespace subedit::test {

/// Roughly a feature film: two subtitles every five seconds for three hours.
inline constexpr std::size_t kSubtitleCount = 4000;

/// Builds the document described above. Identical on every machine and every
/// run — the tables are fixed and the order they are laid in is arithmetic.
[[nodiscard]] core::Project fullLengthProject();

} // namespace subedit::test
