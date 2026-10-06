#pragma once

// Opening the file a subcommand works on, and saying why when it cannot.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/model/encoding.hpp>

#include <optional>
#include <string>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

/// Opens `path`, **or reports its failure and gives nothing**.
///
/// One place for the step every subcommand begins with — `openProject`, then the
/// failure said as `path: <reason>` with its stable identifier and, in JSON, its
/// record. `reading` is either the encoding the caller named or the two choices
/// of `--encoding` and `--frame-rate`; with none, the encoding is detected.
///
/// The caller returns its own failure code: what a failed open means differs
/// between a file of a batch and the only file of the run.
[[nodiscard]] std::optional<subedit::core::OpenedFile>
openReporting(const subedit::core::FileSystem& files,
              const std::string& path,
              const std::optional<subedit::core::Encoding>& reading,
              const Reporter& reporter);

[[nodiscard]] std::optional<subedit::core::OpenedFile>
openReporting(const subedit::core::FileSystem& files,
              const std::string& path,
              const subedit::core::ReadingChoices& reading,
              const Reporter& reporter);

} // namespace subedit::cli
