#pragma once

// Cutting a file in two.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/model/encoding.hpp>

#include <cstddef>
#include <optional>
#include <string>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

/// What `split-file` was asked for, verbatim.
struct SplitRequest {
    /// The file to cut.
    std::string input;

    /// The first subtitle of the tail, **numbered as the table of the window
    /// numbers them**: from 1, so that the cut falls between `at - 1` and `at`.
    std::size_t at = 0;

    /// Where the two halves go. Both empty only for a dry run, which writes nothing.
    std::string head;
    std::string tail;

    bool dryRun = false;

    /// Puts the subtitles in order of their start before cutting, so that the cut
    /// falls on the time order and `at` counts in it. Opt-in, and it says how many
    /// subtitles moved.
    bool sort = false;
};

/// Cuts `request.input` at `request.at` and writes the two halves.
///
/// **The exact inverse of `appendAll`**, through the command the window drives
/// (`splitProject`): the tail is shifted back by the end of the last subtitle that
/// stays, so that appending it to the head gives the file back. Both halves are
/// written in the format, the encoding and the line endings of the input.
///
/// **The one subcommand with two outputs**, hence a destination of its own:
/// `--head` and `--tail`, both required unless the run is a dry one.
///
/// Refused as a usage error, before anything is read, when a destination is
/// the input or when the two are one file. Refused as a failure of the file — code
/// `2`, nothing written — when `at` is not a cut of this file, and when the two
/// halves overlap: the shift would carry a subtitle before the start of the video,
/// which no file can hold, and that subtitle is named.
///
/// The head is written first: a disk that refuses the tail leaves the head behind.
[[nodiscard]] ExitCode splitFile(subedit::core::FileSystem& files,
                                 const SplitRequest& request,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 const Reporter& reporter);

} // namespace subedit::cli
