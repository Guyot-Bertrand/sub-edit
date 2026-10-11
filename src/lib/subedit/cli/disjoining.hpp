#pragma once

// Making the ranges of the subtitles of a file disjoint.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/records.hpp>
#include <subedit/core/edit/disjoint_ranges.hpp>
#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
class Project;
class Session;
} // namespace subedit::core

namespace subedit::cli {

class Destination;
class Reporter;

/// What `disjoint` was asked for: the three settings of the operation, and
/// whether to put the subtitles in order first.
struct DisjointOptions {
    std::string minimum{};
    std::string maxLines{};
    bool dashes = false;
    bool sort = false;
};

/// The settings the options spell out, or why they do not make sense.
[[nodiscard]] std::expected<subedit::core::DisjointSettings, std::string>
settingsOf(const DisjointOptions& options);

/// Makes the ranges of every path disjoint, and says what it did.
///
/// **The subtitles must be in order**, since the groups are runs of neighbours:
/// a file that is not fails with `not-in-order` unless `sort` asked for the
/// order first. The core never sorts of itself (ADR 0012).
[[nodiscard]] ExitCode disjointAll(subedit::core::FileSystem& files,
                                   const std::vector<std::string>& paths,
                                   const std::optional<subedit::core::Encoding>& reading,
                                   const subedit::core::DisjointSettings& settings,
                                   bool sort,
                                   const Destination& destination,
                                   const Reporter& reporter);

/// Makes the ranges of the project of `session` disjoint **through the
/// history**, with the settings the interfaces default to — what `--disjoint`
/// does on the commands that carry it.
[[nodiscard]] std::expected<subedit::core::DisjointReport, Failure>
disjointedIn(subedit::core::Session& session);

/// The same, on a bare project that has no history.
[[nodiscard]] std::expected<subedit::core::DisjointReport, Failure>
disjointedInPlace(subedit::core::Project& project);

/// What `--disjoint` says about one file, at the first level.
[[nodiscard]] std::string narrationOfDisjoint(const std::string& path,
                                              const subedit::core::DisjointReport& report);

/// Every count of the operation, for the command that is about it.
[[nodiscard]] std::vector<Count> fullCountsOfDisjoint(const subedit::core::DisjointReport& report);

/// Adds the counts of `cut` to `counts`, when the option was given.
void addCountsOfDisjoint(std::vector<Count>& counts,
                         const std::optional<subedit::core::DisjointReport>& cut);

/// The counts `--disjoint` adds to a record: only with the option, so that no
/// existing script finds a field it did not know.
[[nodiscard]] std::vector<Count> countsOfDisjoint(const subedit::core::DisjointReport& report);

} // namespace subedit::cli
