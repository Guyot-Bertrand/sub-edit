#pragma once

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QString>

#include <cstddef>
#include <optional>

class QComboBox;
class QLabel;

namespace subedit::gui {

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
/// Deducing it from the positions themselves is possible, and is phase 16's
/// business. Nothing here depends on it: the day the measurement exists it
/// replaces the pre-filled value without this dialog changing shape.
class FrameRateDialog final : public OperationDialog {
    Q_OBJECT

public:
    /// `declared` is what the associated film says of itself, or nothing.
    FrameRateDialog(std::size_t targetCount,
                    core::FrameRate current,
                    std::optional<core::FrameRate> declared = {},
                    QWidget* parent = nullptr);

    [[nodiscard]] core::FrameRate input() const;

    [[nodiscard]] core::FrameRate output() const;

    [[nodiscard]] bool isComplete() const override;

    /// Picks both rates, as a user would.
    void setRates(core::FrameRate from, core::FrameRate to);

    /// What the dialog says of the film's own rate — empty when it says
    /// nothing, which is how a test reads « the row is not there ».
    [[nodiscard]] QString declaredLabel() const;

private:
    QComboBox* m_input;
    QComboBox* m_output;
    QLabel* m_declared = nullptr;
};

} // namespace subedit::gui
