#pragma once

// Writing subtitles back out, in the format and the shape asked for.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <expected>
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

    /// The encoding to write in, the source's when empty.
    ///
    /// **The one of the three that can refuse the file**: a line ending and a
    /// mark can be put on any text, an encoding cannot carry every character.
    /// What then happens is a failure that names the character — never a `?`
    /// written in its place.
    std::optional<subedit::core::Encoding> encoding{};
};

/// The shape a conversion is asked to write in, from the options as they were written:
/// the line endings (`unix`, `windows`, `mac`), the encoding, and `--bom` / `--no-bom`.
/// Empty strings and absent flags mean « as the source had it ».
///
/// Refused, naming the options, when `--bom` and `--no-bom` are both given and when the
/// encoding names nothing — a mistake about the command line, said before a file is read.
[[nodiscard]] std::expected<WriteShape, std::string>
writeShapeOf(const std::string& lineEndings, const std::string& encoding, bool bom, bool noBom);

/// The refusal of `--in-place` when it would leave a file misnamed, and nothing when it
/// would not. **Refused rather than obeyed**: in place there is no second name to carry the
/// new format, and the file would be left under an extension its content no longer
/// justifies.
[[nodiscard]] std::optional<std::string> refusalOfInPlaceRename(
    bool inPlace, const std::vector<std::string>& paths, subedit::core::SubtitleFormat target);

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
///
/// **`sort` puts the subtitles in order of their start before writing**, on a copy:
/// what is written is in order whatever the format, and the file read is untouched.
/// Opt-in and never a default — the order of a file is its author's until asked —
/// and it says how many subtitles moved.
[[nodiscard]] ExitCode convertAll(subedit::core::FileSystem& files,
                                  const std::vector<std::string>& paths,
                                  const subedit::core::ReadingChoices& reading,
                                  subedit::core::SubtitleFormat target,
                                  const WriteShape& shape,
                                  const Destination& destination,
                                  const Reporter& reporter,
                                  bool sort = false);

} // namespace subedit::cli
