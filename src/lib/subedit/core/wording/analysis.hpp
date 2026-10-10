#pragma once

// How the tool names what it found in a document: diagnostics, anomalies,
// operations, frame rates and the verdict on a grid, and the standing facts
// the status bar states about the grid. Shared wording of `core/wording/`; see
// `formats.hpp` for why it lives in the core.

#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/analysis/grid_verdict.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

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

/// One anomaly as a sentence: « subtitle 12 starts before the previous one ends ».
///
/// The number is the one the table shows, counted from 1. Shared by the
/// report of the command line and the list of the window, which say the same
/// thing in the same words.
[[nodiscard]] std::string statementOf(const Anomaly& anomaly);

/// How many subtitles carry one kind of anomaly, as a line of a summary:
/// « subtitle starts before the previous one ends: 3 ».
///
/// The summary is what both surfaces show first; the sentences of
/// `statementOf` are the detail behind it.
[[nodiscard]] std::string summaryOf(AnomalyKind kind, std::size_t count);

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

/// The same place in the status bar, for a document counted in frames.
///
/// **It replaces the grid line rather than joining it**, and for the reason
/// `inspect` gives in the same case: the positions of a MicroDVD document were
/// computed *from* its frames at this very rate, so a grid deduced from them
/// can only find it again. Naming it a measurement would dress a given as a
/// finding.
///
/// What the file does not say is said by the reading, once, as a diagnostic.
/// This line says what is, permanently, as the grid's and the film's do.
[[nodiscard]] std::string framesStatusOf(FrameRate rate);

/// A length in seconds, signed, to the millisecond: "-7.001 s".
///
/// Here since #174, and it was in the command line before — where the window
/// could not reach it. Both surfaces now say a length the same way, which is
/// what this file is for.
[[nodiscard]] std::string secondsOf(Duration length);

} // namespace subedit::core
