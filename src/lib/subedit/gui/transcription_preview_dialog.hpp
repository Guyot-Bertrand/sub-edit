#pragma once

#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/subtitle_assembly.hpp>
#include <subedit/core/transcription/transcript.hpp>

#include <QDialog>

#include <cstddef>
#include <optional>
#include <vector>

class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace subedit::gui {

/// What the preview shows of a transcription.
struct TranscriptionPreview {
    /// **What the engine returned**, with no post-processing: exactly what `--raw` would write.
    std::vector<core::TranscribedSegment> pieces;

    /// What the post-processing made of them, with where each subtitle comes from.
    core::Assembly assembly;

    /// The settings the assembly was made with: the dialog starts its three spin boxes from
    /// them. **They apply to this transcript only** — see `keptSettings`.
    core::TranscriptionSettings settings{};

    /// Set when the work stopped before its end — cancelled, or failed after producing something:
    /// how far into the film it had got.
    std::optional<core::Timestamp> partialUpTo{};
};

/// The subtitles detected, shown before they replace the project's: two tables side by side, each
/// with its own start and end — **on the left what the engine returned, on the right what they will
/// be once post-processed** — and two choices, apply them or ignore them (and lose them).
///
/// **The post-processing is tuned here**: the gap under which pieces are merged, the longest a
/// subtitle stays on screen and the most characters it holds. Changing one makes the right-hand
/// side again at once — the assembly is a function of the pieces and these limits alone — and the
/// selected subtitle stays selected, with its sources highlighted. The values hold for this
/// transcript only; "Keep these values as settings" is the one gesture that hands them to the
/// application, and closing the dialog does not.
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

    [[nodiscard]] QSpinBox* mergeGapBox() const { return m_gap; }

    [[nodiscard]] QSpinBox* maxDurationBox() const { return m_duration; }

    [[nodiscard]] QSpinBox* maxCharactersBox() const { return m_characters; }

    [[nodiscard]] QPushButton* keepButton() const { return m_keep; }

    [[nodiscard]] QLabel* keptNote() const { return m_keptNote; }

    /// The settings as the three spin boxes hold them now.
    [[nodiscard]] core::TranscriptionSettings tuned() const;

    /// What "Keep these values as settings" was asked to keep, as it was when it was asked; nothing
    /// if it never was.
    [[nodiscard]] const std::optional<core::TranscriptionSettings>& keptSettings() const {
        return m_kept;
    }

    [[nodiscard]] QPushButton* applyButton() const { return m_apply; }

    [[nodiscard]] QPushButton* ignoreButton() const { return m_ignore; }

private:
    /// Makes the right-hand side again from the pieces and `tuned()`, and puts back the selection.
    void reassemble();

    /// Highlights the sources of the subtitle at `row` of the right table, and shows them.
    void showSources(int row);

    TranscriptionPreview m_preview;
    QLabel* m_summary;
    QTableWidget* m_pieces;
    QTableWidget* m_subtitles;
    QSpinBox* m_gap;
    QSpinBox* m_duration;
    QSpinBox* m_characters;
    QPushButton* m_keep;
    QLabel* m_keptNote;
    QGroupBox* m_right = nullptr;
    std::optional<core::TranscriptionSettings> m_kept;
    QPushButton* m_apply;
    QPushButton* m_ignore;
    std::vector<int> m_highlighted;
};

} // namespace subedit::gui
