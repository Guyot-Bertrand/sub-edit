#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/transcription/transcript.hpp>

#include <QDialog>

#include <cstddef>
#include <optional>
#include <vector>

class QLabel;
class QPushButton;
class QTableWidget;

namespace subedit::gui {

/// What the preview shows of a transcription.
struct TranscriptionPreview {
    /// **What the engine returned**, with no post-processing: exactly what `--raw` would write.
    std::vector<core::TranscribedSegment> pieces;

    /// What the post-processing made of them, with where each subtitle comes from.
    core::Assembly assembly;

    /// Set when the work stopped before its end — cancelled, or failed after producing something:
    /// how far into the film it had got.
    std::optional<core::Timestamp> partialUpTo{};
};

/// The subtitles detected, shown before they replace the project's: two tables side by side, each
/// with its own start and end — **on the left what the engine returned, on the right what they will
/// be once post-processed** — and two choices, apply them or ignore them (and lose them).
///
/// The tables are read, not edited: neither times nor text can be changed here. **Selecting a
/// subtitle on the right highlights on the left the pieces it was made from, and scrolls the left
/// side to them**, so that a person judging the limits of the post-processing sees what each did to
/// the engine's cut. One subtitle at a time, and in that direction only.
///
/// Shown also, and above all, when a run is stopped: what had been obtained up to there is offered
/// the same way, titled as partial, so that a long run stopped late does not lose its work unasked.
class TranscriptionPreviewDialog final : public QDialog {
    Q_OBJECT

public:
    explicit TranscriptionPreviewDialog(TranscriptionPreview preview, QWidget* parent = nullptr);

    /// Left: the pieces of the engine. Right: the subtitles. Three columns each: start, end, text.
    [[nodiscard]] QTableWidget* piecesTable() const { return m_pieces; }

    [[nodiscard]] QTableWidget* subtitlesTable() const { return m_subtitles; }

    /// The line above the tables: how many of one became how many of the other, or that this is
    /// partial.
    [[nodiscard]] QLabel* summary() const { return m_summary; }

    /// The rows of the left table that stand highlighted: the sources of the selected subtitle.
    [[nodiscard]] std::vector<int> highlightedPieces() const { return m_highlighted; }

    /// The subtitles that would replace the project's: the right-hand side, as it stands.
    [[nodiscard]] std::vector<core::Subtitle> applied() const {
        return m_preview.assembly.subtitles;
    }

    [[nodiscard]] QPushButton* applyButton() const { return m_apply; }

    [[nodiscard]] QPushButton* ignoreButton() const { return m_ignore; }

private:
    /// Highlights the sources of the subtitle at `row` of the right table, and shows them.
    void showSources(int row);

    TranscriptionPreview m_preview;
    QLabel* m_summary;
    QTableWidget* m_pieces;
    QTableWidget* m_subtitles;
    QPushButton* m_apply;
    QPushButton* m_ignore;
    std::vector<int> m_highlighted;
};

} // namespace subedit::gui
