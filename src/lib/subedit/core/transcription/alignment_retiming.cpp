#include <subedit/core/transcription/alignment_retiming.hpp>

#include <cstddef>

namespace subedit::core {

std::vector<RetimedLine> retime(std::span<const LineTimes> originals,
                                std::span<const AlignedLine> places) {
    std::vector<RetimedLine> lines;
    lines.reserve(originals.size());
    for (std::size_t line = 0; line < originals.size(); ++line) {
        RetimedLine made{.times = originals[line]};
        if (line < places.size() && places[line].start && places[line].end) {
            made.times = {.start = places[line].start.value_or(originals[line].start),
                          .end = places[line].end.value_or(originals[line].end)};
            made.moved = true;
        }
        lines.push_back(made);
    }

    for (std::size_t line = 0; line + 1 < lines.size(); ++line) {
        RetimedLine& current = lines[line];
        const Timestamp next = lines[line + 1].times.start;
        if (current.times.end <= next)
            continue;
        if (next > current.times.start) {
            current.times.end = next;
            current.trimmed = true;
        } else {
            current.overlapping = true;
        }
    }
    return lines;
}

} // namespace subedit::core
