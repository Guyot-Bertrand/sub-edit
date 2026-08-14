#pragma once

// The one way the command line names a subtitle, and pairs it with a position.

#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <expected>
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

} // namespace subedit::cli
