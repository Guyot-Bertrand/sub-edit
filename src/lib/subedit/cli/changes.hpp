#pragma once

// The text a subcommand would change, subtitle by subtitle.
//
// What `--dry-run` shows and what `changes` of a JSON record carries: one
// list, read by both, so that the text and the JSON cannot say two things
// (ADR 0040).

#include <subedit/cli/json.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>

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

/// The text of `document` for every subtitle, in order — set aside **before** a
/// command runs, so that what it changed can be told afterwards.
[[nodiscard]] std::vector<std::string> textsOf(const subedit::core::Project& project,
                                               subedit::core::Document document);

/// What a command did to the texts, subtitle by subtitle.
///
/// Read from the command rather than found by comparing: `describe()` names the
/// subtitles rewritten and the ones taken away, and `before` — from
/// `textsOf`, taken before the command ran — gives the text each had.
/// **The indices are those of the project before the command ran**: the
/// rewrites move none, and a removal names the ones it takes — so a text that
/// stays is read back from `after` as many places up as subtitles went before
/// it. `document` is the one the command was built for, and the one `before`
/// was read from.
[[nodiscard]] std::vector<TextChange>
changesOfCommand(const subedit::core::Project& after,
                 const std::vector<std::string>& before,
                 const std::vector<subedit::core::Change>& described,
                 subedit::core::Document document = subedit::core::Document::Main);

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
