#pragma once

// Replacing a text in the subtitles of a file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/core/config/search_options.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Compiles `pattern`, or says why it cannot be — in the words of the window.
///
/// **Read once and before any file is touched**: a pattern that cannot be read
/// is a mistake about the command line and not about a file, and a batch must
/// not be started to find that out on each of its files. The reason is
/// `reasonOf(PatternError)`, the one the search dialog shows.
[[nodiscard]] std::expected<subedit::core::SearchPattern, std::string>
compilePattern(std::string_view pattern, subedit::core::SearchOptions options);

/// Replaces every match of `pattern` by `replacement` in every path, and says
/// how it went.
///
/// **It looks in the visible text and writes into the stored one, without
/// breaking a tag** — the rule of the phase-10 search, that the window applies
/// to the same command. `range` limits the subtitles looked in, in every file.
/// With a `pairing` it is the translation that is looked in and written
/// (`rewriteAll`).
///
/// A file where nothing matched is written all the same — a destination given
/// is a destination written — and says `"…" not found`; one where every match is
/// replaced by itself says `nothing to change`. Neither is a failure.
[[nodiscard]] ExitCode replaceIn(subedit::core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 const subedit::core::SearchPattern& pattern,
                                 std::string_view patternText,
                                 std::string_view replacement,
                                 const std::optional<Range>& range,
                                 const Destination& destination,
                                 const Reporter& reporter,
                                 const std::optional<Pairing>& pairing = std::nullopt);

} // namespace subedit::cli
