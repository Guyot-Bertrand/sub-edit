#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// The opening of a translation, as the history holds it: the texts written and
/// the subtitles born, which are `inner`, and the file the translation came from.
///
/// **One command and not a group with the file in it**, because the file is not
/// a change of any subtitle and has nothing to say to `describe`: what makes a
/// document modified is the texts and the insertions, and they are `inner`'s.
/// The file is set when the command is applied, and the one that was there — or
/// none — is put back when it is reverted.
class AttachTranslationCommand final : public Command {
public:
    AttachTranslationCommand(const Project& project,
                             std::unique_ptr<Command> inner,
                             SourceFile after)
        : m_inner{std::move(inner)},
          m_before{project.translationFile()},
          m_after{std::move(after)} {}

    void apply(Project& project) override {
        m_inner->apply(project);
        project.setSourceFile(Document::Translation, m_after);
    }

    void revert(Project& project) override {
        m_inner->revert(project);
        if (m_before.has_value())
            project.setSourceFile(Document::Translation, *m_before);
        else
            project.clearTranslationFile();
    }

    [[nodiscard]] CommandKind kind() const override { return CommandKind::AttachTranslation; }

    [[nodiscard]] std::vector<Change> describe() const override { return m_inner->describe(); }

private:
    std::unique_ptr<Command> m_inner;
    std::optional<SourceFile> m_before;
    SourceFile m_after;
};

/// A line that found no subtitle, and the place it is to be inserted at: before
/// the subtitle of the main document that was next when it was met.
struct Born {
    std::size_t before;
    std::size_t line;
};

/// Which lines go to which subtitles, and which give birth to one.
struct Matching {
    /// For each subtitle of the main document, the line that reached it.
    std::vector<std::optional<std::size_t>> lineOf;

    /// The lines that found nothing, in the order they were met — which is the
    /// order of their positions, and so of their places in the result.
    std::vector<Born> born;
};

/// The indices of `lines`, in the order of their positions.
///
/// **A copy of the order, and nothing is sorted** (ADR 0012): the document is
/// not touched, and the file is not either. A stable sort on the start, then
/// the end, is what Gaupol's `sorted` does.
[[nodiscard]] std::vector<std::size_t> inTimeOrder(std::span<const Subtitle> lines) {
    std::vector<std::size_t> order(lines.size());
    std::ranges::iota(order, std::size_t{0});
    std::ranges::stable_sort(order, [&lines](std::size_t left, std::size_t right) {
        const Subtitle& a = lines[left];
        const Subtitle& b = lines[right];
        return a.start != b.start ? a.start < b.start : a.end < b.end;
    });
    return order;
}

/// How many lines come after a later one in the file.
///
/// Counted against the latest start seen so far, as Gaupol counts the
/// subtitles it has to move: a line equal to it is in order.
[[nodiscard]] std::size_t linesOutOfOrder(std::span<const Subtitle> lines) {
    std::size_t count = 0;
    std::optional<Timestamp> latest;
    for (const Subtitle& line : lines) {
        if (latest.has_value() && line.start < *latest)
            ++count;
        else
            latest = line.start;
    }
    return count;
}

/// **By number**: the *n*th line, in the order of their positions, goes to the
/// *n*th subtitle. Lines past the last subtitle give birth to one each.
[[nodiscard]] Matching byNumber(std::size_t subtitles, std::span<const std::size_t> order) {
    Matching matching{.lineOf = std::vector<std::optional<std::size_t>>(subtitles), .born = {}};
    for (std::size_t rank = 0; rank < order.size(); ++rank) {
        if (rank < subtitles)
            matching.lineOf[rank] = order[rank];
        else
            matching.born.push_back(Born{.before = subtitles, .line = order[rank]});
    }
    return matching;
}

/// **By position**: the middle of each line is compared to the bounds of the
/// subtitles that are left. Gaupol's `_align_translations_by_position`, in the
/// same order and with the same two tests.
///
/// The pointer never goes back, so a subtitle is met once and a line that comes
/// after another in the same subtitle finds the next one — or, when there is
/// none whose bounds hold its middle, gives birth to one. That is the case
/// case of the pairs where a translation splits one subtitle in two pins.
///
/// **Millisecond middles are compared doubled**, so that the half of a
/// millisecond Gaupol rounds is not rounded here: `2 × end < start + end`.
[[nodiscard]] Matching byPosition(std::span<const Subtitle> subtitles,
                                  std::span<const Subtitle> lines,
                                  std::span<const std::size_t> order) {
    Matching matching{.lineOf = std::vector<std::optional<std::size_t>>(subtitles.size()),
                      .born = {}};
    std::size_t next = 0;

    for (const std::size_t index : order) {
        const std::int64_t middle =
            lines[index].start.milliseconds() + lines[index].end.milliseconds();

        // Past the subtitles that end before the middle of this line.
        while (next < subtitles.size() && 2 * subtitles[next].end.milliseconds() < middle)
            ++next;

        // None holds it: the subtitle it would have gone to starts after it, or
        // there is no subtitle left.
        if (next == subtitles.size() || 2 * subtitles[next].start.milliseconds() > middle) {
            matching.born.push_back(Born{.before = next, .line = index});
            continue;
        }

        matching.lineOf[next] = index;
        ++next;
    }
    return matching;
}

} // namespace

AttachedTranslation attachTranslation(const Project& project,
                                      std::span<const Subtitle> lines,
                                      const SourceFile& source,
                                      TranslationMethod method) {
    const std::size_t count = project.count();
    const std::vector<std::size_t> order = inTimeOrder(lines);

    const Matching matching = method == TranslationMethod::Number
                                  ? byNumber(count, order)
                                  : byPosition(project.subtitles(), lines, order);

    AttachedTranslation result;
    result.outcome.outOfOrder = linesOutOfOrder(lines);
    result.outcome.born = matching.born.size();

    // Every command is built **before** any is applied, as a group is: each one
    // captures the state it undoes when it is built, and the indices of the
    // texts are those of the document as it stands now.
    std::vector<std::unique_ptr<Command>> commands;

    // **The texts first, on the subtitles that are there.** What was there is
    // erased whether or not a line reaches the subtitle — Gaupol does the same —
    // and a subtitle whose text would not change is left alone.
    for (std::size_t rank = 0; rank < count; ++rank) {
        const SubtitleIndex index = SubtitleIndex::fromValue(rank);
        const std::optional<std::size_t>& line = matching.lineOf[rank];
        if (line.has_value())
            ++result.outcome.attached;
        else
            ++result.outcome.untranslated;

        const std::string wanted = line.has_value() ? lines[*line].mainText : std::string{};
        if (wanted != project.subtitleAt(index).translationText)
            commands.push_back(
                std::make_unique<SetTextCommand>(project, index, Document::Translation, wanted));
    }

    // **Then the subtitles the lines gave birth to**, each with its positions and
    // its text — the rows past the end of a paste are born with their text for
    // the same reason. Placed by the index they will have once those before them
    // are in: `before` counts subtitles of the document, and each earlier birth
    // is one more.
    for (std::size_t rank = 0; rank < matching.born.size(); ++rank) {
        const Born& born = matching.born[rank];
        const Subtitle& line = lines[born.line];
        commands.push_back(std::make_unique<InsertCommand>(
            SubtitleIndex::fromValue(born.before + rank),
            std::vector<Subtitle>{Subtitle{.start = line.start,
                                           .end = line.end,
                                           .mainText = {},
                                           .translationText = line.mainText}}));
    }

    result.command = std::make_unique<AttachTranslationCommand>(
        project,
        std::make_unique<CompositeCommand>(CommandKind::AttachTranslation, std::move(commands)),
        source);
    return result;
}

} // namespace subedit::core
