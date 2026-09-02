#pragma once

// Writing subtitles back out, in the format and the shape asked for.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// What the bytes of the written file look like, beyond their format.
///
/// Both are optional, and **empty means "as the source had it"**. The model
/// records the line ending and the encoding of the file it read, mark included;
/// imposing either by default would lose, at every conversion, information the
/// core took care to keep.
struct WriteShape {
    std::optional<subedit::core::Newline> newline{};
    std::optional<subedit::core::ByteOrderMark> bom{};
};

/// Whether writing `target` back over these paths would leave a file misnamed.
///
/// Decided on the **extension alone**, and deliberately: a usage error has to
/// be caught before anything is read, and the extension is both what the caller
/// sees and what would end up lying. A file whose content is already WebVTT but
/// whose name says `.srt` is not this function's problem — `inspect` is.
[[nodiscard]] bool wouldMisname(const std::vector<std::string>& paths,
                                subedit::core::SubtitleFormat target);

/// Converts every path into `target`, and says how it went.
///
/// Each file is independent: the failure of one does not stop the others.
[[nodiscard]] ExitCode convertAll(subedit::core::FileSystem& files,
                                  const std::vector<std::string>& paths,
                                  subedit::core::SubtitleFormat target,
                                  const WriteShape& shape,
                                  const Destination& destination,
                                  const Reporter& reporter);

} // namespace subedit::cli
