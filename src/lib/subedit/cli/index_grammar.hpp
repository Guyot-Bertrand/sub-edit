#pragma once

// The one way the command line names a subtitle, and pairs it with a position.

#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::cli {

/// A subtitle, named as the user sees it, and where its start belongs.
///
/// The number is kept one-based rather than turned into a `SubtitleIndex`
/// straight away: whether it names a subtitle at all depends on the file, and
/// nothing here has read one yet.
struct Reference {
    std::size_t number;
    subedit::core::Timestamp target;
};

/// Reads the number of a subtitle, or says why the text is not one.
///
/// **Counted from 1**, as it shows on the first line of a SubRip block and in
/// every report this tool writes. Zero, a sign, a decimal point and anything
/// that is not a run of digits are refused — none of them names a subtitle in
/// any file, which is why the refusal happens here rather than once a file is
/// open.
[[nodiscard]] std::expected<std::size_t, std::string> parseSubtitleNumber(std::string_view text);

/// Reads `<index>=<time>`, or says why the text is not one.
///
/// The two halves are those of the rest of the command line: the numbering of
/// [`parseSubtitleNumber`], and the time grammar of `--by`. Splitting on the
/// **first** equals sign leaves any other one to the time grammar, which
/// refuses it — reading `3=1=2` as anything would be a guess.
[[nodiscard]] std::expected<Reference, std::string> parseReference(std::string_view text);

/// The subtitles `--range` names, as the user wrote them.
///
/// Both ends are kept **one-based and inclusive**, and the end is optional: `N-`
/// goes to the last subtitle, whatever the file holds. Whether the ends name
/// subtitles of a file is decided against that file (`selectionOf`), for the
/// reason `Reference` keeps its number as written.
struct Range {
    std::size_t first = 1;

    /// Empty means "to the end of the file".
    std::optional<std::size_t> last;

    friend bool operator==(const Range&, const Range&) = default;
};

/// Reads `N-M` or `N-`, or says why the text is not a range.
///
/// **Counted from 1 and inclusive of both ends**, with the numbering of
/// `parseSubtitleNumber` — zero, a sign and a decimal point are refused there.
/// A range that ends before it starts is refused here, since no file could
/// satisfy it. A lone `N` is not accepted: one subtitle is `N-N`, said once,
/// and a form that meant two things (`N` for one, `N-` for the rest) would be
/// misread in the one place a typo is silent.
///
/// **A range applies to every file of a batch**, which only makes sense when the
/// files resemble one another. That is a limit and not an error: a batch of
/// different files has no use for a range, and the bound each file is judged
/// against is its own.
[[nodiscard]] std::expected<Range, std::string> parseRange(std::string_view text);

/// The subtitles of a file of `count` subtitles that `range` names, or why it
/// names none.
///
/// **A bound the file does not have is refused, and says which one** — the end
/// of the file being the answer a user needs to correct the line — **before
/// anything is applied**: a range that reaches past the file is a mistake about
/// the file, and applying it to what exists would answer another question.
[[nodiscard]] std::expected<subedit::core::Selection, std::string> selectionOf(const Range& range,
                                                                               std::size_t count);

} // namespace subedit::cli
