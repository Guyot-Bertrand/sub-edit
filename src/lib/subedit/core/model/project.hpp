#pragma once

#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <cstddef>
#include <filesystem>
#include <optional>
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
///
/// One exception, and it is a rule about the state rather than about editing:
/// **a chosen video is never replaced by a guessed one** — decision D5. Left to
/// each caller, that rule would be re-stated at every place a file is opened or
/// saved, and forgotten at one of them.
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

    /// Puts `subtitles` back at `destinations`, in one pass.
    ///
    /// The exact inverse of `remove`: `destinations` are the very indices that
    /// removal was given — positions in the **restored** project, and not in
    /// the one this is called on. That is what lets a command undo itself by
    /// handing back what it kept, unchanged.
    ///
    /// One call rather than one insertion per subtitle, which is the point:
    /// inserting `k` subtitles one at a time shifts the tail `k` times, where
    /// rebuilding the vector once costs `O(n + k)`.
    ///
    /// Throws `std::invalid_argument` when the counts disagree, and
    /// `std::out_of_range` when a destination lies past the end of the project
    /// that would result. Both are programming errors, refused loudly.
    void restore(const Selection& destinations, std::span<const Subtitle> subtitles);

    /// Tells whether every subtitle starts no earlier than the one before it.
    ///
    /// A boolean and not a list: the strict order policy asks this after every
    /// operation, and building a vector nobody reads to answer it would be a
    /// cost paid at each edit. What is *where* the disorder lies belongs to
    /// `scanAnomalies`, which reports it among the rest.
    ///
    /// Equal starts are in order — neither precedes the other.
    ///
    /// A query and not an invariant: the model never sorts by itself. See the
    /// order policy of the phase-2 spec, and ADR 0012.
    [[nodiscard]] bool isInOrder() const;

    /// Returns the frame rate the project is read against.
    [[nodiscard]] FrameRate frameRate() const { return m_frameRate; }

    void setFrameRate(FrameRate rate) { m_frameRate = rate; }

    /// Returns the video this document is watched against, and where its path
    /// came from. Nothing, for a document no video has been associated with.
    [[nodiscard]] const std::optional<AssociatedVideo>& video() const { return m_video; }

    /// Associates the video the user named, whatever was there before.
    void chooseVideo(std::filesystem::path path);

    /// Offers the video the naming convention found, and answers whether it
    /// was taken.
    ///
    /// A proposal replaces an earlier proposal — reopening or renaming makes
    /// the convention speak again, and its latest word is the one that counts.
    /// It never replaces a choice: that is D5, and it is why the origin is
    /// remembered at all.
    bool proposeVideo(std::filesystem::path path);

    /// Records what the container of the associated video declares, or that it
    /// declares nothing. Does nothing when no video is associated.
    ///
    /// **Separate from associating the film, and not by oversight.** Reading it
    /// costs a process; a proposal that a choice refuses reads nothing, and a
    /// film that has not changed is not asked again. Whoever associates decides
    /// when the question is worth its answer.
    void setDeclaredFrameRate(std::optional<FrameRate> rate);

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

    std::optional<AssociatedVideo> m_video;
};

} // namespace subedit::core
