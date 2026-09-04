#pragma once

// Reporting what a subtitle file is made of, without changing it.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/model/encoding.hpp>

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

/// Reads `path` and writes its report to `out`.
///
/// The report is the result of the subcommand, which is why it goes to
/// standard output and is written whatever the verbosity: silencing it would
/// leave `inspect` with nothing to say.
///
/// Returns true when the file could be read. What went wrong, when it could
/// not, goes through `reporter` — never mixed into the report, so that a
/// caller piping the result never receives a complaint where it expected data.
[[nodiscard]] bool inspectFile(const subedit::core::FileSystem& files,
                               const std::string& path,
                               const std::optional<subedit::core::Encoding>& reading,
                               std::ostream& out,
                               const Reporter& reporter);

/// Reports on every path, and says how it went.
///
/// Each file is independent: the failure of one does not stop the others. The
/// point of taking a batch is to learn about all of it in one go, and stopping
/// at the first failure would mean re-running once per faulty file to get the
/// list.
[[nodiscard]] ExitCode inspectAll(const subedit::core::FileSystem& files,
                                  const std::vector<std::string>& paths,
                                  const std::optional<subedit::core::Encoding>& reading,
                                  std::ostream& out,
                                  const Reporter& reporter);

} // namespace subedit::cli
