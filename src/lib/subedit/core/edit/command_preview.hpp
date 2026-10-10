#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <vector>

namespace subedit::core {

/// One subtitle a command would change: how it stands, and how it would stand.
struct PreviewedChange {
    SubtitleIndex index = SubtitleIndex::fromValue(0);
    Subtitle before;
    Subtitle after;
};

/// What a command would do, without doing it.
struct CommandPreview {
    /// The first changes, in file order, up to the limit asked for.
    std::vector<PreviewedChange> shown;

    /// How many subtitles the command would change in all.
    std::size_t changed = 0;
};

/// Runs `command` on a **copy** of `project` and says what moved.
///
/// The project itself is untouched, and so is the command: it is applied to the
/// copy and nothing else, so the same command can be applied for real
/// afterwards. A copy rather than a dry-run mode on every command, because the
/// commands are one thing and the question « what would it change? » is the
/// same for all of them: compare before and after.
///
/// Only commands that keep the number of subtitles are meaningful here — the
/// positions of the dialogs that offer a preview. Past the shorter of the two
/// lists nothing is compared.
[[nodiscard]] CommandPreview previewOf(const Project& project, Command& command, std::size_t limit);

} // namespace subedit::core
