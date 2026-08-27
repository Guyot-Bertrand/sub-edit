#pragma once

// How the tool names things to the user.
//
// Shared rather than repeated: the command line and the window both name
// formats, line endings, failures and operations, and two copies of that
// vocabulary would drift apart — the same file would be "SubRip" in one report
// and "SRT" in the next.
//
// **The one place in the core that carries words meant for a reader**, and it
// says so in its name. The core is otherwise free of presentation, and this
// file is the exception rather than the crack: naming what the enumerations
// mean has to happen somewhere, both surfaces need the same names, and neither
// surface may depend on the other.
//
// English, like everything the tool prints. Translation is a phase of its own;
// this file is where it will have to reach.

#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/analysis/grid_verdict.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/video_bounds.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// The name of a format, as a report writes it.
[[nodiscard]] std::string_view nameOf(SubtitleFormat format);

/// The extension a file of that format is expected to carry, dot included.
[[nodiscard]] std::string_view extensionOf(SubtitleFormat format);

/// The name of a line ending, as a report writes it.
[[nodiscard]] std::string_view nameOf(Newline newline);

/// Why a file could not be read at all, in the second half of a sentence
/// starting with its path.
[[nodiscard]] std::string_view reasonOf(ReadErrorKind kind);

/// Why the file system refused, in the same shape.
[[nodiscard]] std::string_view reasonOf(FileErrorKind kind);

/// What a reader ran into, as a report writes it.
///
/// A phrase and not a sentence: it is the middle of a line that already names
/// the file and the line number, and it is followed by what was done about it.
[[nodiscard]] std::string_view nameOf(DiagnosticKind kind);

/// What is wrong with a document, in one clause.
///
/// Written to follow « subtitle 12 » — the subject is the subtitle, so the
/// clause starts with its verb. An anomaly names a subtitle where a diagnostic
/// names a line, which is the whole of the distinction ADR 0018 draws.
[[nodiscard]] std::string_view nameOf(AnomalyKind kind);

/// What was done about an anomaly, as a report writes it.
///
/// The distinction the core draws, said out loud: one of the two was settled
/// and needs nobody, the other was left alone because only the user can decide.
[[nodiscard]] std::string_view nameOf(Severity severity);

/// What an operation is, as an undo action names it.
///
/// A phrase and not a sentence: the window puts « Undo: » in front of it, and
/// what goes in front is the window's business.
[[nodiscard]] std::string_view nameOf(CommandKind kind);

/// A frame rate, as a report writes it: "25", "23.9", "24000/1001".
///
/// **A decimal only when a decimal is exact.** A rate whose denominator divides
/// a thousand is written as one, trailing zeros trimmed; anything else is
/// written as its fraction. The NTSC rates fall on the second side, which is
/// the point: naming `24000/1001` "23.976" would report a conversion that did
/// not happen, and this line is the only place the user sees which rate was
/// actually used.
[[nodiscard]] std::string nameOf(FrameRate rate);

/// What the deduction concluded, in one word.
///
/// Here rather than in the report that first needed it: the window says the
/// same thing about the same file, and two surfaces wording one verdict twice
/// is how they start to disagree.
[[nodiscard]] std::string_view nameOf(GridVerdict verdict);

/// A concentration, as a percentage with one decimal.
///
/// One decimal and not more: the third digit of a concentration says nothing a
/// reader can act on, and a clean grid reads better as `99.9%` than as
/// `99.87342%`.
[[nodiscard]] std::string percentOf(double value);

/// What the status bar says of the grid the positions were written on.
///
/// Beside what it already says of the associated film, and worded here for the
/// same reason: it is a standing fact about the document, and the report of
/// `inspect` says it in the same words.
///
/// `rate` is the candidate retained, or nothing when the verdict is silent —
/// there is then no rate to name, which is the whole point of a closed set of
/// candidates.
[[nodiscard]] std::string gridStatusOf(GridVerdict verdict, std::optional<FrameRate> rate);

/// A length in seconds, signed, to the millisecond: "-7.001 s".
///
/// Here since #174, and it was in the command line before — where the window
/// could not reach it. Both surfaces now say a length the same way, which is
/// what this file is for.
[[nodiscard]] std::string secondsOf(Duration length);

/// What an operation left past the end of the video, in one sentence.
///
/// Says the two things decision D4 asks for and nothing else: how many
/// subtitles reach past the end, and by how much the furthest of them does.
///
/// **It names the operation**, which is the whole reason it takes a kind.
/// Shifting, transforming and converting a frame rate overshoot in three
/// different ways, and a notice that did not say which one had just run would
/// leave the user to guess between the three.
///
/// A notice and not a refusal: nothing was prevented, and the sentence is
/// written to be read after the fact.
[[nodiscard]] std::string noticeOf(CommandKind kind, BeyondEnd beyond);

/// What the window says of the film a document is watched against.
///
/// Its name, or that there is none — and its name alone, not its path: the
/// line sits in a status bar, where a path of two hundred characters would push
/// out everything else. Whoever wants the path has the chooser that named it.
/// What the status bar says of the associated film: its name and the rate it
/// declares, or that there is none.
///
/// **The rate goes with the film rather than beside it.** They are one fact —
/// what this document accompanies — and a third widget would put the film's own
/// cadence at the same rank as the grid deduced from the positions, which is a
/// different fact entirely.
///
/// `declared` is nothing when no film is open, or when `ffprobe` is absent: the
/// name is then said alone, as it was before the rate could be read.
[[nodiscard]] std::string videoStatusOf(const std::optional<std::filesystem::path>& video,
                                        std::optional<FrameRate> declared = {});

/// A count and its noun, agreeing: "1 subtitle", "2 subtitles".
///
/// Small, and worth it: "1 subtitles" is the kind of sloppiness a reader
/// notices immediately and that ends up copied into the manual, which is
/// generated from what the tool actually prints.
[[nodiscard]] std::string countOf(std::size_t count, std::string_view noun);

} // namespace subedit::core
