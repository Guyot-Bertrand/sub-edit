#pragma once

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <cstddef>
#include <optional>

class QComboBox;

namespace subedit::gui {

/// Asks which frame rate to lay the positions back onto.
///
/// **One list, and that is the whole difference with the conversion.** A
/// conversion needs two rates because it rescales from one to the other; an
/// alignment needs only the grid to land on, because it moves nothing else. If
/// this dialog ever grows a second list, the operation has stopped being an
/// alignment.
///
/// **It opens on what the film declares, not on what the positions say.** The
/// intention is to join the grid of the film; the deduction names the one being
/// left. Without a film, or without `ffprobe`, it opens on the rate the project
/// carries.
class SnapDialog final : public OperationDialog {
    Q_OBJECT

public:
    SnapDialog(std::size_t targetCount,
               core::FrameRate current,
               std::optional<core::FrameRate> declared = {},
               QWidget* parent = nullptr);

    [[nodiscard]] core::FrameRate rate() const;

    /// Always: any of the eight is a grid to land on, including the one the
    /// file already sits on — aligning a file on its own grid moves nothing,
    /// and refusing it would be refusing a no-op rather than an error.
    [[nodiscard]] bool isComplete() const override { return true; }

    /// Picks a rate, as a user would.
    void setRate(core::FrameRate rate);

private:
    QComboBox* m_rate;
};

} // namespace subedit::gui
