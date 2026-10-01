#pragma once

// How the tool names things to the user — formats, line endings, encodings and
// the reasons a file could not be read, opened, written or saved.
//
// Shared rather than repeated: the command line and the window both name
// formats, line endings, failures and operations, and two copies of that
// vocabulary would drift apart — the same file would be "SubRip" in one report
// and "SRT" in the next.
//
// **The headers under `core/wording/` are the one place in the core that carry
// words meant for a reader**, and they say so in their name. The core is
// otherwise free of presentation, and these files are the exception rather
// than the crack: naming what the enumerations mean has to happen somewhere,
// both surfaces need the same names, and neither surface may depend on the
// other.
//
// **One header per family, and a family is a set of readers.** A single header
// used to carry all of it, so a word changed for one surface recompiled every
// unit that said any word at all (issue #549). Each header includes only what
// its own signatures need, and a change to a family now costs its readers only.
//
// English, like everything the tool prints. Translation is a phase of its own;
// these files are where it will have to reach.

#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/save_error.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// The name of a format, as a report writes it.
[[nodiscard]] std::string_view nameOf(SubtitleFormat format);

/// The short name a command line uses for that format — the values of `--to`.
///
/// **Not the extension, and the reason is that two extensions name two formats
/// each.** `.sub` is MicroDVD and SubViewer 2, `.txt` is MPL2 and TMPlayer, so
/// an option taking extensions could not say which was meant. These names are
/// unambiguous by construction, and coincide with the extension wherever the
/// extension happens to be unambiguous.
[[nodiscard]] std::string_view optionNameOf(SubtitleFormat format);

/// The format that short name means, or nothing if it names none.
[[nodiscard]] std::optional<SubtitleFormat> formatNamed(std::string_view option);

/// The extension a file of that format is expected to carry, dot included.
[[nodiscard]] std::string_view extensionOf(SubtitleFormat format);

/// The decimal mark a file of that format carries between seconds and their
/// fraction.
///
/// **SubRip is the only one of the nine that writes a comma**, and everything
/// else that writes a fraction at all writes a point — WebVTT, SubViewer 2, the
/// two Sub Station Alpha, TMPlayer, LRC. MicroDVD counts in frames and MPL2 in
/// tenths inside brackets, so neither has a mark of its own; the point is what
/// a surface shows for them, being the answer of the other seven.
///
/// **It exists because a surface has to agree with a writer.** The table of the
/// window shows a position before the file holds it, and it promises to show
/// what will be written; asking each writer would be asking eight questions,
/// and testing WebVTT alone — which is what it did until phase 9 — answered
/// « comma » for six formats that write a point.
[[nodiscard]] DecimalMark decimalMarkOf(SubtitleFormat format);

/// The name of a line ending, as a report writes it.
[[nodiscard]] std::string_view nameOf(Newline newline);

/// An encoding and its mark, as a report writes them: "UTF-8, no BOM".
///
/// **The mark is said in words rather than glued to the name.** `Encoding` used
/// to answer `UTF-8-sig` — Python's own spelling, which Gaupol shows — and
/// `UTF-16LE-sig`, which is nobody's: Python has no name for the UTF-16 pair,
/// it detects their mark instead. A name invented here is a name a reader would
/// copy into a search, a bug report, or an `--encoding`, where nothing knows it.
///
/// The shape is the one the command line already wrote at level 2, in three
/// places that spelled it out separately. It is written once now, and the
/// window says the same thing — issue #315.
[[nodiscard]] std::string nameOf(const Encoding& encoding);

/// What the status bar says of the encoding a document was read in.
///
/// Beside what it already says of the grid and of the film, and for the same
/// reason: it is a standing fact about the document, not an event of its
/// reading.
///
/// **It does not say where the answer came from**, where `inspect` does —
/// `detected`, `from its byte order mark`, `as asked for`. That belongs to the
/// panel under the table, which exists to say what the reading *did*; this line
/// says what the document *is*. A guessed encoding is worth a diagnostic, and
/// it has one.
[[nodiscard]] std::string encodingStatusOf(const Encoding& encoding);

/// Why a name did not become an encoding, as a whole sentence naming it.
///
/// **A sentence and not a clause**, unlike its neighbours here: the two answers
/// put the name in different places — `no encoding is named "klingon-1"` ends
/// with it, and the other one has to start with it — so a caller could not
/// assemble either from a fragment.
///
/// The second answer names the way out rather than only the refusal. A user who
/// typed `UTF-16` wants UTF-16; being told that it exists and is refused, with
/// nothing further, would be the worst of both.
[[nodiscard]] std::string refusalOf(EncodingRefusal refusal, std::string_view name);

/// Why a file could not be read at all, in the second half of a sentence
/// starting with its path.
[[nodiscard]] std::string_view reasonOf(ReadErrorKind kind);

/// Why the file system refused to **read**, in the same shape.
///
/// Reading only: a refusal to write is `reasonOfWriting`. They used to be one
/// function, and a disk that refused a write was reported as "cannot be read".
[[nodiscard]] std::string_view reasonOf(FileErrorKind kind);

/// Why the file system refused to **write**, in the same shape.
///
/// `cannot be written: permission denied`, or `cannot be written` for the rest.
/// A missing directory says no more than the rest: the output directory is
/// created before anything is written, so a write that finds none is not the
/// ordinary case.
[[nodiscard]] std::string_view reasonOfWriting(FileErrorKind kind);

/// Why a directory could not be made, in the same shape.
[[nodiscard]] std::string_view reasonOfCreating(FileErrorKind kind);

/// Why a file could not be opened, whichever of the two steps failed.
///
/// **One call for both surfaces**, and it is what the window was missing: it
/// used to have one sentence for four causes, and the only one it was right
/// about was the fourth. Nothing new is worded here — the two overloads above
/// already say all seven — only the choice between them is made in one place
/// rather than at each call site.
[[nodiscard]] std::string_view reasonOf(const OpenError& error);

/// Why subtitles could not be written, in the same shape.
[[nodiscard]] std::string_view reasonOf(WriteErrorKind kind);

/// Why a file could not be saved, whichever of the two steps failed.
///
/// The mirror of the call above, and it exists for the same reason: the window
/// and the command line say the same words about the same failure, and neither
/// chooses between the two overloads on its own.
[[nodiscard]] std::string_view reasonOf(const SaveError& error);

} // namespace subedit::core
