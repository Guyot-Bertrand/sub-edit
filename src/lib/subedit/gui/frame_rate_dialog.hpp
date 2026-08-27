#pragma once

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QString>

#include <cstddef>
#include <optional>

class QLabel;

namespace subedit::gui {

class FrameRateBox;

/// Asks which frame rate the positions were computed against, and which they
/// should be computed against now.
///
/// **The input rate is asked, never guessed.** A subtitle file does not carry
/// it — SubRip has no header, WebVTT's is free text — and getting it wrong
/// shifts the whole file without a word. The field opens on the rate the
/// project carries, which is a starting point and not an answer.
///
/// **What the film declares is proposed, never imposed** — decision D6. When
/// the container names a rate, the dialog opens with « should play at » on it
/// and says where it came from; the user is free to pick another. It is the
/// second source of one answer, and the phase that deduces a third from the
/// positions will want to compare rather than to be overruled.
///
/// Without a film, or without `ffprobe`, the row is simply not there and the
/// dialog is the one that came before.
///
/// **The measurement changed that, and phase 16 is where.** The positions
/// themselves betray the grid they were written on, so the field above opens on
/// what they say rather than on what the project assumed — and the dialog says
/// where the value came from. It stays a proposal: the box is as free as it
/// ever was.
///
/// **Only a clean grid pre-fills it.** A partial one is evidence the deduction
/// itself calls partial, and this field decides an operation on the whole file.
/// The status bar and the analysis carry the partial case; this does not.
///
/// **Two sources, and neither is arbitrated** — decision D13. What the container
/// declares and what the positions say are not the same fact: the first is the
/// rate the film runs at, the second the grid the file was written on. A file
/// at 24 for a film at 25 is not a contradiction, it is the very case the
/// alignment exists for. So both are shown, and the user chooses.
class FrameRateDialog final : public OperationDialog {
    Q_OBJECT

public:
    /// `declared` is what the associated film says of itself, `deduced` what
    /// the positions say, either of them possibly nothing.
    FrameRateDialog(std::size_t targetCount,
                    core::FrameRate current,
                    std::optional<core::FrameRate> declared = {},
                    std::optional<core::FrameRate> deduced = {},
                    QWidget* parent = nullptr);

    [[nodiscard]] core::FrameRate input() const;

    [[nodiscard]] core::FrameRate output() const;

    [[nodiscard]] bool isComplete() const override;

    /// Picks both rates, as a user would.
    void setRates(core::FrameRate from, core::FrameRate to);

    /// What the dialog says of the film's own rate — empty when it says
    /// nothing, which is how a test reads « the row is not there ».
    [[nodiscard]] QString declaredLabel() const;

    /// What the dialog says of the grid the positions were written on — empty
    /// when no clean grid was found.
    [[nodiscard]] QString deducedLabel() const;

private:
    FrameRateBox* m_input;
    FrameRateBox* m_output;
    QLabel* m_declared = nullptr;
    QLabel* m_deduced = nullptr;
};

} // namespace subedit::gui
