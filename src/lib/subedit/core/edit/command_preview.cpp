#include <subedit/core/edit/command_preview.hpp>

#include <algorithm>
#include <cstddef>
#include <span>

namespace subedit::core {

CommandPreview previewOf(const Project& project, Command& command, std::size_t limit) {
    Project copy = project;
    command.apply(copy);

    const std::span<const Subtitle> before = project.subtitles();
    const std::span<const Subtitle> after = copy.subtitles();

    CommandPreview preview;
    const std::size_t common = std::min(before.size(), after.size());
    for (std::size_t position = 0; position < common; ++position) {
        if (before[position] == after[position])
            continue;

        ++preview.changed;
        if (preview.shown.size() < limit) {
            preview.shown.push_back(PreviewedChange{.index = SubtitleIndex::fromValue(position),
                                                    .before = before[position],
                                                    .after = after[position]});
        }
    }
    return preview;
}

} // namespace subedit::core
