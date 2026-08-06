#pragma once

#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace subedit::core {

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
