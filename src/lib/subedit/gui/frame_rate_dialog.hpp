#pragma once

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <cstddef>

class QComboBox;

namespace subedit::gui {

/// Asks which frame rate the positions were computed against, and which they
/// should be computed against now.
///
/// **The input rate is asked, never guessed.** A subtitle file does not carry
/// it — SubRip has no header, WebVTT's is free text — and getting it wrong
/// shifts the whole file without a word. The field opens on the rate the
/// project carries, which is a starting point and not an answer.
///
/// Deducing it from the positions themselves is possible, and is phase 16's
/// business. Nothing here depends on it: the day the measurement exists it
/// replaces the pre-filled value without this dialog changing shape.
class FrameRateDialog final : public OperationDialog {
    Q_OBJECT

public:
    FrameRateDialog(std::size_t targetCount, core::FrameRate current, QWidget* parent = nullptr);

    [[nodiscard]] core::FrameRate input() const;

    [[nodiscard]] core::FrameRate output() const;

    [[nodiscard]] bool isComplete() const override;

    /// Picks both rates, as a user would.
    void setRates(core::FrameRate from, core::FrameRate to);

private:
    QComboBox* m_input;
    QComboBox* m_output;
};

} // namespace subedit::gui
