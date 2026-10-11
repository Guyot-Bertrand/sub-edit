#pragma once

// Joining the subtitles that repeat each other, and the command that does that,
// sorts and makes the ranges disjoint in one go.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/edit/disjoint_ranges.hpp>
#include <subedit/core/edit/join_repeats.hpp>
#include <subedit/core/model/encoding.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
class Project;
} // namespace subedit::core

namespace subedit::cli {

class Destination;
class Reporter;

/// Joins the repeated subtitles of a bare project that has no history, and says
/// what it did — what `--join-repeats` does on the commands that carry it.
[[nodiscard]] subedit::core::JoinReport joinedInPlace(subedit::core::Project& project);

/// What `--join-repeats` says about one file, at the first level.
[[nodiscard]] std::string narrationOfJoin(const std::string& path,
                                          const subedit::core::JoinReport& report);

/// Joins the repeated subtitles of every path, and says how many were.
///
/// A repeat is a subtitle with the text, the translation and the extras of
/// another that it touches or overlaps: a text cut in pieces that follow each
/// other. Independent of the order of the file, which it does not change.
[[nodiscard]] ExitCode joinAll(subedit::core::FileSystem& files,
                               const std::vector<std::string>& paths,
                               const std::optional<subedit::core::Encoding>& reading,
                               const Destination& destination,
                               const Reporter& reporter);

/// Does, on every path, what the three commands do one after the other: joins
/// the repeated subtitles, puts the rest in order of their start, and makes the
/// ranges disjoint with `settings`. In that order, which is the only one that
/// works: the order is read by the cut, and the repeats are best gone before
/// either.
[[nodiscard]] ExitCode repairAll(subedit::core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 const subedit::core::DisjointSettings& settings,
                                 const Destination& destination,
                                 const Reporter& reporter);

} // namespace subedit::cli
