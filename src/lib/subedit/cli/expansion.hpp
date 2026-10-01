#pragma once

// Turning what was named on the command line into the files a batch works on.

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

/// The files of a batch, and what it took to find them.
struct Inputs {
    /// In the order the walk found them: the names as given, and each
    /// directory's files where the directory stood.
    std::vector<std::string> paths;

    /// The directories that were walked, to be handed to `Destination::withRoots`.
    std::vector<std::filesystem::path> roots;
};

/// Settles the inputs: a file is itself, a directory is what is in it.
///
/// **A directory without `recursive` is a usage error**: refusing it is better
/// than processing nothing and returning `0`.
///
/// **With `recursive`, the walk is deterministic and holds no surprise**:
///
/// - the entries of a directory are taken in the order of their names, bytes
///   compared, and a subdirectory is walked where it stands — depth first;
/// - symbolic links are not followed, so that no tree loops;
/// - hidden entries — a name starting with a dot — are ignored;
/// - only a file with the extension of a known format is kept: `.srt`, `.vtt`,
///   `.ssa`, `.ass`, `.lrc`, `.sub`. **`.txt` is out**: it names two formats,
///   and above all every `README.txt` there is — a batch that failed on each
///   would be of no use. What was left aside is counted, and said at level 2;
/// - the output directory, when it lies in the tree, is not walked: a second
///   run must not read what the first one wrote.
///
/// **A file named on the command line is never filtered**: naming it is wanting
/// it.
///
/// The list is **complete before the first write**, which is what makes the
/// collisions of `Destination::plan` decidable and the walk independent of
/// what the writing changes.
[[nodiscard]] std::expected<Inputs, std::string> expandInputs(const core::FileSystem& files,
                                                              const std::vector<std::string>& given,
                                                              bool recursive,
                                                              std::string_view outputDir,
                                                              const Reporter& reporter);

} // namespace subedit::cli
