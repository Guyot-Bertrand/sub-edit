#include <subedit/core/transcription/annotation_notes.hpp>

#include <algorithm>

namespace subedit::core {

namespace {

constexpr std::size_t kMinimumNotes = 2;
constexpr std::size_t kShareDenominator = 3;

[[nodiscard]] char closerOf(char opener) {
    return opener == '[' ? ']' : ')';
}

} // namespace

bool isNotesOnly(std::string_view text) {
    std::size_t at = 0;
    bool any = false;
    while (at < text.size()) {
        if (text[at] == ' ' || text[at] == '\t' || text[at] == '\n') {
            ++at;
            continue;
        }
        if (text[at] != '[' && text[at] != '(')
            return false;
        const std::size_t close = text.find(closerOf(text[at]), at + 1);
        if (close == std::string_view::npos || close == at + 1)
            return false;
        any = true;
        at = close + 1;
    }
    return any;
}

NotesShare notesShare(std::span<const TranscribedSegment> segments) {
    return NotesShare{
        .notes = static_cast<std::size_t>(std::ranges::count_if(
            segments, [](const TranscribedSegment& segment) { return isNotesOnly(segment.text); })),
        .pieces = segments.size()};
}

bool looksLikeWrongLanguage(const NotesShare& share) {
    return share.notes >= kMinimumNotes && share.notes * kShareDenominator >= share.pieces;
}

} // namespace subedit::core
