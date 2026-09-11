#pragma once

#include <subedit/core/format/degradation.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/save_error.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace subedit::core {

class FileSystem;

/// A file that was read: what it holds, and what was wrong with it.
struct OpenedFile {
    Project project{};

    /// What the reading recovered from, in the order it was met.
    ///
    /// **Carried out rather than dropped.** Issue #128 left them behind, on the
    /// grounds that showing them was the window's business — which is true, and
    /// which is exactly why the window has to be handed them. They are the
    /// first thing of a reading a user sees other than through `-vvv`.
    std::vector<Diagnostic> diagnostics{};

    /// What the file weighed, for the reports that say so.
    ///
    /// Here rather than left to the caller because the caller no longer holds
    /// the bytes: `subedit-cli -vvv` writes « 4210 bytes read », and that line
    /// would have been the one thing lost by handing the reading over.
    std::size_t bytes = 0;
};

/// Reads `path` and builds what a window or a command opens on.
///
/// **The one recipe, and it used to be written four times.** The bytes are
/// read, the format is recognised, and the project keeps what it takes to write
/// the file back as it was — its format among the rest, since ADR 0018. Four
/// call sites chained the same three calls, and only one of them carried a
/// comment saying why they go in that order.
///
/// Here rather than in `gui/` or in `cli/` because it knows neither: what it
/// knows is the file system and the formats, which are both the core's. See
/// ADR 0025 for why `format/` rather than `io/`.
///
/// Fails rather than opening something wrong, and **says which of the two steps
/// failed**: a path the system will not give up is not a file whose bytes are
/// not subtitles.
[[nodiscard]] std::expected<OpenedFile, OpenError> openProject(const FileSystem& files,
                                                               const std::filesystem::path& path);

/// Reads it under what the caller knows and the file does not say.
///
/// **The overload the two others are short forms of** — an encoding, a frame
/// rate, or neither. It exists because a `.sub` counted in frames means nothing
/// until someone names a rate, and the one who knows it is upstream of every
/// reading.
[[nodiscard]] std::expected<OpenedFile, OpenError> openProject(const FileSystem& files,
                                                               const std::filesystem::path& path,
                                                               const ReadingChoices& choices);

/// Reads `path` in the encoding given, rather than in the one it proposes.
///
/// What `--encoding` asks for: a detection is a proposal, and Latin-1 and
/// CP1252 are the same bytes on most texts — the user is the one who may know.
///
/// **A mark still wins**, and the reading says so: a file that declares its
/// encoding is not read against its own word. See `readSubtitles`.
[[nodiscard]] std::expected<OpenedFile, OpenError>
openProject(const FileSystem& files, const std::filesystem::path& path, const Encoding& encoding);

/// Writes `project` to `path`, as a file of `format`.
///
/// **Puts the file back as it was found**: the line endings, the byte order
/// mark and the header are those the project came with, so that a file opened
/// and saved without a change is identical byte for byte. Anything else would
/// show a diff on every line where the user expected one corrected subtitle.
///
/// `format` is given rather than read from the project because « save as » may
/// change it; saving plainly passes the one the project already carries.
///
/// The writing is atomic — a save interrupted at any point leaves the previous
/// version exactly as it was, which is `writeAtomically`'s whole purpose.
///
/// **Neither the project nor the session is touched.** Recording that a
/// document was saved is `Session::markSaved`, and moving it is
/// `Session::setSourceFile`; both belong to whoever holds the session.
///
/// **Says which of the two steps failed**, as `openProject` does: subtitles
/// that hold a character the encoding cannot write are not a disk that refuses
/// them, and the user can act on only one of the two.
/// A document turned into what another format will hold, and what that cost.
///
/// **The one place that decides what crosses a format boundary.** Three rules
/// live here and nowhere else: the markup goes into the arriving vocabulary
/// (ADR 0031), the header and the fields a file declared cross only into their
/// own format (ADR 0030), and a file written in frames carries the rate it was
/// counted at. The measure of what will not fit comes out of the same walk.
///
/// **It exists because it was written twice.** Until issue #364 the command
/// line and the window each carried these rules, with two different answers to
/// « which rate », and `saveProject` carried none of them — so a third caller
/// would have written a `.ssa` still saying `<i>`, which is a file the format
/// never interprets. That is the salissure ADR 0031 was written to end, and it
/// was reachable through the door next to it.
struct ConvertedProject {
    std::vector<Subtitle> subtitles{};

    /// The free header, kept only when the format has not changed.
    std::string header{};

    /// What the file declares that no subtitle carries, for a file of `target`.
    FileExtras extras{};

    ConversionLoss loss{};
};

/// Turns `project` into what `target` will hold, counting in `rate` for frames.
///
/// **`rate` is asked for rather than read**, because the two surfaces answer it
/// differently and both are right: the window uses the document's own, which it
/// always has; the command line deduces a grid and **refuses** rather than
/// invent one, which needs a reporter and an exit code. What is common is what
/// happens once the number is known, and that is what this does.
[[nodiscard]] ConvertedProject
convertProjectFor(const Project& project, SubtitleFormat target, FrameRate rate);

/// **Converts on the way out**, through `convertProjectFor`, counting frames at
/// the rate the project works against. A caller that wants to know what the
/// conversion cost — to report it, or to ask before writing — calls
/// `convertProjectFor` itself and hands the result back through the project.
[[nodiscard]] std::expected<void, SaveError> saveProject(FileSystem& files,
                                                         const Project& project,
                                                         const std::filesystem::path& path,
                                                         SubtitleFormat format);

} // namespace subedit::core
