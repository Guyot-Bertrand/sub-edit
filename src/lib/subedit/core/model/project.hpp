#pragma once

#include <subedit/core/model/order_report.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace subedit::core {

class Selection;

/// A set of subtitles, its frame rate, and where it came from.
///
/// The subtitles live in a `std::vector`: access is essentially sequential and
/// by index, insertions and removals come in groups. A cleverer structure will
/// have to be justified by a measurement.
///
/// This class holds state and hands it back; it decides nothing. Editing goes
/// through reversible commands, which arrive with the operations.
class Project {

public:
    /// Returns the subtitles, in file order.
    [[nodiscard]] std::span<const Subtitle> subtitles() const { return m_subtitles; }

    /// Replaces every subtitle at once, as a reader does after parsing.
    void setSubtitles(std::vector<Subtitle> subtitles) { m_subtitles = std::move(subtitles); }

    /// Returns how many subtitles the project holds.
    [[nodiscard]] std::size_t count() const { return m_subtitles.size(); }

    /// Returns the subtitle at `index`.
    ///
    /// Throws `std::out_of_range` past the last one. An index out of range is
    /// a programming error, not a case to handle: failing loudly beats reading
    /// whatever lies after the vector.
    [[nodiscard]] const Subtitle& subtitleAt(SubtitleIndex index) const {
        return m_subtitles.at(index.value());
    }

    /// Returns the subtitle at `index`, for writing.
    ///
    /// This is how a command changes one subtitle without rebuilding the whole
    /// vector, which on a file of several thousand would be absurd.
    [[nodiscard]] Subtitle& subtitleAt(SubtitleIndex index) {
        return m_subtitles.at(index.value());
    }

    /// Inserts `subtitles` before `index`, keeping their order.
    ///
    /// `index` may equal `count()`, which appends — one past the last is the
    /// insertion point of every sequence, and refusing it would make appending
    /// the one case a caller has to write differently. Throws
    /// `std::out_of_range` beyond that.
    void insert(SubtitleIndex index, std::span<const Subtitle> subtitles);

    /// Removes the selected subtitles and returns them, ascending by index.
    ///
    /// Returning them is the point: that is exactly the state the inverse
    /// command has to retain, and handing it back saves reading it before
    /// erasing it. Throws `std::out_of_range` if the selection names an index
    /// the project does not have.
    [[nodiscard]] std::vector<Subtitle> remove(const Selection& selection);

    /// Returns the indices of the subtitles that are out of order, a pure
    /// query.
    ///
    /// The index reported is the one that is out of place, not the one it is
    /// out of place against: that is the line an interface has to show. Equal
    /// starts are not disorder under either reading — neither precedes the
    /// other.
    ///
    /// `report` chooses what "out of order" means; see `OrderReport`. The
    /// default is the reading phase 2 shipped, so that callers written before
    /// the second one existed are unaffected.
    ///
    /// A query and not an invariant: the model never sorts by itself. See the
    /// order policy of the phase-2 spec, and ADR 0012.
    [[nodiscard]] std::vector<SubtitleIndex>
    outOfOrder(OrderReport report = OrderReport::Breaks) const;

    /// Returns the frame rate the project is read against.
    [[nodiscard]] FrameRate frameRate() const { return m_frameRate; }

    void setFrameRate(FrameRate rate) { m_frameRate = rate; }

    /// Returns what the file this project came from looked like.
    [[nodiscard]] const SourceFile& sourceFile() const { return m_sourceFile; }

    void setSourceFile(SourceFile source) { m_sourceFile = std::move(source); }

private:
    std::vector<Subtitle> m_subtitles;

    /// 24000/1001 by default, as in Gaupol. A project that has never seen a
    /// video has to assume something, and this is the assumption its users are
    /// already used to.
    FrameRate m_frameRate{StandardFrameRate::Fps23976};

    SourceFile m_sourceFile;
};

} // namespace subedit::core
