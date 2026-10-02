#pragma once

// The text a subcommand would change, subtitle by subtitle.
//
// What `--dry-run` shows and what `changes` of a JSON record carries: one
// list, read by both, so that the text and the JSON cannot say two things
// (ADR 0040).

#include <subedit/cli/json.hpp>
#include <subedit/core/model/document.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::cli {

/// One subtitle whose text an operation changes, or takes away.
struct TextChange {
    /// The number the subtitle has in the file, counting from one — the one
    /// the window shows, and the one a person looks for.
    std::size_t subtitle = 0;
    subedit::core::Document document = subedit::core::Document::Main;
    std::string before;

    /// What the text becomes; nothing when the subtitle is removed.
    std::optional<std::string> after{};
};

/// What a run was asked to compute besides its counts.
///
/// **A list of changes costs a copy of every text that changes**, and a batch of
/// thousands of subtitles should not build one for nothing: an operation makes it
/// only when this says someone reads it.
struct Wants {
    bool changes = false;
};

/// The `changes` of a record: an array of `{subtitle, document, before, after}`,
/// `after` being `null` for a subtitle that is removed.
[[nodiscard]] Json changesOf(const std::vector<TextChange>& changes);

/// The changes as a person reads them, for the standard output of a dry run.
///
/// One block per subtitle — its place, then the text before with `- ` in front
/// of each line, then the text after with `+ ` — because a text of several lines
/// is the case that decides the form: a line-by-line diff would cut it where it
/// likes, and a one-line form could not hold it. A removed subtitle says so in
/// its first line and has no `+` lines. Each line of the block begins with
/// nothing the text itself could begin with except `- ` and `+ `, and the first
/// begins with the file: `grep '^film.srt: '` finds the blocks of one file.
[[nodiscard]] std::string textOf(std::string_view file, const std::vector<TextChange>& changes);

} // namespace subedit::cli
