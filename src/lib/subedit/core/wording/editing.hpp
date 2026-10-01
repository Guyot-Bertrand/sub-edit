#pragma once

// What the tool says of an editing operation: what it left past the end of the
// video, what an alignment or an adjustment gave up, and why a pattern cannot
// be searched for. Shared wording of `core/wording/`; see `formats.hpp` for
// why it lives in the core.

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/video_bounds.hpp>

#include <cstddef>
#include <string>

namespace subedit::core {

struct PatternError;
struct SacrificedConstraints;

/// What an operation left past the end of the video, in one sentence.
///
/// Says the two things decision D4 asks for and nothing else: how many
/// subtitles reach past the end, and by how much the furthest of them does.
///
/// **It names the operation**, which is the whole reason it takes a kind.
/// The operations that can overshoot do it in different ways, and a notice that
/// did not say which one had just run would leave the user to guess.
///
/// A notice and not a refusal: nothing was prevented, and the sentence is
/// written to be read after the fact.
[[nodiscard]] std::string noticeOf(CommandKind kind, BeyondEnd beyond);

/// What an alignment of part of a file left the document saying, in one
/// sentence.
///
/// **It says the scope and the gap, and neither on its own would do** — issue
/// #324. The scope, because the table shows five rows moving and the user asked
/// for an alignment, with nothing tying the two together. The gap, because that
/// is the news: the document is still read on another grid, which is why the
/// status bar and the analysis did not move, and why they were taken for a
/// refresh that had failed.
///
/// A notice and not a refusal, as `noticeOf` above: nothing was prevented, and
/// the sentence is written to be read after the fact. It says what is, not what
/// should have been — aligning part of a file is a thing one may mean to do.
[[nodiscard]] std::string noticeOf(PartialAlignment partial);

/// What an adjustment of durations did, and what it gave up.
///
/// **What was sacrificed is said, post by post** — ADR 0008, and the one thing
/// the phase adds to Gaupol, which violates in silence. Each constraint appears
/// only when some subtitle could not satisfy it once the others had had their
/// say.
[[nodiscard]] std::string noticeOfAdjustment(std::size_t adjusted,
                                             const SacrificedConstraints& sacrificed);

/// Why a pattern cannot be searched for, in the words the dialog shows.
[[nodiscard]] std::string reasonOf(const PatternError& error);

} // namespace subedit::core
