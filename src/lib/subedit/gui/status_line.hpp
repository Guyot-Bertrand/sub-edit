#pragma once

#include <subedit/core/time/frame_rate.hpp>

#include <optional>

class QLabel;
class QStatusBar;

namespace subedit::core {
enum class Document;
class Project;
} // namespace subedit::core

namespace subedit::gui {

/// The rate a document counted in frames was read at, or nothing for the eight
/// formats of nine that count in time.
///
/// The grid's line reads it, and so does `Convert Frame Rate…`, which offers it
/// in place of a measured grid.
[[nodiscard]] std::optional<core::FrameRate> rateReadInFrames(const core::Project& project);

/// The four standing facts of the status bar — ADR 0034, issue #485.
///
/// **Permanent widgets and not `showMessage`.** What film a document
/// accompanies and what grid its positions were written on are standing facts,
/// not passing remarks, and a message can be pushed aside by the next one. The
/// passing remarks stay the window's.
///
/// **Every line is worded in `core/wording.hpp`**, where the command line reads
/// the same words; this only puts them on screen. It keeps nothing of a
/// project: each refresh is given the one on screen.
class StatusLine final {

public:
    /// Builds the four labels and lays them on `bar`, which owns them.
    explicit StatusLine(QStatusBar& bar);

    /// The text an operation aims at, and nothing while there is only one.
    /// `GUI-TRANS-05`.
    [[nodiscard]] QLabel* target() const { return m_target; }

    /// The encoding the document carries. `GUI-ENC-01`.
    [[nodiscard]] QLabel* encoding() const { return m_encoding; }

    /// The grid the positions were written on. `GUI-GRID-01`.
    [[nodiscard]] QLabel* grid() const { return m_grid; }

    /// The associated film — its name, or that there is none. `GUI-VIDEO-01`.
    [[nodiscard]] QLabel* video() const { return m_video; }

    /// Names `aimed`, or hides the line when there is nothing to choose
    /// between — the label of a window that never opens a translation says
    /// nothing, which is what keeps it from being noise.
    void showTarget(std::optional<core::Document> aimed);

    /// Says the encoding `project`'s document carries.
    void refreshEncoding(const core::Project& project);

    /// Deduces the grid of `project` again, or says the rate it counts frames
    /// at.
    ///
    /// **Recomputed rather than kept**, which is ADR 0021's choice: a stored
    /// derived value is an invalidation to hold, and every edit of a position
    /// would stale it. A pure function called again has no such problem, and
    /// it costs a fraction of a millisecond on a full-length file.
    void refreshGrid(const core::Project& project);

    /// Says what film `project` is watched against, and the rate it declares.
    void refreshVideo(const core::Project& project);

private:
    QLabel* m_target;
    QLabel* m_encoding;
    QLabel* m_grid;
    QLabel* m_video;
};

} // namespace subedit::gui
